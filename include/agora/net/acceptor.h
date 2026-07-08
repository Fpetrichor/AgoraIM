#pragma once

#include "agora/base/noncopyable.h"
#include "agora/net/socket.h"
#include "agora/net/channel.h"
#include <functional>

namespace agora::net {

class EventLoop;
class InetAddress;

/**
 * Acceptor 负责监听端口并接受新连接
 * 
 * 封装监听 socket 的生命周期：
 * 1. 创建 socket → bind → listen
 * 2. 注册到 EventLoop（epoll）
 * 3. 有新连接时 accept，通过回调交给上层
 */
class Acceptor : public NonCopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    /**
     * 开始监听
     * 调用后，监听 fd 进入 epoll，等待连接
     */
    void listen();

    /**
     * 设置新连接回调
     * 参数：新连接的 fd，客户端地址
     */
    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    bool listening() const { return listening_; }

private:
    /**
     * 监听 fd 可读时的回调（内部使用）
     * 调用 accept() 接受新连接
     */
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;      // 监听 socket
    Channel acceptChannel_;    // 监听 fd 的 Channel
    bool listening_;
    
    NewConnectionCallback newConnectionCallback_;
};

} // namespace agora::net