// Server side C/C++ program to demonstrate Socket programming 
#include <iostream> 
#include <string>
#include <vector>
#include <string.h>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#define PORT 8080 

#define size_request 5

using namespace std;

// char_array length must be smaller or equal to n
void concat_charA_to_str(string *str, char *char_array, unsigned int n) {
    for (unsigned int i = 0; i < n; i++)
        (*str).push_back(char_array[i]);
    return;
}

int main(int argc, char const *argv[]) { 
    struct sockaddr_in address; 
    int server_fd, new_socket, valread, addrlen = sizeof(address), opt = 1;
    string tmp;
    char request[size_request + 1], response[size_request];
    
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

    string aux_hello;

    while(true) {
        tmp.clear();
        cout << "Aguardando request...\n";
        valread = recv(new_socket, &request, size_request + 1, 0);
        request[valread] = '\0';
        if (request[0] == 'i') {
            concat_charA_to_str(&tmp, &request[1], valread);
            continue;
        } else concat_charA_to_str(&tmp, &request[1], valread - 1);
        cout << "Request recebida:\n";
<<<<<<< HEAD
        cout << "\033[1;31m" << tmp << "\033[0m" << '\n';
=======
        cout << tmp << '\n';
        tmp.clear();
>>>>>>> tmp
        cout << "Digite a response:\n";
        getline(cin, aux_hello);
        strcpy(response, aux_hello.c_str());
        send(new_socket , &response, sizeof(response), 0 );
        cout << "Response enviada: ";
        cout << "\033[1;34m" << response << "\033[0m\n";
    }
    return 0; 
} 