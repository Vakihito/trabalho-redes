// Client side C/C++ program to demonstrate Socket programming 
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <string.h> 

#define NAME "Client"
#define PORT 8080 
#define size_request 5

using namespace std;

void print_name(string name, string color){
    if (color.compare("red") == 0) cout << "\033[1;31m" << "[" << name << "]: " <<"\033[0m";
    else if (color.compare("green") == 0) cout << "\033[1;32m" << "[" << name << "]: " <<"\033[0m";
    else if (color.compare("yellow") == 0) cout << "\033[1;33m" << "[" << name << "]: " <<"\033[0m";
    else if (color.compare("blue") == 0) cout << "\033[1;34m" << "[" << name << "]: " <<"\033[0m";
    else cout << "[" << name << "]: ";

    return;
}

void print_text(string text, string color, bool new_line){
    if(color.compare("red") == 0) cout << "\033[1;31m" << text <<"\033[0m";
    else if (color.compare("green") == 0) cout << "\033[1;32m" << text  <<"\033[0m";
    else if (color.compare("yellow") == 0) cout << "\033[1;33m" << text <<"\033[0m";
    else if (color.compare("blue") == 0) cout << "\033[1;34m" << text <<"\033[0m";
    else cout << text;

    if(new_line) cout << endl; 

    return;
}

void str_to_charA(char *char_array, string str, unsigned int n) {
    for (unsigned int i = 0; i < n && i < str.length(); i++)
        char_array[i] = str[i];
    return;
}

// char_array length must be smaller or equal to n
void concat_charA_to_str(string *str, char *char_array, unsigned int n) {
    for (unsigned int i = 0; i < n; i++)
        (*str).push_back(char_array[i]);
    return;
}
   
int main(int argc, char const *argv[]) { 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    string tmp;
    char response[size_request + 1], request[size_request + 1];

    print_name("Sistema","yellow");
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

    print_name("Sistema","yellow");
    cout << "Conectando...\n";
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        cout << "404 - Não foi possível encontrar o servidor \n";
        return -1; 
    }

    print_name("Sistema","green");
    cout << "Conexão estabelecida!\n";

    string aux_hello;

    while(true) {
        print_name("Request","blue");
        getline(cin, tmp);
        request[0] = 'i';
        str_to_charA(&request[1], tmp, size_request);
        while(tmp.length() > size_request) {
            send(sock, &request, size_request + 1, 0);
            print_name("Sistema","yellow");
            cout << "Enviando request ";
            print_text(&request[1],"yellow",true);
            tmp = tmp.substr(size_request);
            str_to_charA(&request[1], tmp, size_request);
        }
        request[0] = 'c';
        print_name("Sistema","yellow");
        cout << "Enviando request ";
        print_text(tmp,"yellow",true);
        send(sock, &request, tmp.length() + 1, 0);
        print_name("Sistema","green");
        cout << "Requisição enviada!\n"; 
        tmp.clear();

        print_name("Sistema","yellow");
        cout << "Aguardando response...\n";
        valread = recv(sock, &response, size_request + 1, 0);
        response[valread] = '\0';
        while(response[0] == 'i') {
            concat_charA_to_str(&aux_hello, &response[1], valread);
            valread = recv(sock, &response, size_request + 1, 0);
            response[valread] = '\0';
        }
        concat_charA_to_str(&aux_hello, &response[1], valread - 1);
        print_name("Response","red");
        cout << aux_hello << endl;
        aux_hello.clear();
    }
    return 0; 
} 