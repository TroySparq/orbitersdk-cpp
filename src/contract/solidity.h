#ifndef SOLIDITY_H
#define SOLIDITY_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>


#include "../utils/utils.h"
/**
 * Namespace for encoding data like [Solidity](https://docs.soliditylang.org/en/latest/abi-spec.html) does.
 * A summed up rationale of how each individual type is encoded:
 * - uint256: data
 * - address: data
 * - bool: data
 * - bytes: offset, length, data
 * - string: offset, length, data
 * - uint256[]: offset[], size, data...
 * - address[]: offset[], size, data...
 * - bool[]: offset[], size, data...
 * - bytes[]: offset[], size, offset..., (length, data)...
 * - string[]: offset[], size, offset..., (length, data)...
 * Multiple types at once should be handled by packMulti().
 * ALL RETURNS ARE IN RAW BYTES FORMAT. Utils::bytesToHex() has to be used for
 * debugging and printing.
 */

namespace Solidity {
  /**
   * Check if a given value is of a given type and if its contents are valid.
   * Supported types are as follows:
   * - function (also checks the parameters individually)
   * - uint256 and uint256[]
   * - address and address[]
   * - bool and bool[]
   * - bytes and bytes[]
   * - string and string[]
   * @param type The type to check.
   * @param value The value to check.
   * @return `true` if value and type match, `false` otherwise.
   */
  bool checkType(std::string type, json value);

  /**
   * Pack an individual function into %Solidity format.
   * @param func The full packed function signature (e.g. `foo(bar,baz[])`, not just `foo`).
   * @return The first 4 hex bytes of the SHA3 hash of the function.
   */
  std::string packFunction(std::string func);

  /**
   * Pack an individual uint256 into %Solidity format.
   * @param num The number string.
   * @return The packed number in hex format, **LEFT** padded to a multiple of 32 hex bytes.
   */
  std::string packUint(std::string num);

  /**
   * Pack an individual address into %Solidity format.
   * @param add The address string.
   * @return The packed address, **LEFT** padded to a multiple of 32 hex bytes.
   */
  std::string packAddress(std::string add);

  /**
   * Pack an individual bool into %Solidity format.
   * @param b The boolean string. Can be either `"true"/"false"` or `"0"/"1"`.
   * @return The packed boolean, **LEFT** padded to a multiple of 32 hex bytes.
   */
  std::string packBool(std::string b);

  /**
   * Pack an individual bytes into %Solidity format.
   * @param hex The hex string that represents raw bytes.
   * @return The packed bytes, **RIGHT** padded to a multiple of 32 hex bytes.
   */
  std::string packBytes(std::string hex);

  /**
   * Pack an individual string into %Solidity format.
   * @param str The hex string that represents a UTF-8 formatted string.
   * @return The packed string, **RIGHT** padded to a multiple of 32 hex bytes.
   */
  std::string packString(std::string str);

  /**
   * Pack an individual uint256[] into %Solidity format.
   * @param numV The array of number strings.
   * @return The packed number array.
   */
  std::string packUintArray(std::vector<std::string> numV);

  /**
   * Pack an individual address[] into %Solidity format.
   * @param addV The array of address strings.
   * @return The packed address array.
   */
  std::string packAddressArray(std::vector<std::string> addV);

  /**
   * Pack an individual bool[] into %Solidity format.
   * @param bV The array of boolean strings. Can be either `"true"/"false"` or `"0"/"1"`.
   * @return The packed boolean array.
   */
  std::string packBoolArray(std::vector<std::string> bV);

  /**
   * Pack an individual bytes[] into %Solidity format.
   * @param hexV The array of hex strings that represent raw bytes.
   * @return The packed bytes array.
   */
  std::string packBytesArray(std::vector<std::string> hexV);

  /**
   * Pack an individual string[] into %Solidity format.
   * @param strV The array of hex strings that represent UTF-8 formatted strings.
   * @return The packed string array.
   */
  std::string packStringArray(std::vector<std::string> strV);

  /**
   * Pack multiple types into %Solidity format at once.
   * Always use this function to pack more than one type, as it correctly
   * sets things like offsets and lengths with mixed data.
   * Each arg has to be a JSON object with "type" and "value" (or "t" and "v")
   * and will be encoded in order.
   * ALL VALUES MUST BE STRINGS, INCLUDING NUMBERS.
   * Example:
   *
   *     json j = {
   *       { {"type", "string"}, {"value", "234"} },
   *       { {"t", "uint256"}, {"v", "234"} },
   *       { {"t", "bool"}, {"v", "true"} },
   *       { {"t", "bool"}, {"v", "0"} }
   *     };
   *
   * @param args The data to be packed, except `function`.
   * @param func (optional) The full function signature to be packed, if it exists.
   */
  std::string packMulti(json args, std::string func = "");
};

#endif  // SOLIDITY_H
