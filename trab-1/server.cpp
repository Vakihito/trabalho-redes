/*--Observações--*/
/*

>> Para facilitar o processamento das mensagens, foram estabelecidos alguns códigos, os quais são
inseridos no 1 byte do bloco da requisição/resposta, de modo a identificar a operação correspondente.  
* 'i': outros blocos serão enviados para compor a mensagem final
* 'c': bloco final da mensagem
* 'e': encerra conexão do cliente com o servidor

*/

// Server side C/C++ program to demonstrate Socket programming 
#include <iostream> 
#include <string>
#include <vector>
#include <string.h>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#include <unistd.h>

#define NAME "Server"
// defining the port number
#define PORT 8080 
// defining the max size possible for a message
#define size_message 4096

using namespace std;

// This function prints the "name" in a especific "color"
// the name will be printed in between "[]"
void print_name(string name, string color){
    if (color.compare("red") == 0) cout << "\033[1;31m" << "[" << name << "]: " <<"\033[0m";
    else if (color.compare("green") == 0) cout << "\033[1;32m" << "[" << name << "]: " <<"\033[0m";
    else if (color.compare("yellow") == 0) cout << "\033[1;33m" << "[" << name << "]: " <<"\033[0m";
    else if (color.compare("blue") == 0) cout << "\033[1;34m" << "[" << name << "]: " <<"\033[0m";
    else cout << "[" << name << "]: ";

    return;
}

// This function print a "text" in a especific "color"
// "new_line" decides if there will be a new line of not 
void print_text(string text, string color, bool new_line){
    if(color.compare("red") == 0) cout << "\033[1;31m" << text <<"\033[0m";
    else if (color.compare("green") == 0) cout << "\033[1;32m" << text  <<"\033[0m";
    else if (color.compare("yellow") == 0) cout << "\033[1;33m" << text <<"\033[0m";
    else if (color.compare("blue") == 0) cout << "\033[1;34m" << text <<"\033[0m";
    else cout << text;

    if(new_line) cout << endl; 

    return;
}

// this function passes the content of "str" to the "char_array"
// transforms string into a char array
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

// verifie the command and return a number
// return 0 if not a command
// return 1 if the command is /exit or /quit
// return 2 if the command is /ping
int check_commands(string message) {
    if (message.compare("/exit") == 0 || message.compare("/quit") == 0)
        return 1;
    else if(message.compare("/ping") == 0) return 2;

    return 0; 
}

// this function receives a the request from the client
// return 0 if the request is not a command
// return 1 if the request is /exit or /quit
// return 2 if the request is /ping
int request(int new_socket) {
    int exec = 0;
    int valread;
    int instruction;
    string message;
    char request[size_message + 1];

    print_name(NAME,"yellow");
    cout << "Aguardando request...\n";
    // receiving the message
    valread = recv(new_socket, &request, size_message + 1, 0);
    request[valread] = '\0';
    // the inicial char of the message is a flag char
    // so checks the flag
    // and concatenate the messages receved
    while (request[0] == 'i') { 
        concat_charA_to_str(&message, &request[1], valread);
        valread = recv(new_socket, &request, size_message + 1, 0);
        request[valread] = '\0';
    }
    concat_charA_to_str(&message, &request[1], valread - 1);
    print_name("Request","red");
    cout << message << endl;
    // check for commads
    if(message[0] == '/') exec = check_commands(message);
    message.clear();

    return exec;
}

// this function returns the response to the cliente
void response(int new_socket, int exec) {
    string message;
    char response[size_message + 1];

    // check if the last command where /quit or /exit
    // if it was /quit or /exit exec will be 1
    if(exec == 0) {
        print_name("Response","blue");
        getline(cin, message);
        /* sending message */
        response[0] = 'i';
        str_to_charA(&response[1], message, size_message);
        // checking if the message length is bigger than the max size possible
        // if it is divide the message em parts that are the same size of smoller than the max size
        // and send those litte messages 
        while(message.length() > size_message) {
            send(new_socket, &response, size_message + 1, 0);
            print_name(NAME,"yellow");
            cout << "Enviando response ";
            print_text(&response[1],"yellow",true);
            message = message.substr(size_message);
            str_to_charA(&response[1], message, size_message);
        }
    }
    // the last command received was /quit or /exit
    // so send a message to the client to end the program
    else if(exec == 1) {
        message = "Encerrando conexão com o cliente...";
        print_name(NAME,"yellow");
        print_text(message,"yellow",true);

        response[0] = 'e';
        send(new_socket, &response, message.length() + 1, 0);
    }
    // the las command received was /ping
    // so send the message pong
    else if(exec == 2) {
        message = "pong";
        response[0] = 'i';
        str_to_charA(&response[1], message, size_message);

        print_name("Response","blue");
        cout << message << endl;
    } 
    // exac is normal so just send the message.
    if(exec != 1) {
        response[0] = 'c';
        print_name(NAME,"yellow");
        cout << "Enviando response ";
        print_text(message,"yellow",true);
        send(new_socket, &response, message.length() + 1, 0);
    }

    print_name(NAME,"green");
    cout << "Response enviada!...\n";
    message.clear(); 

    return;
}

int main(int argc, char const *argv[]) { 
    struct sockaddr_in address; 
    int server_fd, new_socket, addrlen = sizeof(address), opt = 1;
    
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

    int exec = 0;
    // while the command /quit is not receved keep up
    while(exec != 1) {
        exec = request(new_socket);
        response(new_socket,exec);
    }

    close(new_socket);

    return 0; 
} 