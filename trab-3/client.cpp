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
#include <csignal>

#include "chat.h"

using namespace std;

int token = 0;         // código identificador do usuário na aplicação
string username;       // nome do usuário no servidor
string channel;        // nome do canal
int flag_command = 0;  // numero do comando recebedi / enviado

void signalHandler( int signum ) {
   cout << "\nSIGINT (CTRL + C) não tem efeito neste programa. Por favor, saia por meio do comando '/quit'\n";
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

int check_command(string command) {
    if(command.compare(0, 8,"/connect") == 0) return 1;
    if(command.compare(0, 5, "/quit") == 0) return 2;
    if(command.compare(0, 5, "/ping") == 0) return 3;

    return 0;
}

/* Para enviar o token do usuário pela mensagem precisamos de um tamanho padrão */
/* esta função completa com zeros caso seja menor que o tamanho esperado */
/* ex : se token = 32, precisa colocar 000032 */   
string fill_nickname() {
    string stoken_s = to_string(token);
    while (stoken_s.length() < size_token)
        stoken_s = "0" + stoken_s;
    return stoken_s;
}



string choose_username(int server_socket) {
    string name;
    string message;
    bool valid_username = false;
    char *request;
    char response;
    int valread;

    getchar();

    while(!valid_username) {
        print_name("System","yellow");
        cout << "Write your username" << endl;
        print_name("User","yellow");
        getline(cin, name);

        if(name.length() > size_username) {
            print_name("System","red");
            cout << "Exceeded limit of " << size_username << " characters" << endl;
        }

        message = fill_nickname() + FLAG_CHANGE_USERNAME + name;
        request = str_to_charA(message, size_token + size_username + 1);
        memset(&request[1 + size_token + name.length()],0,size_username - name.length());
        send(server_socket, request, size_token + size_username + 1, 0);

        valread = recv(server_socket, &response, 1, 0);

        if(response == INVALID_USERNAME) {
            print_name("Server","red");
            cout << "This username already exists" << endl;
        }

        else if(response == VALID_USERNAME) {
            valid_username = true;
            username = name;
            print_name("Server","green");
            cout << "Registered username as " << username << endl;
        }

        else {
            print_name("Server","red");
            cout << "Request error" << endl;
        }
    }

    return username;
}

void define_channel(int server_socket){
    
    string channelAux; 
    bool validChannel = true;
    print_name("System","yellow");
    cout << "Select your channel" << endl;
    do {
        validChannel = true;
        print_name("User","yellow");
        getline(cin,channelAux);
        if (channelAux.length() > 49 || channelAux.find('[') != std::string::npos)
        {
            print_name("System","red");
            cout << "invalid input" << endl;
            validChannel = false;
        }
    }while(! validChannel);
    channel = "#" + channelAux;
    channelAux = to_string(token) + channel;

    char* request = str_to_charA(channelAux, channelAux.length() + size_token + 1);
    
    send(server_socket, request, channelAux.length() + size_token + 1, 0);

    print_name("System","green");
    cout << "connecting... " << endl;
}

int socket_init() {
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
    username = choose_username(server_socket);
    define_channel(server_socket);

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
        // checks for ctr + d
        if (cin.eof() == 1)
            message ="/quit";
        
        flag_command = check_command(message);  //avalia se a mensagem corresponde a algum comando da aplicação
        str_request = to_string(token);         //insere o token na requisição

        // envio de mensagem comum
        if(flag_command == 0) {
            op_code = FLAG_INCOMPLETE;
            tmp_message = message;
            str_request = str_request + op_code + tmp_message;
            request = str_to_charA(str_request, size_message + size_token + 1);

            while(tmp_message.length() > size_message) {
                send(server_socket, &request, size_token + size_message + 1, 0);
                tmp_message = tmp_message.substr(size_message);
                stoca(&request[1 + size_token], tmp_message, size_message);
            }

            request[size_token] = FLAG_COMPLETE[0];
            for(int i = size_token + tmp_message.length() + 1; i < size_message; ++i) request[i] = '\0';
            send(server_socket, request, tmp_message.length() + size_token + 1, 0);
        }

        else {
            if(flag_command == 2) op_code = FLAG_EXIT;        // envio de desconexão com o servidor
            else if(flag_command == 3) op_code = FLAG_PING;   // envio do comando "/ping"
                                                      
            str_request = str_request + op_code;                       //insere o código da operação e a mensagem          
            request = str_to_charA(str_request,str_request.length());  //converte o formato para vetor de caracteres
            send(server_socket, request, size_token + 1, 0);              //envia a requisição ao servidor
        }

        memset(request, 0, size_token + tmp_message.length() + 1);
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
        char *tmp_buffer = str_to_charA(buffer, 1 + size_username + size_message);
        valread = read(server_socket, tmp_buffer, 1 + size_username + size_message);
        char sent_by = tmp_buffer[0];

        //mensagem enviada por um cliente
        if(sent_by == CLIENT[0]) {
            string client = &tmp_buffer[1];
            string message = &tmp_buffer[1 + size_username];
            //outro cliente
            if(client.compare(username) != 0) {
                print_name(client, "blue");
                print_text(message,"",true);
            }
            //você
            else {
                print_name(username, "green");
                print_text(message,"white",true);
            }
        } 

        //mensagem enviada pelo servidor
        else if(sent_by == SERVER[0]) {
            string message = &tmp_buffer[1];
            print_name("Server","green");  
            cout << message << endl;
        }

        free(tmp_buffer);
    }

    return;
}
   
int main(int argc, char const *argv[]) { 
    signal(SIGINT, signalHandler);

    int sock = 0;
    int valread; 
    string command;
    int op_code = 0;
    struct sockaddr_in serv_addr; 
    string message; 
    string buffer; 

    // move o cursor para o fim da tela
    cout << "\033[2J\033[1;1H";
    cout << "\033[9999;1H";

    // Boas vindas
    print_text("Bem vindo! Comandos disponíveis:","green", true);
    // Comando connect
    print_text("/connect ","blue", false);
    print_text("- Conecta ao servidor","white", true);
    // Comando quit
    print_text("/quit ","blue", false);
    print_text("- Sai do programa","white", true);
    cout << endl;

    do {
        print_name("User", "yellow");
        cin >> command;
        op_code = check_command(command);
        if(op_code == 0) {
            print_name("System", "red");
            cout << "Command not found" << endl;
        }
    } while(op_code == 0);

    if(op_code == 2) return QUIT;

    // estabelece conexão com o servidor
    sock = socket_init();
    

    if(sock == QUIT) {
        print_name("System","yellow");
        cout << "Closing application..." << endl;
        return 0;
    }


    thread receive_thread(receive_message, sock);
    thread response_thread(send_message, sock);

    response_thread.join();
    receive_thread.join();

    close(sock);
    return 0; 
} 