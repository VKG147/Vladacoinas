#pragma once

#include <string>
#include <sstream>
#include <deque>
#include <bitset>
#include <cmath>
#include <limits>
#include <iomanip>

class Vladahasher {
private:
    static constexpr uint32_t kHashSize = 512;

    const uint32_t kContants[8] = {
            static_cast<uint32_t>((sqrt(2)/ceil(sqrt(2)))*(std::numeric_limits<uint32_t>::max())),
            static_cast<uint32_t>((sqrt(3)/ceil(sqrt(3)))*(std::numeric_limits<uint32_t>::max())),
            static_cast<uint32_t>((sqrt(5)/ceil(sqrt(5)))*(std::numeric_limits<uint32_t>::max())),
            static_cast<uint32_t>((sqrt(7)/ceil(sqrt(7)))*(std::numeric_limits<uint32_t>::max())),
            static_cast<uint32_t>((sqrt(11)/ceil(sqrt(11)))*(std::numeric_limits<uint32_t>::max())),
            static_cast<uint32_t>((sqrt(13)/ceil(sqrt(13)))*(std::numeric_limits<uint32_t>::max())),
            static_cast<uint32_t>((sqrt(17)/ceil(sqrt(17)))*(std::numeric_limits<uint32_t>::max())),
            static_cast<uint32_t>((sqrt(19)/ceil(sqrt(17)))*(std::numeric_limits<uint32_t>::max())),
    };

    // Helper methods
    uint32_t rotr(uint32_t word, int32_t shift);
    uint32_t choice(uint32_t w1, uint32_t w2, uint32_t w3);
    uint32_t maj(uint32_t w1, uint32_t w2, uint32_t w3);

    uint32_t op1(uint32_t word);
    uint32_t op2(uint32_t word);
    uint32_t op3(uint32_t word);
    uint32_t op4(uint32_t word);

    std::deque<std::deque<uint32_t>> getWordBlocks(const std::string & input);
    std::deque<uint32_t> getMessageSchedule(const std::deque<uint32_t>& words);

    // Utils
    std::string hashToString(const std::bitset<kHashSize> & hash);
    std::bitset<kHashSize> hashStringToBitset(const std::string & hashString);
public:
    std::string getHash(const std::string & input);
};
