#include "p2pmanagerdiscovery.h"

namespace P2P {
  void ManagerDiscovery::handleMessage(std::shared_ptr<BaseSession> session, const Message message) {
    if (this->closed_) return;
    switch (message.type()) {
      case Requesting:
        handleRequest(session, message);
        break;
      case Answering:
        handleAnswer(session, message);
        break;
      default:
        Utils::logToDebug(Log::P2PParser, __func__, "Invalid message type from " + session->hostNodeId().hex().get() + ", closing session.");
        this->disconnectSession(session->hostNodeId());
        break;
    }
  }

  void ManagerDiscovery::handleRequest(std::shared_ptr<BaseSession>& session, const Message& message) {
    switch (message.command()) {
      case Ping:
        handlePingRequest(session, message);
        break;
      // case Info:
        // We don't handle info within discovery because discovery keeps zero track of blockchain history.
        // handleInfoRequest(session, message);
        // break;
      case RequestNodes:
        handleRequestNodesRequest(session, message);
        break;
      default:
        Utils::logToDebug(Log::P2PParser, __func__, "Invalid Request Command Type from " + session->hostNodeId().hex().get() + ", closing session.");
        this->disconnectSession(session->hostNodeId());
        break;
    }
  }

  void ManagerDiscovery::handleAnswer(std::shared_ptr<BaseSession>& session, const Message& message) {
    switch (message.command()) {
      case Ping:
        handlePingAnswer(session, message);
        break;
      case Info:
        // handleInfoAnswer(session, message);
        break;
      case RequestNodes:
        handleRequestNodesAnswer(session, message);
        break;
      default:
        Utils::logToDebug(Log::P2PParser, __func__, "Invalid Answer Command Type from " + session->hostNodeId().hex().get() + ", closing session.");
        this->disconnectSession(session->hostNodeId());
        break;
    }
  }

  void ManagerDiscovery::handlePingRequest(std::shared_ptr<BaseSession>& session, const Message& message) {
    if (!RequestDecoder::ping(message)) {
      Utils::logToDebug(Log::P2PParser, __func__, "Invalid ping request from " + session->hostNodeId().hex().get() + " closing session.");
      this->disconnectSession(session->hostNodeId());
      return;
    }
    this->answerSession(session, AnswerEncoder::ping(message));
  }

  void ManagerDiscovery::handleRequestNodesRequest(std::shared_ptr<BaseSession>& session, const Message& message) {
    if (!RequestDecoder::requestNodes(message)) {
      Utils::logToDebug(Log::P2PParser, __func__, "Invalid requestNodes request, closing session.");
      this->disconnectSession(session->hostNodeId());
      return;
    }

    std::unordered_map<Hash, std::tuple<NodeType, boost::asio::ip::address, unsigned short>, SafeHash> nodes;
    {
      std::shared_lock lock(sessionsMutex);
      for (const auto& session : this->sessions_) {
        nodes[session.second->hostNodeId()] = std::make_tuple(session.second->hostType(), session.second->address(), session.second->hostServerPort());
      }
    }
    this->answerSession(session, AnswerEncoder::requestNodes(message, nodes));
  }

  void ManagerDiscovery::handlePingAnswer(std::shared_ptr<BaseSession>& session, const Message& message) {
    std::unique_lock lock(requestsMutex);
      if (!requests_.contains(message.id())) {
      Utils::logToDebug(Log::P2PParser, __func__, "Answer to invalid request from " + session->hostNodeId().hex().get());
      this->disconnectSession(session->hostNodeId());
      return;
    }
    requests_[message.id()]->setAnswer(message);
  }

  void ManagerDiscovery::handleRequestNodesAnswer(std::shared_ptr<BaseSession>& session, const Message& message) {
    std::unique_lock lock(requestsMutex);
    if (!requests_.contains(message.id())) {
      Utils::logToDebug(Log::P2PParser, __func__, "Answer to invalid request from " + session->hostNodeId().hex().get());
      this->disconnectSession(session->hostNodeId());
      return;
    }
    requests_[message.id()]->setAnswer(message);
  }
};

