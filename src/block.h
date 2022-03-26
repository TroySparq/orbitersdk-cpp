#ifndef BLOCK_H
#define BLOCK_H

#include <include/web3cpp/devcore/Common.h>
#include <include/web3cpp/ethcore/TransactionBase.h>
#include "utils.h"

class Block {
    private:

        // Block header = 48 * 5 == 240 Bytes.
        // _blockHash itself is not incuded in header
        std::string _blockHash;
        // HEADER
        dev::u256 _prevBlockHash;
        dev::u256 _timestamp;
        dev::u256 _txCount;
        dev::u256 _nHeight;
        // Data should be at max 32 bytes.
        dev::u256 _blockData;
        // HEADER END
        // Block contents.
        std::vector<dev::eth::TransactionBase> _transactions;

    public:

        std::string blockHash()       { return _blockHash; };
        dev::u256 prevBlockHash()   { return _prevBlockHash; };
        dev::u256 timestamp()       { return _timestamp; };
        dev::u256 txCount()         { return _txCount; };
        dev::u256 nHeight()         { return _nHeight; };
        dev::u256 blockData()       { return _blockData; };
        std::vector<dev::eth::TransactionBase> transactions() { return _transactions; };

        // Serialize to protobuf string.
        // serializeToString() also creates the blockhash! so we call it for both purposes.
        std::string serializeToString();
        // Serialize from protobufstring.
        bool serializeFromString(std::string blockBytes);

        bool addTx(dev::eth::TransactionBase tx);

        void submitBlock();

        Block(std::string blockBytes);
};



#endif // BLOCK_H