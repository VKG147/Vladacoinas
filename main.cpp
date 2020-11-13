#include <iostream>
#include "vcoin.h"
#include <omp.h>
#include <chrono>
#include <algorithm>

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

    const std::string miners[5] = { "1A", "1B", "1C", "1D", "1E" };
    long long minTime = LLONG_MAX, maxTime = 0;
    double totalMineTime = 0;
    while (!transactions.empty()) {
        int winnerIndex = 0;
        auto start = std::chrono::steady_clock::now();
        auto end = std::chrono::steady_clock::now();
#pragma omp parallel default(none) shared(chain, users, transactions, miners, winnerIndex, start, end, minTime, maxTime, std::cout) num_threads(5)
        {
            VUsers pUsers(users);
            VTransactions pTransactions(transactions);

            VBlock block;
            transferTransactionsToBlock(pUsers, pTransactions, block);

            std::cout << miners[omp_get_thread_num()] + " mining..\n";
            Miner::mine(block, &chain, omp_get_thread_num() * 1000);
            if (chain.insert(block)) {
                end = std::chrono::steady_clock::now();
                maxTime = std::max(maxTime, std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count());
                minTime = std::min(minTime, std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count());
                updateUsersBalance(users, block.transactions);
                IO::writeUsersToFile(USERS_DATA_PATH, pUsers);
                IO::writeTransactionsToFile(TRANSACTIONS_DATA_PATH, pTransactions);
                winnerIndex = omp_get_thread_num();
            }
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        totalMineTime += elapsed.count();

        std::cout << miners[winnerIndex] << " has finished mining in " << 1.0*elapsed.count()/1000 << "s!\n";
        std::cout << "========MINED BLOCK========\n";
        chain.get(chain.head()).printHeader();
        std::cout << "===========================\n";


        users = IO::getUsersFromFile(USERS_DATA_PATH);
        transactions = IO::getTransactionsFromFile(TRANSACTIONS_DATA_PATH);

        std::cout << "Remaining transactions: " << transactions.size() << "\n\n";
    }

    std::cout << "Average block mine time: " << totalMineTime/chain.size()/1000 << "s\n";
    std::cout << "Minimum block mine time: " << 1.0*minTime/1000 << "s\n";
    std::cout << "Maximum block mine time: " << 1.0*maxTime/1000 << "s\n\n";
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