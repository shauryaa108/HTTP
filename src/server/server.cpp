#include "server.h"

#include "../http/HttpParser.h"
#include "../http/HttpResponse.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <filesystem>

#include <iostream>
#include <string>

using namespace std;

Server::Server()
    : serverSocket(-1) {
}

bool Server::start(int port) {
    addrinfo hints{}, *result;

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
        cerr << "getaddrinfo failed\n";
        return false;
    }

    for (addrinfo* current = result;
         current != nullptr;
         current = current->ai_next) {

        serverSocket = socket(
            current->ai_family,
            current->ai_socktype,
            current->ai_protocol
        );

        if (serverSocket == -1)
            continue;

        int yes = 1;

        setsockopt(
            serverSocket,
            SOL_SOCKET,
            SO_REUSEADDR,
            &yes,
            sizeof(yes)
        );

        if (bind(
            serverSocket,
            current->ai_addr,
            current->ai_addrlen
        ) == 0) {
            break;
        }

        close(serverSocket);
        serverSocket = -1;
    }

    freeaddrinfo(result);

    if (serverSocket == -1) {
        cerr << "Could not bind socket\n";
        return false;
    }

    if (listen(serverSocket, 10) == -1) {
        cerr << "listen failed\n";

        close(serverSocket);
        serverSocket = -1;

        return false;
    }

    cout << "Server listening on port "
         << port
         << "\n";

    return true;
}

void Server::run() {
    while (true) {
        int clientSocket = accept(
            serverSocket,
            nullptr,
            nullptr
        );

        if (clientSocket == -1) {
            cerr << "accept failed\n";
            continue;
        }

        handleClient(clientSocket);

        close(clientSocket);
    }
}

void Server::handleClient(int clientSocket) {
    HttpParser parser;

    char buffer[4096];

    while (true) {
        ssize_t bytesRead = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );

        if (bytesRead == 0)
            return;

        if (bytesRead < 0) {
            HttpResponse response =
                HttpResponse::internalServerError();

            string data = response.toString();

            send(
                clientSocket,
                data.c_str(),
                data.size(),
                0
            );

            return;
        }

        ParseResult result = parser.feed(
            string(buffer, bytesRead)
        );

        if (result == ParseResult::Incomplete)
            continue;

        if (result == ParseResult::BadRequest) {
            HttpResponse response =
                HttpResponse::badRequest();

            string data = response.toString();

            send(
                clientSocket,
                data.c_str(),
                data.size(),
                0
            );

            return;
        }

        const HttpRequest& request =
            parser.getRequest();

        HttpResponse response;

        if (request.method != "GET" &&
            request.method != "HEAD" &&
            request.method != "POST") {

            response =
                HttpResponse::methodNotAllowed();
        }
        else if (request.target == "/hello") {

            if (request.method == "POST") {
                response = HttpResponse::ok(
                    "POST received: " +
                    request.body
                );
            }
            else {
                response = HttpResponse::ok(
                    "Hello from the HTTP server!\n"
                );
            }
        }
        else {
            response =
                HttpResponse::notFound();
        }

        string data = response.toString(
            request.method == "HEAD"
        );

        size_t totalSent = 0;

        while (totalSent < data.size()) {
            ssize_t sent = send(
                clientSocket,
                data.c_str() + totalSent,
                data.size() - totalSent,
                0
            );

            if (sent <= 0)
                return;

            totalSent += sent;
        }

        return;
    }
}