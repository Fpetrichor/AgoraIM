#include "agora/net/buffer.h"
#include "agora/base/logger.h"

#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <cassert>
#include <cstring>
#include <algorithm>

namespace agora::net {

Buffer::Buffer()
    : buffer_(kCheapPrepend + kInitialSize),
      readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend) {}

size_t Buffer::readableBytes() const {
    return writerIndex_ - readerIndex_;
}

size_t Buffer::writableBytes() const {
    return buffer_.size() - writerIndex_;
}

size_t Buffer::prependableBytes() const {
    return readerIndex_;
}

const char* Buffer::peek() const {
    return begin() + readerIndex_;
}

void Buffer::retrieve(size_t len) {
    if (len < readableBytes()) {
        readerIndex_ += len;
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveAll() {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
}

std::string Buffer::retrieveAsString(size_t len) {
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readableBytes());
}

void Buffer::append(const char* data, size_t len) {
    ensureWritableBytes(len);
    std::memcpy(begin() + writerIndex_, data, len);
    writerIndex_ += len;
}

void Buffer::append(const std::string& str) {
    append(str.data(), str.size());
}

void Buffer::ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
        makeSpace(len);
    }
}

void Buffer::makeSpace(size_t len) {
    if (prependableBytes() + writableBytes() < len + kCheapPrepend) {
        buffer_.resize(writerIndex_ + len);
    } else {
        size_t readable = readableBytes();
        std::copy(begin() + readerIndex_,
                  begin() + writerIndex_,
                  begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
    }
}

ssize_t Buffer::readFd(int fd, int* savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];
    
    const size_t writable = writableBytes();
    
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    ssize_t n = ::readv(fd, vec, iovcnt);
    
    if (n < 0) {
        if(savedErrno){
            *savedErrno = errno;
        }
    } else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    
    return n;
}

ssize_t Buffer::writeFd(int fd, int* savedErrno) {
    ssize_t n = ::write(fd, peek(), readableBytes());
    
    if (n < 0) {
        if (savedErrno) *savedErrno = errno;
    }
    
    return n;
}


const char* Buffer::findCRLF() const {
    // 使用 peek() + readableBytes() 替代 beginWrite()
    const char* crlf = std::search(peek(), peek() + readableBytes(), kCRLF, kCRLF + 2);
    return crlf == peek() + readableBytes() ? nullptr : crlf;
}

void Buffer::retrieveUntil(const char* end) {
    retrieve(end - peek());
}

} // namespace agora::net