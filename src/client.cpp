#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <iostream>
#include <string>

using namespace std;

int main() {
    addrinfo hints{}, *result;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(
        "127.0.0.1",
        "4221",
        &hints,
        &result
    ) != 0) {
        return 1;
    }

    int sock = socket(
        result->ai_family,
        result->ai_socktype,
        result->ai_protocol
    );

    if (sock == -1) {
        freeaddrinfo(result);
        return 1;
    }

    if (connect(
        sock,
        result->ai_addr,
        result->ai_addrlen
    ) == -1) {
        close(sock);
        freeaddrinfo(result);
        return 1;
    }

    freeaddrinfo(result);

    string request =
        "GET /hello HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: my-client\r\n"
        "Accept: */*\r\n"
        "Connection: close\r\n"
        "\r\n";

    send(
        sock,
        request.c_str(),
        request.size(),
        0
    );

    char buffer[4096];

    ssize_t bytes;

    while ((bytes = recv(
        sock,
        buffer,
        sizeof(buffer),
        0
    )) > 0) {
        cout.write(buffer, bytes);
    }

    close(sock);

    return 0;
}