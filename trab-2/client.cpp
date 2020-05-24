#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <errno.h>  
#include <string.h> 
#include <thread>

#define PORT 8888 
#define QUIT -9999
#define size_nickname 50
#define size_message 20
#define size_id 11

using namespace std;

int token = 0;      // código identificador do usuário na aplicação
string nickname;    // nome do usuário no servidor

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

void concat_charA_to_str(string *str, char *char_array, unsigned int n) {
    for(unsigned int i = 0; i < n; i++) 
        (*str).push_back(char_array[i]);

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
    if(command.compare(0, 5, "/quit") == 0 || command.compare(0, 5, "/exit") == 0) return 2;
    if(command.compare(0, 5, "/ping") == 0) return 3;

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
        if (op_code != 1 && op_code != 2){
            print_name("System", "red");
            cout << "Invalid syntax" << endl;
        }
    } while(op_code != 1 && op_code != 2);

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

    char received[size_message + 1];
    string tmp_message;
    int valread = 0;

    memset(received,0,size_message + 1);

    // recebe o número de usuário
    valread = recv(server_socket, &received, size_message + 1, 0);    
    concat_charA_to_str(&tmp_message, received, valread);
    // define o apelido inicial do usuário
    nickname = "User #" + tmp_message;
    token = stoi(tmp_message);

    return server_socket;   
}

void send_message(int server_socket) {
    string message, tmp_message;               //mensagem do usuário
    char response[size_id + size_message + 1]; //bloco de dados enviado ao servidor
    int id_lenght;                             //número de caracteres ocupados pelo id do cliente (incluindo o '\0')

    return;   
}

void receive_message(int server_socket) {
    int valread;
    string message;
    char received[size_message + 1];

    while(true) {
        valread = recv(server_socket, &received, size_message + 1, 0);

        if(received[0] == 'e') {
            print_name("Outro usuário", "blue");
            print_text("Encerrou o chat", "red", true);
            exit(0);
            break;
        }

        received[valread] = '\0';
        while (received[0] == 'i') {
            concat_charA_to_str(&message, &received[1], valread);
            valread = recv(server_socket, &received, size_message + 1, 0);
            received[valread] = '\0';
        }

        if(received[0] != 'n') {
            concat_charA_to_str(&message, &received[1], valread - 1);
            print_name("Outro usuário", "red");
            cout << message << '\n';
            message.clear();
        }   
    }

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

    int flag_command = 0;

    getchar();

    // thread receive_thread(receive_message, sock);
    // thread response_thread(send_message, sock);

    // receive_thread.join();
    // response_thread.join();

    // close(sock);

    while(flag_command != 2) { 
        print_name(nickname,"blue");       
        getline(cin, message);
        char *tmp_message = str_to_charA(message, message.length());
        send(sock, tmp_message, message.length(), 0 );
        free(tmp_message);
        char *tmp_buffer = str_to_charA(buffer, 1024);
        valread = read( sock, tmp_buffer, 1024);
        string tmp_buffer_s(tmp_buffer);
        flag_command = check_command(tmp_buffer_s);
        free(tmp_buffer);
    }

    return 0; 
} 