#include <iostream>
#include "vladahasher.h"

Vladahasher hasher;
int main() {
    std::cout << hasher.getHash("aa");

    return 0;
}