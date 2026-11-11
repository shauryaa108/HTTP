
#pragma once

#include <sys/types.h>
#include <cstddef>
#include <string>
class ClientConnection {
public:
    explicit ClientConnection(int socketFd = -1);

    ~ClientConnection();

    // A socket descriptor must have exactly one owner.
    ClientConnection(const ClientConnection&) = delete;
    ClientConnection& operator=(const ClientConnection&) = delete;

    // Ownership can be transferred.
    ClientConnection(ClientConnection&& other) noexcept;
    ClientConnection& operator=(ClientConnection&& other) noexcept;

    bool valid() const;

    // Configure receive timeout.
    bool setReadTimeout(long seconds, long microseconds = 0);

    // Receive data from the client.
    ssize_t receive(char* buffer, size_t size);

    // Send all bytes, handling partial sends.
    bool sendAll(const std::string& data);

    // Explicitly release the socket.
    void close();

private:
    int socketFd_;
};
