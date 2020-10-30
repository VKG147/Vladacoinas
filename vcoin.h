#pragma once
#include <iostream>
#include <string>

namespace VCoin {
    struct VBlock {
        std::string prevBlock_;
        time_t timeStamp_;
        std::string version_ = "v0.1";
        std::string merkelRootHash_;
        uint64_t nonce;
        uint8_t diffTarget = 2;



        std::string toString()
        {
            return prevBlock_ + "\n" + std::to_string(timeStamp_);
        }
    };
}


