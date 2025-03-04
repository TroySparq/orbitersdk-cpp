#include "nativewrapper.h"

/// Default Constructor when loading contract from DB.
NativeWrapper::NativeWrapper(ContractManager::ContractManagerInterface &interface, const Address& address, const std::unique_ptr<DB> &db) :
  DynamicContract(interface, address, db), _name(this), _symbol(this), _decimals(this), _totalSupply(this), _balances(this), _allowed(this) {

  this->_name = db->get("_name", DBPrefix::contracts + this->getContractAddress().get());
  this->_symbol = db->get("_symbol", DBPrefix::contracts + this->getContractAddress().get());
  this->_decimals = Utils::bytesToUint8(db->get("_decimals", DBPrefix::contracts + this->getContractAddress().get()));
  this->_totalSupply = Utils::bytesToUint256(db->get("_totalSupply", DBPrefix::contracts + this->getContractAddress().get()));
  auto balances = db->getBatch(DBPrefix::contracts + this->getContractAddress().get() + "_balances");
  for (const auto& dbEntry : balances) {
    this->_balances[Address(dbEntry.key, true)] = Utils::fromBigEndian<uint256_t>(dbEntry.value);
  }

  auto allowances = db->getBatch(DBPrefix::contracts + this->getContractAddress().get() + "_allowed");
  for (const auto& dbEntry : allowances) {
    this->_allowed[Address(dbEntry.key, true)][Address(dbEntry.value.substr(0, 20), true)] = Utils::fromBigEndian<uint256_t>(dbEntry.value.substr(20));
  }
  this->registerContractFunctions();
  updateState(true);
}

NativeWrapper::NativeWrapper(ContractManager::ContractManagerInterface &interface, const std::string& erc20_name, const std::string& erc20_symbol, const uint8_t& erc20_decimals,
             const Address& address, const Address& creator, const uint64_t& chainId, const std::unique_ptr<DB> &db) :
  DynamicContract(interface, "NativeWrapper", address, creator, chainId, db), _name(this), _symbol(this), _decimals(this), _totalSupply(this), _balances(this), _allowed(this) {
  _name = erc20_name;
  _symbol = erc20_symbol;
  _decimals = erc20_decimals;
  this->registerContractFunctions();
  updateState(true);
}

NativeWrapper::~NativeWrapper() {
  DBBatch balancesBatch;
  DBBatch allowanceBatch;

  this->db->put("_name", _name.get(), DBPrefix::contracts + this->getContractAddress().get());
  this->db->put("_symbol", _symbol.get(), DBPrefix::contracts + this->getContractAddress().get());
  this->db->put("_decimals", Utils::uint8ToBytes(_decimals.get()), DBPrefix::contracts + this->getContractAddress().get());
  this->db->put("_totalSupply", Utils::uint256ToBytes(_totalSupply.get()), DBPrefix::contracts + this->getContractAddress().get());

  for (auto it = _balances.cbegin(); it != _balances.cend(); ++it) {
    std::string key = it->first.get();
    std::string value;
    value += Utils::uintToBytes(it->second);
    balancesBatch.puts.emplace_back(DBEntry(key, value));
  }

  for (auto it = _allowed.cbegin(); it != _allowed.cend(); ++it) {
    for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2) {
      std::string key = it->first.get();
      std::string value;
      value += it2->first.get();
      value += Utils::uintToBytes(it2->second);
      allowanceBatch.puts.emplace_back(DBEntry(key, value));
    }
  }
  this->db->putBatch(balancesBatch, DBPrefix::contracts + this->getContractAddress().get() + "_balances");
  this->db->putBatch(allowanceBatch, DBPrefix::contracts + this->getContractAddress().get() + "_allowed");
  this->registerContractFunctions();
}

