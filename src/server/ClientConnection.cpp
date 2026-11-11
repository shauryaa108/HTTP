
#include "ClientConnection.h"

#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <utility>
#include <iostream>

ClientConnection::ClientConnection(int socketFd)
    : socketFd_(socketFd) {
}

ClientConnection::~ClientConnection() {
    close();
}

ClientConnection::ClientConnection(
    ClientConnection&& other
) noexcept
    : socketFd_(other.socketFd_) {

    other.socketFd_ = -1;
}

ClientConnection& ClientConnection::operator=(
    ClientConnection&& other
) noexcept {

    if (this != &other) {
        close();

        socketFd_ = other.socketFd_;

        other.socketFd_ = -1;
    }

    return *this;
}

bool ClientConnection::valid() const {
    return socketFd_ >= 0;
}

bool ClientConnection::setReadTimeout(
    long seconds,
    long microseconds
) {
    if (!valid() ||
        seconds < 0 ||
        microseconds < 0 ||
        microseconds >= 1000000) {
        return false;
    }

    timeval timeout{seconds, microseconds};

    return setsockopt(
        socketFd_,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        sizeof(timeout)
    ) == 0;
}

ssize_t ClientConnection::receive(
    char* buffer,
    size_t size
) {
    if (!valid()) {
        errno = EBADF;
        return -1;
    }

    return recv(socketFd_, buffer, size, 0);
}

bool ClientConnection::sendAll(
    const std::string& data
) {
    if (!valid()) {
        errno = EBADF;
        return false;
    }

    size_t totalSent = 0;

    while (totalSent < data.size()) {

        ssize_t sent = send(
            socketFd_,
            data.data() + totalSent,
            data.size() - totalSent,
            MSG_NOSIGNAL
        );

        if (sent < 0) {
            if (errno == EINTR)
                continue;

            return false;
        }

        if (sent == 0)
            return false;

        totalSent += static_cast<size_t>(sent);
    }

    return true;
}

void ClientConnection::close() {
    if (socketFd_ >= 0) {
        ::close(socketFd_);
        socketFd_ = -1;
    }
}
