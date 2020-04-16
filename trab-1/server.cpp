// Server side C/C++ program to demonstrate Socket programming 
#include <iostream> 
#include <string>
#include <vector>

#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#define PORT 8080 

using namespace std;

int main(int argc, char const *argv[]) { 
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[4096]; 
    char hello[4096]; 
    
    printf("Configurando conexão...\n");
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    if (listen(server_fd, 3) < 0) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    cout << "Aguardando conexão...\n";
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
    cout << "Conexão estabelecida!\n";

    while(true) {
        cout << "Aguardando request...\n";
        valread = recv(new_socket, &buffer, 4096, 0);
        cout << "Request recebida:\n";
        cout << buffer << '\n';
        cout << "Digite a response:\n";
        cin.getline(hello, 4096);
        send(new_socket , &hello, sizeof(hello), 0 );
        cout << "Response enviada: ";
        cout << hello << '\n'; 
    }
    return 0; 
} 