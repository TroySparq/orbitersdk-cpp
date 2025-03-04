#ifndef DB_H
#define DB_H

#include <cstring>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include <rocksdb/db.h>
#include <rocksdb/write_batch.h>

#include "utils.h"

/**
 * Namespace for accessing database prefixes. Values are:
 * - blocks = "0001"
 * - blockHeightMaps = "0002"
 * - nativeAccounts = "0003"
 * - txToBlocks = "0004"
 * - rdPoS = "0005"
 * - contracts = "0006"
 * - contractManager = "0007"
 */
namespace DBPrefix {
  const std::string blocks = std::string("\x00\x01", 2);
  const std::string blockHeightMaps = std::string("\x00\x02", 2);
  const std::string nativeAccounts = std::string("\x00\x03", 2);
  const std::string txToBlocks = std::string("\x00\x04", 2);
  const std::string rdPoS = std::string("\x00\x05", 2);
  const std::string contracts = std::string("\x00\x06", 2);
  const std::string contractManager = std::string("\x00\x07", 2);
};

/// Struct for a database connection/endpoint.
struct DBServer {
  std::string host;     ///< Database host/address.
  std::string version;  ///< Database version.

  /**
   * Constructor.
   * @param host The database's host/address.
   * @param version The database's version string.
   */
  DBServer(std::string host, std::string version) : host(host), version(version) {};
};

/// Struct for a database entry (key/value).
struct DBEntry {
  std::string key;    ///< Entry key.
  std::string value;  ///< Entry value.

  /**
   * Constructor.
   * @param key The entry's key.
   * @param value The entry's value.
   */
  DBEntry(std::string key, std::string value) : key(key), value(value) {};
};

/**
 * Struct for a database batch request.
 * Several requests can be grouped here to be issued at once.
 */
struct DBBatch {
  std::vector<DBEntry> puts;      ///< List of entries to insert.
  std::vector<std::string> dels;  ///< List of entries to delete.
  uint64_t id;                    ///< Unique identifier for the batch request.
  bool continues; ///< Indicates if writing will continue after gRPC request limit of *2^32 bytes*.
};

/**
 * Abstraction of a [RocksDB](https://github.com/facebook/rocksdb) database.
 * Keys begin with prefixes that separate entries in several categories (see DBPrefix).
 */
class DB {
  private:
    rocksdb::DB* db;              ///< Pointer to the database object itself.
    rocksdb::Options opts;        ///< Struct with options for managing the database.
    mutable std::mutex batchLock; ///< Mutex for managing read/write access to batch operations.

  public:
    /**
     * Constructor. Automatically creates the database if it doesn't exist. Throws on error.
     * @param path The database's filesystem path (relative to the binary's current working directory).
     */
    DB(const std::string path);

    /**
     * Destructor. Automatically closes the database so it doesn't leave a LOCK file behind.
     */
    ~DB() { this->close(); }

    /**
     * Close the database. "Closing" a RocksDB database is just deleting its object.
     * @return `true` if the database is closed successfully, `false` otherwise.
     */
    inline bool close() { delete this->db; this->db = nullptr; return (this->db == nullptr); }

    /**
     * Check if a key exists in the database.
     * @param key The key to search for.
     * @param pfx (optional) The prefix to search for. Defaults to an empty string.
     * @return `true` if the key exists, `false` otherwise.
     */
    bool has(const std::string& key, const std::string& pfx = "");

    /**
     * Get a value from a given key in the database.
     * @param key The key to search for.
     * @param pfx (optional) The prefix to search for. Defaults to an empty string.
     * @return The requested value, or an empty string if the key doesn't exist.
     */
    std::string get(const std::string& key, const std::string& pfx = "") const;

    /**
     * Insert an entry into the database.
     * @param key The key to insert.
     * @param value The value to insert.
     * @param pfx (optional) The prefix to insert the key into. Defaults to an empty string.
     * @return `true` if the insert is successful, `false` otherwise.
     */
    bool put(const std::string& key, const std::string& value, const std::string& pfx = "") const;

    /**
     * Delete an entry from the database.
     * @param key The key to delete.
     * @param pfx (optional) The prefix to delete the key from. Defaults to an empty string.
     * @return `true` if the deletion is successful, `false` otherwise.
     */
    bool del(const std::string& key, const std::string& pfx = "") const;

    /**
     * Do several put and/or delete operations in one go.
     * @param batch The batch object with the operations to be done.
     * @param pfx (optional) The prefix to operate on. Defaults to an empty string.
     * @return `true` if all operations were successful, `false` otherwise.
     */
    bool putBatch(const DBBatch& batch, const std::string& pfx = "") const;

    /**
     * Get all entries from a given prefix.
     * @param pfx The prefix string to get entries from. Works with strings.
     * @param keys (optional) A list of keys to filter values from.
     *             Defaults to an empty list (same as "get all entries").
     * @return The list of database entries.
     */
    std::vector<DBEntry> getBatch(
      const rocksdb::Slice& pfx, const std::vector<std::string>& keys = {}
    ) const;
};

#endif // DB_H
