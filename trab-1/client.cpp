// Client side C/C++ program to demonstrate Socket programming 
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <string.h> 
#define PORT 8080 
#define size_request 5

using namespace std;

void str_to_charA(char *char_array, string str, unsigned int n) {
    for (unsigned int i = 0; i < n && i < str.length(); i++)
        char_array[i] = str[i];
    return;
}
   
int main(int argc, char const *argv[]) { 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    string tmp;
    char response[size_request], request[size_request + 1];

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
        tmp.clear();
        cout << "Digite a request: ";
        getline(cin, tmp);
        request[0] = 'i';
        str_to_charA(&request[1], tmp, size_request);
        while(tmp.length() > size_request) {
            send(sock, &request, size_request + 1, 0);
            tmp = tmp.substr(size_request);
            str_to_charA(&request[1], tmp, size_request);
        }
        request[0] = 'c';
        cout << "Enviando request...\n";
        send(sock, &request, tmp.length() + 1, 0);
        cout << "Requisição enviada!\nAguardando response...\n";
        valread = recv(sock, &response, size_request, 0); 
        cout << "Response recebida: ";
        cout << response << '\n';
    }
    return 0; 
} 