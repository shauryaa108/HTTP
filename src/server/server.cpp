
#include "server.h"

#include "ClientConnection.h"

#include "../http/HttpParser.h"
#include "../http/HttpResponse.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <cerrno>

using namespace std;

Server::Server()
    : serverSocket(-1) {
}

Server::~Server() {
    if (serverSocket >= 0) {
        ::close(serverSocket);
        serverSocket = -1;
    }
}

bool Server::start(int port) {
    addrinfo hints{}, *result = nullptr;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    string portString = to_string(port);

    int status = getaddrinfo(
        nullptr,
        portString.c_str(),
        &hints,
        &result
    );

    if (status != 0) {
        cerr << "getaddrinfo failed: "
             << gai_strerror(status) << "\n";
        return false;
    }

    for (addrinfo* current = result;
         current != nullptr;
         current = current->ai_next) {

        int listeningFd = socket(
            current->ai_family,
            current->ai_socktype,
            current->ai_protocol
        );

        if (listeningFd == -1)
            continue;

        int yes = 1;

        setsockopt(
            listeningFd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &yes,
            sizeof(yes)
        );

        if (bind(
            listeningFd,
            current->ai_addr,
            current->ai_addrlen
        ) == 0) {

            serverSocket = listeningFd;
            break;
        }

        ::close(listeningFd);
    }

    freeaddrinfo(result);

    if (serverSocket == -1) {
        cerr << "Could not bind socket\n";
        return false;
    }

    if (listen(serverSocket, 10) == -1) {
        cerr << "listen failed: "
             << strerror(errno) << "\n";

        ::close(serverSocket);
        serverSocket = -1;

        return false;
    }

    cout << "Server listening on port "
         << port << "\n";

    return true;
}

void Server::run() {

    while (true) {

        int acceptedFd = accept(
            serverSocket,
            nullptr,
            nullptr
        );

        if (acceptedFd == -1) {

            if (errno == EINTR)
                continue;

            cerr << "accept failed: "
                 << strerror(errno) << "\n";

            continue;
        }

        // Ownership of acceptedFd transfers to client.
        ClientConnection client(acceptedFd);

        if (!client.setReadTimeout(20, 0)) {
            cerr << "Failed to configure receive timeout\n";
            continue;
        }

        handleClient(client);

        // ClientConnection destructor closes acceptedFd.
    }
}

void Server::handleClient(ClientConnection& client) {

    HttpParser parser;

    char buffer[4096];

    while (true) {

        ssize_t bytesRead = client.receive(
            buffer,
            sizeof(buffer)
        );

        // Client disconnected gracefully.
        if (bytesRead == 0) {
            cout << "Client disconnected\n";
            return;
        }

        // Handle receive errors.
        if (bytesRead < 0) {

            // Receive timeout.
            if (errno == EAGAIN || errno == EWOULDBLOCK) {

                cerr << "Client request timed out\n";

                HttpResponse response =
                    HttpResponse::requestTimeout();

                client.sendAll(response.toString());

                return;
            }

            // Interrupted system call.
            if (errno == EINTR) {
                continue;
            }

            // Actual socket error.
            cerr << "recv failed: "
                 << strerror(errno) << "\n";

            return;
        }

        // Feed received data into the parser.
        ParseResult result = parser.feed(
            string(buffer, static_cast<size_t>(bytesRead))
        );

        // Request is incomplete. Wait for more data.
        if (result == ParseResult::Incomplete) {
            continue;
        }

        // Malformed HTTP request.
        if (result == ParseResult::BadRequest) {

            HttpResponse response =
                HttpResponse::badRequest();

            client.sendAll(response.toString());

            return;
        }

        // Request is complete.
        const HttpRequest& request =
            parser.getRequest();

        HttpResponse response;

        // Method validation.
        if (request.method != "GET" &&
            request.method != "HEAD" &&
            request.method != "POST") {

            response =
                HttpResponse::methodNotAllowed();
        }

        // Route handling.
        else if (request.target == "/hello") {

            if (request.method == "POST") {

                response = HttpResponse::ok(
                    "POST received: " + request.body
                );

            } else {

                response = HttpResponse::ok(
                    "Hello from the HTTP server!\n"
                );
            }
        }

        // Unknown route.
        else {

            response =
                HttpResponse::notFound();
        }

        // HEAD must not include a response body.
        string responseData = response.toString(
            request.method == "HEAD"
        );

        // Send the complete response.
        if (!client.sendAll(responseData)) {

            cerr << "Failed to send response: "
                 << strerror(errno) << "\n";

            return;
        }

        return;
    }
}
