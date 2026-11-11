
#pragma once

class ClientConnection;
class Server {
private:
    int serverSocket;

    void handleClient(class ClientConnection& client);

public:
    Server();

    ~Server();

    // Server owns its listening socket.
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool start(int port);
    void run();
};
