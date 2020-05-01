// Client side C/C++ program to demonstrate Socket programming
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <thread>

#define PORT 8080        //número da porta de comunicação
#define size_id 11       //comprimento máximo do id de um usuário
#define size_message 20  //comprimento máximo de uma mensagem
#define size_nickname 50 //comprimento máximo do nome de usuário

using namespace std;

int exec = 0;      //código da operação recebida pelo servidor
int id = 0;        //identificador do usuário na aplicação
string nickname;   //nome do usuário no servidor
bool stop = false; //controlador de execução da aplicação

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

int choose_nickname(int server_socket)
{
    int user_id = 0;                  //identificador do usuário na aplicação
    char request[size_nickname + 1];  //bloco de requisição para o nome de usuário
    char response[size_nickname + 1]; //bloco de resposta para o nome de usuário enviado
    bool accept = false;              //flag para verificação da validade do nome

    int valread;        //número de bytes recebidos pela mensagem
    string response_id; //id enviado pelo servidor como resposta (0 corresponde a um número inválido)

    while (!accept)
    {
        print_name("Aplicação", "yellow");
        cout << "Insira o seu nome de usuário: ";
        getline(cin, nickname);

        if (nickname.length() > 50)
        {
            print_name("Aplicação", "yellow");
            cout << "Limite de 50 caracteres excedido. Insira outro nome" << endl;
            continue;
        }

        request[0] = 'n';
        str_to_charA(&request[1], nickname, size_nickname);
        send(server_socket, &request, nickname.length() + 1, 0);

        valread = recv(server_socket, &response, size_nickname + 1, 0);

        if (response[0] == 'n')
        {
            concat_charA_to_str(&response_id, &response[1], valread - 1);

            if (response_id.compare("0") == 0)
            {
                print_name("Aplicação", "yellow");
                cout << "Nome já cadastrado no servidor" << endl;
            }

            else
            {
                accept = true;
                user_id = stoi(response_id);

                print_name("Aplicação", "green");
                cout << "Bem-vindx, ";
                print_text(nickname, "green", false);
                cout << "!" << endl;
            }
        }

        else
        {
            print_name("Aplicação", "yellow");
            cout << "Falha na resposta recebida. Tente novamente" << endl;
        }
    }

    return user_id;
}

bool command_compare(string message, string command)
{
    if (message.size() < command.size())
        return false;
    for (int i = 0; i < command.size(); i++)
        if (message[i] != command[i])
            return false;
    return true;
}

int check_commands(string message)
{
    if (command_compare(message, "/exit") || command_compare(message, "/quit"))
        return 1;
    else if (command_compare(message, "/ping"))
        return 2;

    return 0;
}

int socket_init()
{
    int server_socket = 0;
    struct sockaddr_in serv_addr;

    print_name("Aplicação", "yellow");
    cout << "Configurando conexão...\n";
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Socket creation error \n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        cout << "Invalid address/ Address not supported\n";
        return -1;
    }

    while (connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        print_name("Aplicação", "yellow");
        cout << "Conectando...\n";
        sleep(3);
    }

    print_name("Aplicação", "green");
    cout << "Conexão estabelecida!\n";

    return server_socket;
}

void receive_message(int new_socket)
{
    int valread;
    int instruction;
    string message;
    char request[size_message + 1];

    while (true)
    {
        valread = recv(new_socket, &request, size_message + 1, 0);

        if (request[0] == 'e')
        {
            print_name("Outro usuário", "blue");
            print_text("Encerrou o chat", "red", true);
            exit(0);
            break;
        }

        request[valread] = '\0';
        while (request[0] == 'i')
        {
            concat_charA_to_str(&message, &request[1], valread);
            valread = recv(new_socket, &request, size_message + 1, 0);
            request[valread] = '\0';
        }

        if (request[0] != 'n')
        {
            concat_charA_to_str(&message, &request[1], valread - 1);

            print_name("Outro usuário", "red");
            cout << message << '\n';
            message.clear();
        }
    }

    return;
}

void send_message(int new_socket)
{
    string message, tmp_message;               //mensagem do usuário
    char response[size_id + size_message + 1]; //bloco de dados enviado ao servidor
    int id_lenght;                             //número de caracteres ocupados pelo id do cliente (incluindo o '\0')

    while (!stop)
    {
        if (exec == 0)
        {
            getline(cin, message);
            tmp_message = message;

            if (check_commands(message) == 1)
            {
                print_name(nickname, "blue");
                print_text("Encerrou o chat", "red", true);
                response[0] = 'q';
                id_lenght = sprintf(&response[1], "%d", id) + 1;
                str_to_charA(&response[1 + id_lenght], message, size_message);
                send(new_socket, &response, id_lenght + size_message + 1, 0);
                exec = 1;
                break;
            }

            response[0] = 'i';
            id_lenght = sprintf(&response[1], "%d", id) + 1;
            str_to_charA(&response[1 + id_lenght], message, size_message);

            while (tmp_message.length() > size_message)
            {
                send(new_socket, &response, id_lenght + size_message + 1, 0);
                tmp_message = tmp_message.substr(size_message);
                str_to_charA(&response[1 + id_lenght], tmp_message, size_message);
            }
        }

        else if (exec == 2)
        {
            message = "ping";
            response[0] = 'i';
            str_to_charA(&response[1 + id_lenght], message, size_message);
            cout << message << '\n';
        }

        if (exec != 1)
        {
            response[0] = 'c';
            print_name(nickname, "blue");
            print_text(message, "white", true);
            send(new_socket, &response, 1 + id_lenght + tmp_message.length(), 0);
        }

        message.clear();
        tmp_message.clear();
        memset(response, 0, size_id + size_message + 1);
    }

    return;
}

int main(int argc, char const *argv[])
{

    int new_socket = socket_init();
    id = choose_nickname(new_socket);

    thread receive_thread(receive_message, new_socket);
    thread response_thread(send_message, new_socket);

    receive_thread.join();
    response_thread.join();

    close(new_socket);

    return 0;
}