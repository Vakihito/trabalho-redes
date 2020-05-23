#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <errno.h>  
#include <string> 

#define PORT 8888 
#define QUIT -9999

using namespace std;

void print_name(string name, string color) {
    if (color.compare("red") == 0)
        cout << "\033[1;31m" << "[" << name << "]: " << "\033[0m";
    else if (color.compare("green") == 0)
        cout << "\033[1;32m" << "[" << name << "]: " << "\033[0m";
    else if (color.compare("yellow") == 0)
        cout << "\033[1;33m" << "[" << name << "]: " << "\033[0m";
    else if (color.compare("blue") == 0)
        cout << "\033[1;34m" << "[" << name << "]: " << "\033[0m";
    else if (color.compare("white") == 0)
        cout << "\033[1;314m" << name << "\033[0m";
    else
        cout << "[" << name << "]: ";

    return;
}

void print_text(string text, string color, bool new_line) {
    if (color.compare("red") == 0) cout << "\033[1;31m" << text << "\033[0m";
    else if (color.compare("green") == 0) cout << "\033[1;32m" << text << "\033[0m";
    else if (color.compare("yellow") == 0) cout << "\033[1;33m" << text << "\033[0m";
    else if (color.compare("blue") == 0) cout << "\033[1;34m" << text << "\033[0m";
    else if (color.compare("white") == 0) cout << "\033[1;314m" << text << "\033[0m";
    else cout << text;

    if (new_line) cout << endl;

    return;
}

char *str_to_charA(string str, int n) {
    char *char_array = new char[n];
    for (unsigned int i = 0; i < n and i < str.length(); i++)
        char_array[i] = str[i];
    char_array[n < str.length() ? n : str.length()] = '\0';
    return char_array;
}

int check_command(string command) {
    if(command.compare(0,8,"/connect") == 0) return 1;
    else if(command.compare(0,5,"/quit") == 0) return 2;

    print_name("System","red");
    cout << "Invalid syntax" << endl;

    return 0;
}

int socket_init() {
    int server_socket = 0;
    struct sockaddr_in serv_addr;
    string command;
    int op_code = 0;

    do {
        print_name("User","yellow");
        cin >> command;
        op_code = check_command(command);
    } while(op_code == 0);

    if(op_code == 2) return QUIT;

    print_name("System","yellow");
    cout << "Setting connection..." << endl;

    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Socket creation error" << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 

    // converte endereços IPv4 e IPv6 addresses de texto para o formato binário 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) { 
        perror("Invalid or unsupported address\n");
        return -1; 
    } 

    // aguarda conexão com o servidor
     while(connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        print_name("System", "yellow");
        cout << "Connecting...\n";
        sleep(3);
    }

    print_name("System","green");
    cout << "Server connection stablished" << endl;

    return server_socket;   
}

void send_message(int server_socket) {

    return;   
}

void receive_message(int server_socket) {

    return;
}
   
int main(int argc, char const *argv[]) { 
    int sock = 0;
    int valread; 
    struct sockaddr_in serv_addr; 
    string message; 
    string buffer; 

    // move o cursor para o fim da tela
    cout << "\033[9999;1H";

    // estabelece conexão com o servidor
    sock = socket_init();

    if(sock == QUIT) {
        print_name("System","yellow");
        cout << "Closing application..." << endl;
        return 0;
    }

    while(true) {
        cin >> message;
        char *tmp_message = str_to_charA(message, message.length());
        send(sock, tmp_message, message.length(), 0 );
        free(tmp_message);
        char *tmp_buffer = str_to_charA(buffer, 1024);
        valread = read( sock, tmp_buffer, 1024); 
        cout << buffer << '\n';
        free(tmp_buffer);
    }

    return 0; 
} 