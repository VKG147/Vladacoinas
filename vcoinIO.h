#pragma once
#include "vcoin.h"
#include <fstream>
#include <sstream>
#include <random>

namespace VCoin { namespace IO
{
    VUsers getUsersFromFile(std::string fpath) {
        VUsers users;

        std::ifstream in(fpath);
        if (!in.is_open()) throw std::runtime_error("Failed to open file " + fpath);

        std::string line;
        while (std::getline(in, line)) {
            if (line == "") break;
            std::stringstream sstream(line);
            VUser user;
            sstream >> user.key >> user.name >> user.balance;
            users[user.key] = user;
        } in.close();

        return users;
    }

    VTransactions getTransactionsFromFile(std::string fpath) {
        VTransactions transactions;

        std::ifstream in(fpath);
        if (!in.is_open()) throw std::runtime_error("Failed to open file " + fpath);

        std::string line;
        while (std::getline(in, line)) {
            if (line == "") break;
            std::stringstream sstream(line);
            VTransaction transaction;
            sstream >> transaction.id >> transaction.receiver >> transaction.sender >> transaction.sum >> transaction.timestamp;
            transaction.id = VHasher::getHash(transaction.toString());
            transactions.push_back(transaction);
        } in.close();

        return transactions;
    }

    void writeUsersToFile(std::string fpath, VUsers users, bool append = false) {
        std::ofstream out;
        if (append) out.open(fpath, std::ofstream::app);
        else out.open(fpath);

        for (auto it = users.begin(); it != users.end(); ++it) {
            out << it->second.key << " " << it->second.name << " " << it->second.balance << "\n";
        } out.close();
    }

    void writeTransactionsToFile(std::string fpath, VTransactions transactions, bool append = false) {
        std::ofstream out;
        if (append) out.open(fpath, std::ofstream::app);
        else out.open(fpath);

        for (auto it = transactions.begin(); it != transactions.end(); ++it) {
            out << it->id << " " << it->receiver << " " << it->sender << " " << it->sum << " " << it->timestamp << "\n";
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
            transaction.timestamp = std::time(0) - timeDist(generator);
            transaction.id = VHasher::getHash(transaction.toString());
            transactions.push_back(transaction);
        }
    }
} }
