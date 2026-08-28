#include<bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
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
    int sk_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


    connect(sk_fd,res->ai_addr, res->ai_addrlen);

    string msg = "HELLO FROM THE CLIENT \r\n";
    int bytes_sent = send(sk_fd, msg.c_str(), msg.size(), 0);
    if (bytes_sent == -1) {
        perror("send");
    }
    
    char buff[4096];
    
    int bytes_recv = recv(
        sk_fd,
        buff,
        sizeof buff,
        0
    );
    cout<<buff;
}