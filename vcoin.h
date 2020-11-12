#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <deque>
#include "vhasher.h"


namespace VCoin
{
#define VUsers std::map<std::string, VUser>
#define VTransactions std::deque<VTransaction>

    const uint32_t kTransactionsPerBlock = 100;
    const uint8_t kCurrentDifficulty = 2;

    struct VUser {
        std::string key;
        std::string name;
        double balance;
    };

    struct VTransaction {
        std::string id;
        std::string sender;
        std::string receiver;
        double sum;
        time_t timestamp;

        std::string toHex() {
            std::stringstream transHexStream;
            transHexStream << sender << receiver << sum << timestamp;
            return transHexStream.str();
        }
    };

    struct VBlock {
        std::string prevBlock;
        time_t timeStamp;
        std::string version = "v0.1";
        std::string merkleRootHash;
        VTransactions transactions;
        uint64_t nonce = 0;
        uint8_t diffTarget = VCoin::kCurrentDifficulty;

        std::string toHex()
        {
            std::stringstream blockHexStream;
            blockHexStream << std::hex << prevBlock << timeStamp << version << merkleRootHash;
            for (auto & transaction : transactions) {
                blockHexStream << transaction.toHex();
            }
            blockHexStream << nonce << diffTarget;

            return blockHexStream.str();
        }
    };

    bool compareTransactions(const VTransaction& a, const VTransaction& b) {
        return a.id > b.id;
    }

    std::string getMerkleRoot(const VTransactions& transactions) {
        VTransactions transSorted(transactions);
        std::sort(transSorted.begin(), transSorted.end(), compareTransactions);
        std::deque<std::string> merkleTree;
        for (auto & it : transSorted) {
            merkleTree.push_back(VHasher::getHash(it.toHex()));
        }

        while (merkleTree.size() != 1) {
            for (int i = 0; i < merkleTree.size(); i+=2) {
                if (i == merkleTree.size()-1) { // if last hash
                    std::string hash = VHasher::getHash(merkleTree[i]+merkleTree[i]);
                    merkleTree[i] = hash;
                }
                else {
                    std::string hash = VHasher::getHash(merkleTree[i]+merkleTree[i+1]);
                    merkleTree.erase(merkleTree.begin() + i, merkleTree.begin() + i + 2);
                    merkleTree.insert(merkleTree.begin() + i, hash);
                }
            }
        }

        return merkleTree[0];
    }

    void transferTransactionsToBlock(VUsers& users, VTransactions& transactions, VBlock& block) {
        std::default_random_engine generator;

        VUsers tmpUsers(users);
        while (block.transactions.size() < kTransactionsPerBlock && !transactions.empty()) {
            std::uniform_int_distribution<int> transDist(0, transactions.size()-1);
            uint32_t randIndex = transDist(generator);

            if (tmpUsers[transactions[randIndex].sender].balance >= transactions[randIndex].sum) {
                block.transactions.push_back(transactions[randIndex]);
                tmpUsers[transactions[randIndex].sender].balance -= transactions[randIndex].sum;
                tmpUsers[transactions[randIndex].receiver].balance += transactions[randIndex].sum;
            }

            transactions.erase(transactions.begin() + randIndex);
        }
    }

    void validateTransactions(VTransactions& transactions) {
        for (int i = 0; i < transactions.size(); ++i) {
            if (transactions[i].id != VHasher::getHash(transactions[i].toHex())) {
                std::cout << "Invalid transaction found!\n";
                transactions.erase(transactions.begin() + i);
            }
        }
    }

    void updateUsersBalance(VUsers& users, const VTransactions& transactions) {
        for (const auto & transaction : transactions) {
            users[transaction.sender].balance -= transaction.sum;
            users[transaction.receiver].balance += transaction.sum;
        }
    }

    class Miner
    {
    public:
        static bool hashMeetsTarget(std::string hash, char target) {
            for (char i = 0; i < target; ++i) {
                if (hash[(int)i] != '0') return false;
            }
            return true;
        }

        static void mine(VBlock& block, uint64_t seed = 0) {
            block.nonce = seed;
            do
            {
                block.timeStamp = std::time(nullptr);
                block.nonce++;

                if (block.nonce % 100 == 0) {
                    std::cout << block.nonce << std::endl;
                }
            }
            while (!hashMeetsTarget(VHasher::getHash(block.toHex()), block.diffTarget));

            std::cout << VHasher::getHash(block.toHex()) << std::endl;
        }
    };

    class BlockChain
    {
    private:
        std::map<std::string, VBlock> blockChain;
        std::string chainHead;
        size_t _size;

        bool hashMeetsTarget(std::string hash, char target) {
            for (char i = 0; i < target; ++i) {
                if (hash[(int)i] != '0') return false;
            }
            return true;
        }

