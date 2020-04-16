// Client side C/C++ program to demonstrate Socket programming 
#include <iostream> 
#include <string>
#include <vector>


#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <string.h> 
#define PORT 8080 

using namespace std;
   
int main(int argc, char const *argv[]) { 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char buffer[4096]; 
    char hello[4096]; 
    cout << "Configurando conexão...\n";
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        cout << "Socket creation error \n";
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) { 
        cout << "Invalid address/ Address not supported\n";
        return -1; 
    } 

    cout << "Conectando...\n";
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        cout << "404 - Não foi possível encontrar o servidor \n";
        return -1; 
    }

    while(true) {
        cout << "Digite a request: ";
        cin >> hello;
        cout << "Enviando request...\n";
        send(sock, &hello , 4096, 0);
        cout << "Requisição enviada!\nAguardando response...\n";
        valread = recv( sock , &buffer, 4096, 0); 
        cout << "Response recebida: ";
        cout << buffer << '\n';
    }
    return 0; 
} 