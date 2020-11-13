#include <iostream>
#include "vcoin.h"
#include <omp.h>
#include <chrono>

#define USERS_DATA_PATH "users.dat"
#define TRANSACTIONS_DATA_PATH "transactions.dat"

using namespace VCoin;

int main(int argc, char** argv) {
    VUsers users;
    VTransactions transactions;

    IO::genRandUsers(users, 1000, 100, 1000000);
    IO::writeUsersToFile(USERS_DATA_PATH, users);
    IO::genRandTransactions(transactions, users, 1000, 1, 10000, 3600*7);
    IO::writeTransactionsToFile(TRANSACTIONS_DATA_PATH, transactions);

    VBlock genesisBlock;
    std::cout << "Mining genesis block...\n";
    Miner::mine(genesisBlock);
    BlockChain chain = BlockChain(genesisBlock);
    std::cout << "Genesis block hash: " << VHasher::getHash(genesisBlock.toHex()) << "\n\n";

    validateTransactions(transactions);

    const std::string miners[5] = { "A", "B", "C", "D", "E" };
    double totalMineTime = 0;
    while (!transactions.empty()) {
        int winnerIndex = 0;
        auto start = std::chrono::steady_clock::now();
        auto end = std::chrono::steady_clock::now();
#pragma omp parallel default(none) shared(chain, users, transactions, miners, winnerIndex, end, std::cout) num_threads(5)
        {
            VUsers pUsers(users);
            VTransactions pTransactions(transactions);

            VBlock block;
            transferTransactionsToBlock(pUsers, pTransactions, block);

            std::cout << std::to_string(chain.size()) + miners[omp_get_thread_num()] + " mining..\n";
            Miner::mine(block, &chain, omp_get_thread_num() * 10000);
            if (chain.insert(block)) {
                end = std::chrono::steady_clock::now();
                updateUsersBalance(users, block.transactions);
                IO::writeUsersToFile(USERS_DATA_PATH, pUsers);
                IO::writeTransactionsToFile(TRANSACTIONS_DATA_PATH, pTransactions);
                winnerIndex = omp_get_thread_num();
            }
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        totalMineTime += elapsed.count();
        std::cout << chain.size() << miners[winnerIndex] << " has finished mining in " << 1.0*elapsed.count()/1000 << "s!\n";
        std::cout << "========MINED BLOCK========\n";
        chain.get(chain.head()).printHeader();
        std::cout << "===========================\n";


        users = IO::getUsersFromFile(USERS_DATA_PATH);
        transactions = IO::getTransactionsFromFile(TRANSACTIONS_DATA_PATH);

        std::cout << "Remaining transactions: " << transactions.size() << "\n\n";
    }

    std::cout << "Average block mine time: " << totalMineTime/chain.size()/1000 << "s\n\n";

    std::cout << "\nFinal blockchain:\n";
    int currentBlockIndex = chain.size();
    std::string currentBlock = chain.head();
    while (currentBlockIndex > 0)
    {
        std::cout << "Block " << currentBlockIndex << ": " << currentBlock << "\n";
        currentBlock = chain.get(currentBlock).prevBlock;
        currentBlockIndex--;
    }

    return 0;
}