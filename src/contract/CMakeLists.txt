set(CONTRACT_HEADERS
  ${CMAKE_SOURCE_DIR}/src/contract/abi.h
  ${CMAKE_SOURCE_DIR}/src/contract/contract.h
  ${CMAKE_SOURCE_DIR}/src/contract/dynamiccontract.h
  ${CMAKE_SOURCE_DIR}/src/contract/contractmanager.h
  ${CMAKE_SOURCE_DIR}/src/contract/erc20wrapper.h
  ${CMAKE_SOURCE_DIR}/src/contract/nativewrapper.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safebase.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safeuint8_t.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safeuint16_t.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safeuint32_t.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safeuint64_t.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safeuint256_t.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safestring.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safeaddress.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safeunorderedmap.h
  ${CMAKE_SOURCE_DIR}/src/contract/variables/safebool.h
  ${CMAKE_SOURCE_DIR}/src/contract/erc20.h
  PARENT_SCOPE
)

set(CONTRACT_SOURCES
  ${CMAKE_SOURCE_DIR}/src/contract/abi.cpp
  ${CMAKE_SOURCE_DIR}/src/contract/contractmanager.cpp
  ${CMAKE_SOURCE_DIR}/src/contract/dynamiccontract.cpp
  ${CMAKE_SOURCE_DIR}/src/contract/nativewrapper.cpp
  ${CMAKE_SOURCE_DIR}/src/contract/erc20.cpp
  ${CMAKE_SOURCE_DIR}/src/contract/erc20wrapper.cpp
  PARENT_SCOPE
)

