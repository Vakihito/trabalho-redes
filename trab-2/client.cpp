#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <errno.h>  
#include <string.h> 
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 8888 
#define QUIT -9999
#define size_nickname 20
#define size_message 100
#define size_id 6

//códigos presentes na requisição para identificar o tipo de operação solicitada
#define FLAG_EXIT "e"               //término de conexão com o servidor
#define FLAG_INCOMPLETE "i"         //mensagem incompleta
#define FLAG_COMPLETE "c"           //mensagem completa
#define FLAG_PING "p"               //instrução "ping-pong"
#define FLAG_CHANGE_USERNAME "u"    //trocar username

using namespace std;

int token = 0;         // código identificador do usuário na aplicação
string nickname;       // nome do usuário no servidor
int flag_command = 0;  // numero do comando recebedi / enviado

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
        cout << "\033[1;314m" << "[" << name << "]: " << "\033[0m";
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

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
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
    if(command.compare(0, 8,"/connect") == 0) return 1;
    if(command.compare(0, 5, "/quit") == 0) return 2;
    if(command.compare(0, 5, "/ping") == 0) return 3;


    print_name("System", "red");
    cout << "Command not found" << endl;

    return 0;
}

/* Para enviar o token do usuário pela mensagem precisamos de um tamanho padrão */
/* esta função completa com zeros caso seja menor que o tamanho esperado */
/* ex : se token = 32, precisa colocar 000032 */   
string fill_nickname() {
    string stoken_s = to_string(token);
    while (stoken_s.length() < size_id)
        stoken_s = "0" + stoken_s;
    return stoken_s;
}

void stoca(char *char_array, string str, unsigned int n) {
    for (unsigned int i = 0; i < n && i < str.length(); i++)
        char_array[i] = str[i];
    return;
}

int socket_init(string username) {
    int server_socket = 0;
    struct sockaddr_in serv_addr;
    int op_code = 0;

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
    int max_attempts = 5, qtd_attempts = 0;
    while(qtd_attempts < max_attempts and connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        print_name("System", "yellow");
        cout << "Connecting...\n";
        sleep(3);
        qtd_attempts++;
    }

    if (qtd_attempts == max_attempts) {
        print_name("System", "red");
        cout << "Could not connect to server\n";
        exit(QUIT);
    }

    print_name("System","green");
    cout << "Server connection stablished" << endl;

    char received[size_message + 1];
    // Recebe o token
    int valread = recv(server_socket, &received, size_message + 1, 0);
    received[valread] = '\0';
    token = stoi(received);

    // Troca o username para o escolhido
    string message = to_string(token) + FLAG_CHANGE_USERNAME + username;
    char *tmp = str_to_charA(message, message.length());
    send(server_socket, tmp, message.length(), 0);
    free(tmp);

    return server_socket;   
}

void send_message(int server_socket) {
    string message;                            //mensagem do usuário
    int id_lenght;                             //número de caracteres ocupados pelo id do cliente (incluindo o '\0')
    string op_code;                            //char correspondente à útlima requisição do cliente

    while (flag_command != 2) {
        char *request;
        string str_request;
        string tmp_message;

        getline(cin, message);                  //recebe a entrada do usuário
        flag_command = check_command(message);  //avalia se a mensagem corresponde a algum comando da aplicação
        str_request = to_string(token);         //insere o token na requisição

        // envio de mensagem comum
        if(flag_command == 0) {
            op_code = FLAG_INCOMPLETE;
            tmp_message = message;
            str_request = str_request + op_code + tmp_message;
            request = str_to_charA(str_request, size_message + size_id + 1);

            while(tmp_message.length() > size_message) {
                send(server_socket, &request, size_id + size_message + 1, 0);
                tmp_message = tmp_message.substr(size_message);
                stoca(&request[1 + size_id], tmp_message, size_message);
            }

            request[size_id] = FLAG_COMPLETE[0];
            for(int i = size_id + tmp_message.length() + 1; i < size_message; ++i) request[i] = '\0';
            send(server_socket, request, tmp_message.length() + size_id + 1, 0);
        }

        else {
            if(flag_command == 2) op_code = FLAG_EXIT;        // envio de desconexão com o servidor
            else if(flag_command == 3) op_code = FLAG_PING;   // envio do comando "/ping"
                                                      
            str_request = str_request + op_code;                       //insere o código da operação e a mensagem          
            request = str_to_charA(str_request,str_request.length());  //converte o formato para vetor de caracteres
            send(server_socket, request, size_id + 1, 0);              //envia a requisição ao servidor
        }

        memset(request, 0, size_id + tmp_message.length() + 1);
        message.clear();
        tmp_message.clear();

        free(request);
    }

    return;   
}

void receive_message(int server_socket) {
    int valread;
    string message, buffer;
    char received[size_message + 1];

    while (flag_command != 2) {
        char *tmp_buffer = str_to_charA(buffer, 1024);
        valread = read(server_socket, tmp_buffer, 1024);
        string clientToken (tmp_buffer, 6);
        print_name(nickname, "green");
        cout << &tmp_buffer[6] << endl;
        free(tmp_buffer);
    }

    return;
}
   
int main(int argc, char const *argv[]) { 
    int sock = 0;
    int valread; 
    string command;
    int op_code = 0;
    struct sockaddr_in serv_addr; 
    string message; 
    string buffer; 

    // move o cursor para o fim da tela
    cout << "\033[9999;1H";

    // Boas vindas
    print_text("Bem vindo! Comandos disponíveis:","red", true);
    // Comando connect
    print_text("/connect ","blue", false);
    print_text("username ", "green", false);
    print_text("- Conecta ao servidor com o ","yellow", false);
    print_text("username ", "green", false);
    print_text("informado.","yellow", true);
    // Comando ping
    print_text("/ping ","blue", false);
    print_text("- Envia um ping para o servidor","yellow", true);
    // Comando quit
    print_text("/quit ","blue", false);
    print_text("- Sai do programa","yellow", true);

    do {
        print_name("User", "yellow");
        cin >> command;
        op_code = check_command(command);
    } while(op_code == 0);

    if(op_code == 2) return QUIT;

    string username;
    cin >> username;

    // estabelece conexão com o servidor
    sock = socket_init(username);
    close(sock);
    return 0;

    if(sock == QUIT) {
        print_name("System","yellow");
        cout << "Closing application..." << endl;
        return 0;
    }

    getchar();

    thread receive_thread(receive_message, sock);
    thread response_thread(send_message, sock);

    response_thread.join();
    receive_thread.join();

    close(sock);
    return 0; 
} 