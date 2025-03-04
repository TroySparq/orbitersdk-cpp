#include "p2pclient.h"
#include "p2pmanagerbase.h"

namespace P2P {
  void ClientSession::run() {
    Utils::logToDebug(Log::P2PClientSession, __func__,
      "ClientSession Trying to resolve: " + this->host_ + ":" + std::to_string(this->port_)
    );
    this->resolve();
  }

  void ClientSession::stop() {}

  void ClientSession::handleError(const std::string& func, const beast::error_code& ec) {
    if (!this->closed_) {
      Utils::logToDebug(Log::P2PClientSession, func,
        "Client Error Code: " + std::to_string(ec.value()) + " message: " + ec.message()
      );
      this->manager_.unregisterSession(shared_from_this());
      this->closed_ = true;
    }
    return;
  }

  void ClientSession::resolve() {
    resolver_.async_resolve(this->host_, std::to_string(port_), beast::bind_front_handler(
      &ClientSession::on_resolve, shared_from_this()
    ));
  }

  void ClientSession::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) { handleError(__func__, ec); return; }
    this->connect(results);
  }

  void ClientSession::connect(tcp::resolver::results_type& results) {
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30)); // Set timeout for client
    beast::get_lowest_layer(ws_).async_connect(results, beast::bind_front_handler(
      &ClientSession::on_connect, shared_from_this()
    ));
  }

  void ClientSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
    if (ec) { handleError(__func__, ec); return; }

    // Turn off timeout on the tcp_stream since websocket stream has its own timeout system
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(websocket::stream_base::decorator(
    [this](websocket::request_type& req) {
      req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
      req.set("X-Node-Id", this->manager_.nodeId().hex().get()); // Add the custom header
      req.set("X-Node-Type", std::to_string(this->manager_.nodeType()));
      req.set("X-Node-ServerPort", std::to_string(this->manager_.serverPort()));
    }));

    std::string hostStr = this->host_ + ':' + std::to_string(ep.port());
    this->handshake(hostStr);
  }

  void ClientSession::handshake(const std::string& host) {
    // async_handshake has to be done within a lambda function, in order to
    // read the request headers made by the server.
    // req had to be declared in the class otherwise it would be destroyed and cause a segfault.
    Utils::logToFile("ClientSession: Handshaking");
    ws_.async_handshake(this->req_, host, "/", [this](beast::error_code ec) {
      if (ec) { handleError(__func__, ec); return; }

      try {
        this->hostNodeId_ = Hash(Hex::toBytes(std::string(this->req_["X-Node-Id"])));
      } catch (std::exception &e) {
        Utils::logToDebug(Log::P2PClientSession, __func__,
          "ClientSession: X-Node-Id header is not valid from: "
          + this->host_ + ":" + std::to_string(this->port_)
        );
      }

      std::string nodeTypeStr = std::string(this->req_["X-Node-Type"]);
      if (nodeTypeStr.size() == 1) {
        if (!std::isdigit(nodeTypeStr[0])) {
          Utils::logToDebug(Log::P2PClientSession, __func__,
            "ClientSession: X-Node-Type header is not valid, not a digit from: "
            + this->hostNodeId_.hex().get()
          );
          return;
        }
        this->hostType_ = (NodeType)std::stoi(nodeTypeStr);
      } else {
        Utils::logToDebug(Log::P2PClientSession, __func__,
          "ClientSession: X-Node-Type header is not valid from: "
          + this->hostNodeId_.hex().get()
        );
        return;
      }

      std::string serverPortStr = std::string(this->req_["X-Node-ServerPort"]);
      if (serverPortStr.size() > 0) {
        if (!std::all_of(serverPortStr.begin(), serverPortStr.end(), ::isdigit)) {
          Utils::logToDebug(Log::P2PClientSession, __func__,
            "ClientSession: X-Node-ServerPort header is not valid, not a digit from: "
            + this->hostNodeId_.hex().get()
          );
          return;
        }
        this->hostServerPort_ = std::stoi(serverPortStr);
      } else {
        Utils::logToDebug(Log::P2PClientSession, __func__,
          "ClientSession: X-Node-ServerPort header is not valid from: "
          + this->hostNodeId_.hex().get()
        );
        return;
      }

      this->address_ = ws_.next_layer().socket().remote_endpoint().address();
      if (!this->manager_.registerSession(shared_from_this())) return;
      this->read();
    });
  }

  void ClientSession::read() {
    ws_.async_read(this->receiveBuffer_, beast::bind_front_handler(
      &ClientSession::on_read, shared_from_this()
    ));
  }

  void ClientSession::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    if (ec) { handleError(__func__, ec); return; }

    try {
      if (this->receiveBuffer_.size() >= 11) {
        if (this->manager_.isClosed()) return;
        std::string messageStr = boost::beast::buffers_to_string(this->receiveBuffer_.data());
        Message message(std::move(messageStr));
        this->threadPool->push_task(
          &ManagerBase::handleMessage, &this->manager_, shared_from_this(), message
        );
      } else {
        Utils::logToDebug(Log::P2PClientSession, __func__,
          "Message too short: " + this->hostNodeId_.hex().get()
        );
      }
    } catch (std::exception &e) {
      Utils::logToDebug(Log::P2PClientSession, __func__,
        "ClientSession exception from: " + this->hostNodeId_.hex().get() + " " + e.what()
      );
    }
    this->receiveBuffer_.consume(this->receiveBuffer_.size());
    this->read();
  }

  void ClientSession::write(const Message& data) {
    writeLock_.lock();
    size_t n = boost::asio::buffer_copy(
      this->answerBuffer_.prepare(data.raw().size()), boost::asio::buffer(data.raw())
    );
    this->answerBuffer_.commit(n);
    ws_.async_write(this->answerBuffer_.data(), beast::bind_front_handler(
      &ClientSession::on_write, shared_from_this()
    ));
  }

  void ClientSession::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    this->answerBuffer_.consume(this->answerBuffer_.size());
    writeLock_.unlock();
    if (ec) { handleError(__func__, ec); return; }
  }

  void ClientSession::close() {
    // TODO: close does not wait for on_close to be called,
    // we need to somehow change this to a future and wait.
    // Server does not need this because it is already waited.
    this->closed_ = true;
    ws_.async_close(websocket::close_code::normal, beast::bind_front_handler(
      &ClientSession::on_close, shared_from_this()
    ));
  }

  void ClientSession::on_close(beast::error_code ec) {
    if (ec) { handleError(__func__, ec); return; }
    // If we get here then the connection is closed gracefully
    Utils::logToFile("Client Session disconnected: " + host_ + ":" + std::to_string(port_));
  }
};

