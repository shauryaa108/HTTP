// #define BACKLOG 10;

#include<bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include<unistd.h>
#include<sstream>
using namespace std;

int main(){
    int status;
    struct addrinfo hints, *res;
    memset(&hints,0, sizeof hints );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if((status = getaddrinfo(NULL,"4221",&hints,&res)) != 0){
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        exit(1);
    }
    
    // 1. Socket setup
    int sk_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sk_fd == -1) {
        perror("socket");
        exit(1);
    }
    
    // 2. bind socket to the port
    if (bind(sk_fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        exit(1);
    }

    // 3. Listen on the socket
    int BACKLOG = 10;
    if (listen(sk_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // 4. actual server loop
    while(true){
        // 5. basic initialisations
        struct sockaddr_storage their_addr;
        socklen_t addr_size;
        addr_size = sizeof their_addr;

        // 6. accept a connection
        int new_sk_fd = accept(sk_fd, (struct sockaddr *)&their_addr, &addr_size);
        if (new_sk_fd == -1) {
            perror("accept");
            continue;
        }
        cout<<"client connected"<<endl;
        
        // 7. recieve a req
        char buff[4096];
        int recv_bytes = recv(
                            new_sk_fd,
                            buff,
                            sizeof(buff) - 1,
                            0
                        );
        if (recv_bytes == -1) {
            perror("recv");
            close(new_sk_fd);
            continue;
        } else if (recv_bytes == 0) {
            cout << "client disconnected"<<endl;
            close(new_sk_fd);
            continue;
        } else {
            buff[recv_bytes] = '\0';
            // cout << buff;
        }

        // HTTP request struct
        struct httpRequest {
            string method;
            string target;
            string version;
        };

        struct httpResponse {
            string version;
            int statusCode;
            string statusText;
            vector<pair<string, string>> headers;
            string body;
        };

        httpRequest http_req;

        // Extract raw HTTP request
        string recv_req = buff;

        // Parse request line
        string statusline = recv_req.substr(0, recv_req.find("\r\n"));

        stringstream ss(statusline);

        vector<string> parts;
        string part;

        while (ss >> part) {
            parts.push_back(part);
        }

        // Basic validation
        if (parts.size() != 3) {
            close(new_sk_fd);
            continue;
        }

        // Populate HTTP request object
        http_req.method = parts[0];
        http_req.target = parts[1];
        http_req.version = parts[2];


        //  HTTP RESPONSE 

        httpResponse response;

        response.version = "HTTP/1.1";
        response.statusCode = 200;
        response.statusText = "OK";

        response.headers.push_back({
            "Content-Type",
            "text/plain"
        });


        // ROUTING 

        if (http_req.method == "GET") {

            string route = http_req.target;

            if (route == "/") {
                response.body =
                    "HELLO FROM THE SERVER, WELCOME HOME";
            }
            else if (route == "/about") {
                response.body =
                    "HELLO FROM THE SERVER, WELCOME TO ABOUT";
            }
            else if (route == "/message") {
                response.body =
                    "HELLO FROM THE SERVER, WELCOME MESSAGE";
            }
            else if (route == "/about/login") {
                response.body =
                    "HELLO FROM THE SERVER, WELCOME TO LOGIN ON ABOUT";
            }
            else {
                response.statusCode = 404;
                response.statusText = "Not Found";

                response.body =
                    "HELLO FROM THE SERVER, everything is wrong here";
            }
        }


        //  RESPONSE HEADERS 

        response.headers.push_back({
            "Content-Length",
            to_string(response.body.size())
        });


        // SERIALISE RESPONSE 

        string resMessage;

        resMessage += response.version + " "
                + to_string(response.statusCode) + " "
                + response.statusText + "\r\n";

        for (auto header : response.headers) {

            resMessage += header.first
                    + ": "
                    + header.second
                    + "\r\n";
        }

        // Empty line separates headers from body
        resMessage += "\r\n";

        // Add body
        resMessage += response.body;


        // SEND RESPONSE 

        int newLen = resMessage.size();

        int bytes_sent = send(
            new_sk_fd,
            resMessage.c_str(),
            newLen,
            0
        );

        if (bytes_sent == -1) {
            perror("send");
        }

        cout << bytes_sent;


        // CLOSE CONNECTION

        close(new_sk_fd);
    }
    freeaddrinfo(res);
}