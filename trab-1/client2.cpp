// Client side C/C++ program to demonstrate Socket programming 
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include <unistd.h>
#include <thread>

#define NAME "Client"
#define PORT 8080 
#define size_message 8

using namespace std;

int exec = 0;
bool stop = false;

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

int check_commands(string message) {
    if (message.compare("/exit") == 0 || message.compare("/quit") == 0)
        return 1;
    
    else if(message.compare("/ping") == 0) return 2;

    return 0; 
}

int socket_init() {
    int server_socket = 0;
    struct sockaddr_in serv_addr; 

    print_name("Aplicação","yellow");
    cout << "Configurando conexão...\n";
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
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

    while(connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        print_name("Aplicação","yellow");
        cout << "Conectando...\n";
        sleep(3);
    }

    print_name("Aplicação","green");
    cout << "Conexão estabelecida!\n";

    return server_socket;
}

void receive_message(int new_socket) {
    int valread;
    int instruction;
    string message;
    char request[size_message + 1];

    while(true) {

        valread = recv(new_socket, &request, size_message + 1, 0);

        if (request[0] == 'e') {
            print_name("Outro usuário", "blue");
            print_text("Encerrou o chat", "red", true);
            exit(0);
            break;
        }

        request[valread] = '\0';
        while (request[0] == 'i') { 
            concat_charA_to_str(&message, &request[1], valread);
            valread = recv(new_socket, &request, size_message + 1, 0);
            request[valread] = '\0';
        }

        concat_charA_to_str(&message, &request[1], valread - 1);

        print_name("Outro usuário","yellow");
        cout << message << '\n';
        message.clear();
    }

    return;
}

void send_message(int new_socket) {
    string message, tmp_message;
    char response[size_message + 1];

    while(!stop) {
        if(exec == 0) {
            getline(cin, message);
            tmp_message = message;

            if (check_commands(message) == 1) {
                print_name("Outro usuário", "blue");
                print_text("Encerrou o chat", "red", true);
                response[0] = 'c';
                str_to_charA(&response[1], message, size_message);
                send(new_socket, &response, size_message + 1, 0);
                exec = 1;
                stop = true;
            }

            response[0] = 'i';
            str_to_charA(&response[1], message, size_message);

            while(tmp_message.length() > size_message) {
                send(new_socket, &response, size_message + 1, 0);
                tmp_message = tmp_message.substr(size_message);
                str_to_charA(&response[1], tmp_message, size_message);
            }
        }

        else if(exec == 2) {
            message = "ping";
            response[0] = 'i';
            str_to_charA(&response[1], message, size_message);
            cout << message << '\n';
        } 
    
        if(exec != 1) {
            response[0] = 'c';
            print_name(NAME,"blue");
            print_text(message,"yellow",true);
            send(new_socket, &response, tmp_message.length() + 1, 0);
        }
        message.clear(); 
        tmp_message.clear();
    }

    return;
}

int main(int argc, char const *argv[]) { 
    
    int new_socket = socket_init();

    thread receive_thread (receive_message, new_socket);
    thread response_thread (send_message, new_socket);

    receive_thread.join();
    response_thread.join();

    close(new_socket);

    return 0; 
} 