    public:
        BlockChain(VBlock genesis) {
            if (!hashMeetsTarget(VHasher::getHash(genesis.toHex()), genesis.diffTarget)) throw;

            std::string hash = VHasher::getHash(genesis.toHex());
            blockChain[hash] = genesis;
            chainHead = hash;
            _size = 1;
        }

        size_t size() {
            return _size;
        }

        std::string head() {
            return this->chainHead;
        }

        VBlock get(const std::string& hash) {
            return blockChain[hash];
        }

        void insert(VBlock block) {
            std::string blockHash = VHasher::getHash(block.toHex());
            if (!hashMeetsTarget(blockHash, kCurrentDifficulty)) return;
            block.prevBlock = this->chainHead;
            blockChain[blockHash] = block;
            this->chainHead = blockHash;
            _size++;
            std::cout << "Size: " << _size << std::endl;
        }
    };
}

namespace VCoin { namespace IO
    {
        VUsers getUsersFromFile(const std::string& fpath) {
            VUsers users;

            std::ifstream in(fpath);
            if (!in.is_open()) throw std::runtime_error("Failed to open file " + fpath);

            std::string line;
            while (std::getline(in, line)) {
                if (line.empty()) break;
                std::stringstream sstream(line);
                VUser user;
                sstream >> user.key >> user.name >> user.balance;
                users[user.key] = user;
            } in.close();

            return users;
        }

        VTransactions getTransactionsFromFile(const std::string& fpath) {
            VTransactions transactions;

            std::ifstream in(fpath);
            if (!in.is_open()) throw std::runtime_error("Failed to open file " + fpath);

            std::string line;
            while (std::getline(in, line)) {
                if (line.empty()) break;
                std::stringstream sstream(line);
                VTransaction transaction;
                sstream >> transaction.id >> transaction.receiver >> transaction.sender >> transaction.sum >> transaction.timestamp;
                // Check if hash is valid here
                transaction.id = VHasher::getHash(transaction.toHex());
                transactions.push_back(transaction);
            } in.close();

            return transactions;
        }

        void writeUsersToFile(const std::string& fpath, const VUsers& users, bool append = false) {
            std::ofstream out;
            if (append) out.open(fpath, std::ofstream::app);
            else out.open(fpath);

            for (auto & user : users) {
                out << user.second.key << " " << user.second.name << " " << user.second.balance << "\n";
            } out.close();
        }

        void writeTransactionsToFile(const std::string& fpath, const VTransactions& transactions, bool append = false) {
            std::ofstream out;
            if (append) out.open(fpath, std::ofstream::app);
            else out.open(fpath);

            for (auto & transaction : transactions) {
                out << transaction.id << " " << transaction.receiver << " " << transaction.sender << " " << transaction.sum << " " << transaction.timestamp << "\n";
            } out.close();
        }

        void genRandUsers(VUsers& users, uint32_t count, double minBalance, double maxBalance) {
            users.clear();
            std::default_random_engine generator;

            const std::vector<std::string> randNames = {
                    "Tomas", "Matas", "Danielius", "Augustinas", "Viktoras", "Ernestas", "Adomas", "Darius", "Zydrunas", "Ignas",
                    "Elena", "Lilija", "Smilte", "Viktorija", "Urte", "Regina", "Kristina", "Leja", "Austeja", "Jolanta"
            };

            for (int i = 0; i < count; ++i) {
                VUser user;

                std::uniform_int_distribution<int> keyDist;
                user.key = VHasher::getHash(std::to_string(keyDist(generator)));
                std::uniform_real_distribution<double> balDist(minBalance, maxBalance);
                user.balance = balDist(generator);
                std::uniform_int_distribution<int> nameDist(0, randNames.size()-1);
                user.name = randNames[nameDist(generator)];

                users[user.key] = user;
            }
        }

        void genRandTransactions(VTransactions& transactions, const VUsers& users, uint32_t count, double minSum, double maxSum, uint32_t maxTransAge) {
            transactions.clear();
            std::default_random_engine generator;

            for (int i = 0; i < count; ++i) {
                VTransaction transaction;

                std::uniform_int_distribution<int> userDist(0, users.size()-1);
                auto randSender = users.begin();
                std::advance(randSender, userDist(generator));
                transaction.sender = randSender->second.key;
                auto randReceiver = users.begin();
                std::advance(randReceiver, userDist(generator));
                transaction.receiver = randReceiver->second.key;
                std::uniform_real_distribution<double> sumDist(minSum, maxSum);
                transaction.sum = sumDist(generator);
                std::uniform_int_distribution<int> timeDist(0, maxTransAge);
                transaction.timestamp = std::time(nullptr) - timeDist(generator);
                transaction.id = VHasher::getHash(transaction.toHex());
                transactions.push_back(transaction);
            }
        }
    } }

