// Server side C/C++ program to demonstrate Socket programming 
#include <iostream> 
#include <string>
#include <vector>
#include <string.h>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>

#define NAME "Server"
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
    struct sockaddr_in address; 
    int server_fd, new_socket, valread, addrlen = sizeof(address), opt = 1;
    string tmp;
    char request[size_request + 1], response[size_request + 1];
    
    print_name(NAME,"yellow");
    cout << "Configurando conexão..." << endl;
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

    print_name(NAME,"yellow");
    cout << "Aguardando conexão...\n";

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

    print_name(NAME,"green");
    cout << "Conexão estabelecida!\n";

    string aux_hello;

    while(true) {
        print_name(NAME,"yellow");
        cout << "Aguardando request...\n";
        valread = recv(new_socket, &request, size_request + 1, 0);
        request[valread] = '\0';
        if (request[0] == 'i') {
            concat_charA_to_str(&tmp, &request[1], valread);
            continue;
        } else concat_charA_to_str(&tmp, &request[1], valread - 1);
        print_name("Request","red");
        cout << tmp << endl;
        tmp.clear();

        print_name("Response","blue");
        getline(cin, aux_hello);
        response[0] = 'i';
        str_to_charA(&response[1], aux_hello, size_request);
        while(aux_hello.length() > size_request) {
            send(new_socket, &response, size_request + 1, 0);
            print_name(NAME,"yellow");
            cout << "Enviando response ";
            print_text(&response[1],"yellow",true);
            aux_hello = aux_hello.substr(size_request);
            str_to_charA(&response[1], aux_hello, size_request);
        }
        response[0] = 'c';
        print_name(NAME,"yellow");
        cout << "Enviando response ";
        print_text(aux_hello,"yellow",true);
        send(new_socket, &response, aux_hello.length() + 1, 0);
        print_name(NAME,"green");
        cout << "Response enviada!...\n";
        aux_hello.clear();  
    }
    return 0; 
} 