void NativeWrapper::registerContractFunctions() {
  this->registerViewFunction(Hex::toBytes("0x06fdde03"), [this](const ethCallInfo &callInfo) {
    return this->name();
  });
  this->registerViewFunction(Hex::toBytes("0x95d89b41"), [this](const ethCallInfo &callInfo) {
    return this->symbol();
  });
  this->registerViewFunction(Hex::toBytes("0x313ce567"), [this](const ethCallInfo &callInfo) {
    return this->decimals();
  });
  this->registerViewFunction(Hex::toBytes("0x18160ddd"), [this](const ethCallInfo &callInfo) {
    return this->totalSupply();
  });
  this->registerViewFunction(Hex::toBytes("0x70a08231"), [this](const ethCallInfo &callInfo) {
    std::vector<ABI::Types> types = { ABI::Types::address };
    ABI::Decoder decoder(types, std::get<5>(callInfo).substr(4));
    return this->balanceOf(decoder.getData<Address>(0));
  });
  this->registerViewFunction(Hex::toBytes("0xdd62ed3e"), [this](const ethCallInfo &callInfo) {
    std::vector<ABI::Types> types = { ABI::Types::address, ABI::Types::address };
    ABI::Decoder decoder(types, std::get<5>(callInfo).substr(4));
    return this->allowance(decoder.getData<Address>(0), decoder.getData<Address>(1));
  });

  this->registerFunction(Hex::toBytes("0xa9059cbb"), [this](const ethCallInfo &callInfo) {
    std::vector<ABI::Types> types = { ABI::Types::address, ABI::Types::uint256 };
    ABI::Decoder decoder(types, std::get<5>(callInfo).substr(4));
    this->transfer(decoder.getData<Address>(0), decoder.getData<uint256_t>(1));
  });
  this->registerFunction(Hex::toBytes("0x095ea7b3"), [this](const ethCallInfo &callInfo) {
    std::vector<ABI::Types> types = { ABI::Types::address, ABI::Types::uint256 };
    ABI::Decoder decoder(types, std::get<5>(callInfo).substr(4));
    this->approve(decoder.getData<Address>(0), decoder.getData<uint256_t>(1));
  });
  this->registerFunction(Hex::toBytes("0x23b872dd"), [this](const ethCallInfo &callInfo) {
    std::vector<ABI::Types> types = { ABI::Types::address, ABI::Types::address, ABI::Types::uint256 };
    ABI::Decoder decoder(types, std::get<5>(callInfo).substr(4));
    this->transferFrom(decoder.getData<Address>(0), decoder.getData<Address>(1), decoder.getData<uint256_t>(2));
  });
  this->registerPayableFunction(Hex::toBytes("0xd0e30db0"), [this](const ethCallInfo &callInfo) {
    this->deposit();
  });
  this->registerPayableFunction(Hex::toBytes("0x2e1a7d4d"), [this](const ethCallInfo &callInfo) {
    std::vector<ABI::Types> types = { ABI::Types::uint256 };
    ABI::Decoder decoder(types, std::get<5>(callInfo).substr(4));
    this->withdraw(decoder.getData<uint256_t>(0));
  });
}


void NativeWrapper::_mintValue(const Address& address, const uint256_t& value) {
  _balances[address] += value;
  _totalSupply += value;
}

std::string NativeWrapper::name() const {
  return ABI::Encoder({this->_name.get()}).getRaw();
}

std::string NativeWrapper::symbol() const {
  return ABI::Encoder({this->_symbol.get()}).getRaw();
}

std::string NativeWrapper::decimals() const {
  return ABI::Encoder({this->_decimals.get()}).getRaw();
}

std::string NativeWrapper::totalSupply() const {
  return ABI::Encoder({this->_totalSupply.get()}).getRaw();
}

std::string NativeWrapper::balanceOf(const Address& _owner) const {
  const auto& it = std::as_const(this->_balances).find(_owner);
  if (it == this->_balances.end()) {
    return ABI::Encoder({0}).getRaw();
  } else {
    return ABI::Encoder({it->second}).getRaw();
  }
}

void NativeWrapper::transfer(const Address& _to, const uint256_t& _value) {
  this->_balances[this->getCaller()] -= _value;
  this->_balances[_to] += _value;
}

void NativeWrapper::approve(const Address& _spender, const uint256_t& _value) {
  this->_allowed[this->getCaller()][_spender] = _value;
}

std::string NativeWrapper::allowance(const Address& _owner, const Address& _spender) const {
  const auto& it = std::as_const(this->_allowed).find(_owner);
  if (it == this->_allowed.end()) {
    return ABI::Encoder({0}).getRaw();
  } else {
    const auto& it2 = it->second.find(_spender);
    if (it2 == it->second.end()) {
      return ABI::Encoder({0}).getRaw();
    } else {
      return ABI::Encoder({it2->second}).getRaw();
    }
  }
}

void NativeWrapper::transferFrom(const Address& _from, const Address& _to, const uint256_t& _value) {
  this->_allowed[_from][this->getCaller()] -= _value;
  this->_balances[_from] -= _value;
  this->_balances[_to] += _value;
}

void NativeWrapper::deposit() {
  this->_balances[this->getCaller()] += this->getValue();
}

void NativeWrapper::withdraw(const uint256_t& _value) {
  this->_balances[this->getCaller()] -= _value;
  this->sendTokens(this->getCaller(), _value);
}



