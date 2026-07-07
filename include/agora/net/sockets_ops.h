#pragma once 

#include "agora/net/inet_address.h"
#include <cstdint>

namespace agora::net::sockets {

// Create a non-blocking socket file descriptor, abort if any error.
int createNonblockingOrDie();

// Bind a socket to an address, abort if any error.
void bindOrDie(int sockfd, const InetAddress& addr);

// Mark a socket as listening, abort if any error.
void listenOrDie(int sockfd);

// Accept a connection on a socket, abort if any error.
int accept(int sockfd, InetAddress* addr);

// close a socket, abort if any error.
void close(int sockfd);

// Read data from a socket, abort if any error.
ssize_t read(int sockfd, void* buf, size_t count);

// Read data from a socket into multiple buffers, abort if any error.
ssize_t readv(int sockfd, const struct iovec* iov, int iovcnt);

// Write data to a socket, abort if any error.
ssize_t write(int sockfd, const void* buf, size_t count);

// Shut down the write side of a socket, abort if any error.
void shutdownWrite(int sockfd);

// set SO_REUSEADDR on a socket, abort if any error.
void setReuseAddr(int sockfd, bool on);

// set SO_REUSEPORT on a socket, abort if any error.
void setReusePort(int sockfd, bool on);

// set TCP_NODELAY on a socket, abort if any error.
void setKeepAlive(int sockfd, bool on);

// get socket error, abort if any error.
int getSocketError(int sockfd);

} // namespace agora::net::sockets