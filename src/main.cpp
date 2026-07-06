#include <iostream>

#include "agora/base/timestamp.h"

int main() {
    auto ts = agora::Timestamp::now();

    std::cout << ts.toString() << std::endl;

    return 0;
}