#pragma once

#include "agora/base/noncopyable.h"
#include <vector>
#include <string>
#include <cstdint>

static const char kCRLF[] = "\r\n";

namespace agora::net {

/**
 * 网络缓冲区
 * 
 * 采用 Muduo 经典设计：
 * - 内部使用 vector<char> 管理内存
 * - 维护 readerIndex / writerIndex
 * - 支持自动扩容和内存复用
 */
class Buffer : public NonCopyable {
public:
    static constexpr size_t kCheapPrepend = 8;   // 预留头部空间
    static constexpr size_t kInitialSize = 1024; // 初始大小

    Buffer();
    ~Buffer() = default;

    // ========== 容量查询 ==========

    /** 可读字节数 */
    size_t readableBytes() const;

    /** 可写字节数 */
    size_t writableBytes() const;

    /** 可前置字节数（已读取区域） */
    size_t prependableBytes() const;

    // ========== 读取接口 ==========

    /** 返回可读数据的起始地址（不移动索引） */
    const char* peek() const;

    /** 读取 len 字节，移动 readerIndex */
    void retrieve(size_t len);

    /** 读取所有数据 */
    void retrieveAll();

    /** 读取 len 字节，返回字符串 */
    std::string retrieveAsString(size_t len);

    /** 读取所有数据，返回字符串 */
    std::string retrieveAllAsString();

    // ========== 写入接口 ==========

    /** 追加数据 */
    void append(const char* data, size_t len);
    void append(const std::string& str);

    /** 确保可写空间 >= len */
    void ensureWritableBytes(size_t len);

    // ========== 网络接口 ==========

    /**
     * 从 fd 读取数据到 Buffer
     * @return 读取的字节数，出错返回 -1
     */
    ssize_t readFd(int fd, int* savedErrno);

    /**
     * 从 Buffer 写入数据到 fd
     * @return 写入的字节数，出错返回 -1
     */
    ssize_t writeFd(int fd, int* savedErrno);

    const char* findCRLF() const;
    void retrieveUntil(const char* end);

private:
    /** 返回 buffer_ 起始地址 */
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }

    /** 扩容/整理空间 */
    void makeSpace(size_t len);

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

} // namespace agora::net