#include <iostream>
#include "vcoin.h"

#define USERS_DATA_PATH "users.dat"
#define TRANSACTIONS_DATA_PATH "transactions.dat"

using namespace VCoin;

int main(int argc, char** argv) {
    VUsers users;
    VTransactions transactions;

    //users = VCoin::IO::getUsersFromFile(USERS_DATA_PATH);
    //transactions = IO::getTransactionsFromFile(TRANSACTIONS_DATA_PATH);

    IO::genRandUsers(users, 1000, 100, 1000000);
    IO::writeUsersToFile(USERS_DATA_PATH, users);

    IO::genRandTransactions(transactions, users, 10000, 1, 10000, 3600*7);
    IO::writeTransactionsToFile(TRANSACTIONS_DATA_PATH, transactions);

    VBlock genesisBlock;
    genesisBlock.prevBlock = "";
    genesisBlock.diffTarget = 4;

    Miner miner;
    miner.mine(genesisBlock);

    BlockChain chain = BlockChain(genesisBlock);

    while (!transactions.empty()) {
        VBlock block;
        transferTransactionsToBlock(transactions, block);
        block.merkleRootHash = getMerkleRoot(block.transactions);

        Miner miner;
        miner.mine(block);

        if (Miner::hashMeetsTarget(VHasher::getHash(block.toHex()), block.diffTarget)) {
            chain.insert(block);
            updateUsersBalance(users, transactions);
            IO::writeUsersToFile(USERS_DATA_PATH, users);
            IO::writeTransactionsToFile(TRANSACTIONS_DATA_PATH, transactions);
        }

        std::cout << "Remaining transactions: " << transactions.size() << std::endl;
    }

    int i = chain.size();
    std::string currentBlock = chain.head();
    while (i > 0) {
        std::cout << "Block " << i << ": " << currentBlock << std::endl;
        currentBlock = chain.get(currentBlock).prevBlock;
        i--;
    }

    return 0;
}