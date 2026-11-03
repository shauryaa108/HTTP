#pragma once

using namespace std;

class Server {
private:
    int serverSocket;

    void handleClient(int clientSocket);

public:
    Server();

    bool start(int port);
    void run();
};