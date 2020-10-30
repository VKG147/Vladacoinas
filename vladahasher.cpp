#include "vladahasher.h"
#include <iostream>
#include <fstream>

std::string Vladahasher::getHash(const std::string & input) {
    auto wordBlocks = getWordBlocks(input);

    uint32_t hash[8] = { // No avalanche
            static_cast<uint32_t>(sqrt(2)*pow(2, 32)),
            static_cast<uint32_t>(sqrt(3)*pow(2, 32)),
            static_cast<uint32_t>(sqrt(5)*pow(2, 32)),
            static_cast<uint32_t>(sqrt(7)*pow(2, 32)),
            static_cast<uint32_t>(sqrt(11)*pow(2, 32)),
            static_cast<uint32_t>(sqrt(13)*pow(2, 32)),
            static_cast<uint32_t>(sqrt(17)*pow(2, 32)),
            static_cast<uint32_t>(sqrt(19)*pow(2, 32)),
    };
    for (int i = 0; i < wordBlocks.size(); ++i) {
        std::deque<uint32_t> ms = getMessageSchedule(wordBlocks[i]);

        uint32_t t1, t2;
        for (int j = 0; j < ms.size(); ++j) {
            t1 = op3(hash[4]) + choice(hash[4], hash[5], hash[6]) + hash[7] + ms[j] + kContants[j%8];
            t2 = op4(hash[0]) + maj(hash[0], hash[1], hash[2]);

            for (int k = 1; k < 8; ++k) {
                hash[k] = hash[k-1];
            }

            hash[0] = t1 + t2;
            hash[4] += t1;
        }


        for (int j = 0; j < ms.size(); ++j) {
            hash[j%8] ^= ms[j] ^ kContants[j%8];
        }
    }

    std::string hashStr = "";
    for (int i = 0; i < 8; ++i) {
        std::bitset<32> word(hash[i]);
        for (int j = 0; j < 8; ++j) {
            std::bitset<4> hexChar(0);
            for (int k = 0; k < 4; ++k) {
                hexChar[k] = word[j*4+k];
            }
            std::stringstream sr;
            sr << std::hex << hexChar.to_ulong();
            hashStr += sr.str();
            sr.clear();
        }
    }

    return hashStr;
}

std::deque<std::deque<uint32_t>> Vladahasher::getWordBlocks(const std::string & input) {
    std::deque<std::deque<uint32_t>> wordBlocks;

    if (input.length() == 0) {
        std::deque<uint32_t> wordBlock;
        wordBlock.push_back(0);
        wordBlock.push_back(1);
        while (wordBlock.size() < 16) {
            wordBlock.push_back(0);
        }
        wordBlocks.push_back(wordBlock);
    }

    std::deque<std::bitset<8>> strBits8;
    for (int i = 0; i < input.length(); ++i) {
        std::bitset<8> charBits(input[i]);
        strBits8.push_back(charBits);
    }

    std::deque<std::bitset<32>> strBits32;
    for (int i = 0; i < strBits8.size(); i+=4) {
        std::bitset<32> bits32(0);
        for (int j = i; j-i < 4 && j < strBits8.size(); ++j) {
            for (int k = 0; k < 8; ++k) {
                bits32[(j-i)*8+k] = strBits8[j][k];
            }
        }
        strBits32.push_back(bits32);
    }

    strBits8.clear();

    std::deque<uint32_t> wordBlock;
    for (int i = 0; i < strBits32.size(); i+=16) {
        for (int j = i; j - i < 16 && j < strBits32.size(); ++j) {
            wordBlock.push_back(static_cast<int>(strBits32[j].to_ulong()));
        }

        if (wordBlock.size() < 15)
            wordBlock.push_back(1);

        while (wordBlock.size() < 16) {
            if (wordBlock.size() == 15) {
                wordBlock.push_back(input.length());
            }
            else {
                wordBlock.push_back(0);
            }
        }

        wordBlocks.push_back(wordBlock);
        wordBlock.clear();
    }

    return wordBlocks;
}

std::deque<uint32_t> Vladahasher::getMessageSchedule(const std::deque<uint32_t>& words) {
    std::deque<uint32_t> ms(words);

    for (int i = words.size(); i < 64; ++i) {
        uint32_t word = op1(ms[i-16]) + ms[i-12] + op2(ms[i-7]) + ms[i-3];
        ms.push_back(word);
    }

    return ms;
}

uint32_t Vladahasher::rotr(uint32_t word, int32_t shift) {
    int s = shift>=0? shift%32 : -((-shift)%32);
    return (word>>shift) | (word<<(32-shift));
}

uint32_t Vladahasher::op1(uint32_t word) {
    uint32_t a = rotr(word, 14);
    uint32_t b = word >> 3;
    uint32_t c = rotr(word, 7);

    return a^b^c;
}
uint32_t Vladahasher::op2(uint32_t word) {
    uint32_t a = word >> 7;
    uint32_t b = rotr(word, 20);
    uint32_t c = rotr(word, 17);

    return a^b^c;
}
uint32_t Vladahasher::op3(uint32_t word) {
    uint32_t a = rotr(word, 2);
    uint32_t b = rotr(word, 7);
    uint32_t c = rotr(word, 21);

    return a^b^c;
}
uint32_t Vladahasher::op4(uint32_t word) {
    uint32_t a = rotr(word, 15);
    uint32_t b = rotr(word, 13);
    uint32_t c = rotr(word, 2);

    return a^b^c;
}

uint32_t Vladahasher::choice(uint32_t w1, uint32_t w2, uint32_t w3) {
    std::bitset<32> b1(w1);
    std::bitset<32> b2(w2);
    std::bitset<32> b3(w3);
    std::bitset<32> b;

    for (int i = 0; i < 32; ++i) {
        if (b1[i]) b[i] = b2[i];
        else b[i] = b3[i];
    }

    return static_cast<uint32_t>(b.to_ulong());
}

uint32_t Vladahasher::maj(uint32_t w1, uint32_t w2, uint32_t w3) {
    std::bitset<32> b1(w1);
    std::bitset<32> b2(w2);
    std::bitset<32> b3(w3);
    std::bitset<32> b;

    for (int i = 0; i < 32; ++i) {
        if ((b1[i]&&b2[i])||(b1[i]&&b3[i])||(b2[i]&&b3[i]))
            b[i] = 1;
        else b[i] = 0;
    }

    return static_cast<uint32_t>(b.to_ulong());
}

std::string Vladahasher::hashToString(const std::bitset<512> &hash) {
    std::stringstream res;

    for (int i = 0; i < 512; i += 8) {
        std::bitset<4> hexChar;
        for (int j = 0; j < 4; ++j) {
            hexChar[j] = hash[i+j];
        }

        res << std::hex << std::uppercase << hexChar.to_ulong();
    }

    return res.str();
}