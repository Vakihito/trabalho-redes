/*--Observações--*/
/*

>> Para facilitar o processamento das mensagens, foram estabelecidos alguns códigos, os quais são
inseridos no 1 byte do bloco da requisição/resposta, de modo a identificar a operação correspondente.  
* 'i': outros blocos serão enviados para compor a mensagem final
* 'c': bloco final da mensagem
* 'e': encerra conexão do cliente com o servidor

*/

// Client side C/C++ program to demonstrate Socket programming 
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include <unistd.h>

#define NAME "Client"
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

// this function send the request to the server
void request(int server_socket) {
    char request[size_message + 1];
    string message;

    print_name("Request","blue");
    getline(cin, message);
    request[0] = 'i';
    str_to_charA(&request[1], message, size_message);
    // if the size of the message to be send is bigger than the max size
    // divede de messa in small requests and send message in parts
    while(message.length() > size_message) {
        send(server_socket, &request, size_message + 1, 0);
        print_name("Sistema","yellow");
        cout << "Enviando request ";
        print_text(&request[1],"yellow",true);
        message = message.substr(size_message);
        str_to_charA(&request[1], message, size_message);
    }
    // send the end of the message
    request[0] = 'c';
    print_name("Sistema","yellow");
    cout << "Enviando request ";
    print_text(message,"yellow",true);
    send(server_socket, &request, message.length() + 1, 0);
    print_name("Sistema","green");
    cout << "Requisição enviada!\n"; 
    message.clear();

    return;
}
// receives the response from the server
// return true if there was no problems
// return false if there was a disconnection
bool response(int server_socket) {
    bool exec = true;
    int valread; 
    char response[size_message + 1];
    string message;

    print_name("Sistema","yellow");
    cout << "Aguardando response...\n";
    valread = recv(server_socket, &response, size_message + 1, 0);
    response[valread] = '\0';
    // concatenate the smoll parts of the message received
    while(response[0] == 'i') {
        concat_charA_to_str(&message, &response[1], valread);
        valread = recv(server_socket, &response, size_message + 1, 0);
        response[valread] = '\0';
    }
    // checks if the server asked to disconnect
    if(response[0] == 'e'){
        print_name("Sistema","green");
        print_text("Desconectando do servidor...","green",true);
        // change exec to false so we return false by the end of this function
        exec = false;
    } 
    // if the server did not asked to disconnect just print the message
    // send by the server
    else {
        print_name("Response","red");
        concat_charA_to_str(&message, &response[1], valread - 1);
        cout << message << endl;
        message.clear();
    }
    return exec;
}
   
int main(int argc, char const *argv[]) { 
    int server_socket = 0;
    struct sockaddr_in serv_addr; 

    print_name("Sistema","yellow");
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

    print_name("Sistema","yellow");
    cout << "Conectando...\n";
    if (connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        cout << "404 - Não foi possível encontrar o servidor \n";
        return -1; 
    }

    print_name("Sistema","green");
    cout << "Conexão estabelecida!\n";

    bool exec = true;
    // runs while we do not send /quit
    while(exec) {
        request(server_socket);
        exec = response(server_socket);
    }

    close(server_socket);

    return 0; 
} 