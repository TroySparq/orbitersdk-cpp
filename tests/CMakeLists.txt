set(TESTS_HEADERS
  ""
  PARENT_SCOPE
)

set (TESTS_SOURCES
  ""
  ${CMAKE_SOURCE_DIR}/tests/utils/block.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/block_throw.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/db.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/ecdsa.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/hex.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/merkle.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/randomgen.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/strings.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/tx.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/tx_throw.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/utils.cpp
  ${CMAKE_SOURCE_DIR}/tests/utils/options.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/abi.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/erc20.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/contractmanager.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/erc20wrapper.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/nativewrapper.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeuint8_t.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeuint16_t.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeuint32_t.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeuint64_t.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeuint256_t.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safestring.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeaddress.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safeunorderedmap.cpp
  ${CMAKE_SOURCE_DIR}/tests/contract/variables/safebool.cpp
  ${CMAKE_SOURCE_DIR}/tests/core/rdpos.cpp
  ${CMAKE_SOURCE_DIR}/tests/core/storage.cpp
  ${CMAKE_SOURCE_DIR}/tests/core/state.cpp
  ${CMAKE_SOURCE_DIR}/tests/core/blockchain.cpp
  ${CMAKE_SOURCE_DIR}/tests/net/p2p/p2p.cpp
  ${CMAKE_SOURCE_DIR}/tests/net/http/httpjsonrpc.cpp
  PARENT_SCOPE
)
