#include "server/server.h"

using namespace std;

int main() {
    Server server;

    if (!server.start(4221))
        return 1;

    server.run();

    return 0;
}