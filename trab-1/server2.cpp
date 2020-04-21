/*--Observações--*/
/*

>> Para facilitar o processamento das mensagens, foram estabelecidos alguns códigos, os quais são
inseridos no 1 byte do bloco da requisição/resposta, de modo a identificar a operação correspondente.  
* 'i': outros blocos serão enviados para compor a mensagem final
* 'c': bloco final da mensagem
* 'e': encerra conexão do cliente com o servidor
* 'n': processamento de mensagem relacionada ao nome de um usuário

*/

// Server side C/C++ program to demonstrate Socket programming
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>

#define NAME "Server"    //nome do servidor
#define PORT 8080        //número da porta de comunicação
#define size_id 11       //comprimento máximo do id de um usuário
#define size_message 20  //comprimento máximo de uma mensagem
#define size_nickname 50 //comprimento máximo do nome de usuário

using namespace std;

int exec = 0;                             //código para identificação da operação solicitada
int user_id = 0;                          //variável para controle de criação dos identificadores de usuário
map<int, pair<string, int>> users;        //estrutura para armazenamento das informações de cada usuário
map<int, pair<string, int>>::iterator it; //iterador para busca dos usuários

void print_name(string name, string color)
{
    if (color.compare("red") == 0)
        cout << "\033[1;31m"
             << "[" << name << "]: "
             << "\033[0m";
    else if (color.compare("green") == 0)
        cout << "\033[1;32m"
             << "[" << name << "]: "
             << "\033[0m";
    else if (color.compare("yellow") == 0)
        cout << "\033[1;33m"
             << "[" << name << "]: "
             << "\033[0m";
    else if (color.compare("blue") == 0)
        cout << "\033[1;34m"
             << "[" << name << "]: "
             << "\033[0m";
    else if (color.compare("white") == 0)
        cout << "\033[1;314m" << name << "\033[0m";
    else
        cout << "[" << name << "]: ";

    return;
}

void print_text(string text, string color, bool new_line)
{
    if (color.compare("red") == 0)
        cout << "\033[1;31m" << text << "\033[0m";
    else if (color.compare("green") == 0)
        cout << "\033[1;32m" << text << "\033[0m";
    else if (color.compare("yellow") == 0)
        cout << "\033[1;33m" << text << "\033[0m";
    else if (color.compare("blue") == 0)
        cout << "\033[1;34m" << text << "\033[0m";
    else if (color.compare("white") == 0)
        cout << "\033[1;314m" << text << "\033[0m";
    else
        cout << text;

    if (new_line)
        cout << endl;

    return;
}

void str_to_charA(char *char_array, string str, unsigned int n)
{
    for (unsigned int i = 0; i < n && i < str.length(); i++)
        char_array[i] = str[i];
    return;
}

// char_array length must be smaller or equal to n
void concat_charA_to_str(string *str, char *char_array, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
        (*str).push_back(char_array[i]);
    return;
}

bool command_compare(string message, string command){
    if (message.size() < command.size())
        return false;
    for (int i = 0; i < command.size(); i++)
        if (message[i] != command[i])
            return false;
    return true;        
    
}

void print(string s){
    cout << "|" << s << "|" <<endl;
}

int check_commands(string message)
{
    if (command_compare(message,"/exit") || command_compare(message,"/quit"))
        return 1;
    else if (command_compare(message,"/ping"))
        return 2;

    return 0;
}

int socket_init()
{
    struct sockaddr_in address;
    int server_fd, new_socket, addrlen = sizeof(address), opt = 1;

    print_name(NAME, "yellow");
    cout << "Configurando conexão..." << endl;
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    print_name(NAME, "yellow");
    cout << "Aguardando conexão...\n";

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    print_name(NAME, "green");
    cout << "Conexão estabelecida!\n";

    return new_socket;
}

int get_id(char *message, int *request_id)
{
    string message_id;
    int pos = 1;

    while (message[pos] != '\0')
    {
        message_id.push_back(message[pos]);
        pos++;
    }

    *request_id = stoi(message_id);

    return pos;
}

void receive_message(int user_socket)
{
    int valread;
    int instruction;
    string message;
    char request[size_message + 1];

    while (exec != 1)
    {
        valread = recv(user_socket, &request, size_id + size_message + 1, 0);
        request[valread] = '\0';

        if (request[0] == 'n')
        {
            bool find = false;
            concat_charA_to_str(&message, &request[1], valread - 1);
            char response[size_nickname + 1];
            response[0] = 'n';

            for (it = users.begin(); it != users.end(); ++it)
                if (message.compare(it->second.first) == 0)
                {
                    find = true;
                    break;
                }

            if (!find)
            {
                user_id++;
                it = users.begin();
                pair<string, int> user = make_pair(message, user_socket);

                sprintf(&response[1], "%d", user_id);
                users.insert(it, pair<int, pair<string, int>>(user_id, user));
                print_name(NAME, "green");
                print_text(message, "green", false);
                cout << " entrou no chat" << endl;
            }

            else
                sprintf(&response[1], "%d", 0);

            send(user_socket, &response, size_nickname + 1, 0);
        }

        else
        {
            int request_id = 0;
            int id_lenght = 0;

            while (request[0] == 'i')
            {
                id_lenght = get_id(request, &request_id);
                concat_charA_to_str(&message, &request[1 + id_lenght], valread);
                valread = recv(user_socket, &request, size_id + size_message + 1, 0);
                request[valread] = '\0';
            }

            id_lenght = get_id(request, &request_id);
            concat_charA_to_str(&message, &request[1 + id_lenght], valread - 1);

            map<int, pair<string, int>>::iterator request_user = users.find(request_id);

            print_name(request_user->second.first, "red");
            cout << message << endl;

            if (message[0] == '/')
                exec = check_commands(message);
        }

        message.clear();
        memset(request, 0, size_id + size_message + 1);
    }

    return;
}

void send_message(int new_socket)
{
    string message, tmp_message;
    char response[size_message + 1];

    while (true)
    {
        if (exec == 0)
        {
            getline(cin, message);
            tmp_message = message;

            response[0] = 'i';
            str_to_charA(&response[1], message, size_message);
            while (tmp_message.length() > size_message)
            {
                send(new_socket, &response, size_message + 1, 0);
                tmp_message = tmp_message.substr(size_message);
                str_to_charA(&response[1], tmp_message, size_message);
            }
        }

        else if (exec == 1)
        {
            message = "Encerrando conexão com o cliente...";
            print_name(NAME, "yellow");
            print_text(message, "yellow", true);

            response[0] = 'e';
            send(new_socket, &response, message.length() + 1, 0);
            break;
        }

        else if (exec == 2)
        {
            message = "ping";
            response[0] = 'i';
            str_to_charA(&response[1], message, size_message);
            cout << message << '\n';
        }

        if (exec != 1)
        {
            response[0] = 'c';
            print_name(NAME, "blue");
            print_text(message, "white", true);
            send(new_socket, &response, tmp_message.length() + 1, 0);
        }

        message.clear();
        tmp_message.clear();
    }

    return;
}

int main(int argc, char const *argv[])
{

    int new_socket = socket_init();

    thread receive_thread(receive_message, new_socket);
    thread response_thread(send_message, new_socket);

    receive_thread.join();
    response_thread.join();

    close(new_socket);

    return 0;
}