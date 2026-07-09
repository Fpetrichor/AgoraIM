#pragma once

#include "agora/base/noncopyable.h"
#include "agora/net/inet_address.h"
#include <functional>
#include <memory>

namespace agora::net {

class EventLoop;
class Channel;

class Connector : public NonCopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;
    
    enum class State {
        kDisconnected, 
        kConnecting,
        kConnected 
    };

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void start();
    void restart();
    void stop();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    const InetAddress& serverAddress() const { return serverAddr_; }

private:
    void setState(State s);
    
    void startInLoop();
    void stopInLoop();           // 新增
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry();
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    InetAddress serverAddr_;
    bool connect_;
    State state_;
    std::unique_ptr<Channel> channel_;

    NewConnectionCallback newConnectionCallback_;
};

} // namespace agora::net