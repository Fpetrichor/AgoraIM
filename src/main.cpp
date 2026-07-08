#include "agora/net/buffer.h"
#include "agora/base/logger.h"
#include <iostream>

int main() {
    LOG_INFO("=== Buffer 修复测试 ===");

    agora::net::Buffer buf;

    // 测试 writeFd 消费数据
    buf.append("hello world");
    std::cout << "before retrieve: " << buf.readableBytes() << std::endl;
    
    std::string str = buf.retrieveAsString(2);
    std::cout << "retrieved: '" << str << "'" << std::endl;
    std::cout << "after retrieve: " << buf.readableBytes() << std::endl;

    buf.append(" world");
    std::cout << "final: '" << buf.retrieveAllAsString() << "'" << std::endl;

    // 测试大容量扩容
    std::string big(100000, 'a');
    buf.append(big);
    std::cout << "big readable: " << buf.readableBytes() << std::endl;

    return 0;
}