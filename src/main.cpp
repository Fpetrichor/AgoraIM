#include <iostream>
#include "agora/base/current_thread.h"

int main() {
    std::cout << agora::CurrentThread::tid() << '\n';
    return 0;
}