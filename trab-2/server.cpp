#include <iostream>
#include <string>       
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>       
#include <arpa/inet.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h>
#include <cstring> 
#include <map>

#include "chat.h"

#define NAME "Server"    // nome do servidor

using namespace std;

map<int, pair<string, int>> users;          // estrutura para armazenamento das informações de cada usuário
map<int, pair<string, int>>::iterator it;   // iterador para busca dos usuários
map<int, string> cache;

int main(int argc, char *argv[]) {
    int opt = true;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    //inicializa a semente do gerador aleatório 
    srand(time(NULL));

    //set of socket descriptors  
    fd_set readfds;

    //initialise all client_socket[] to 0 so not checked  
    for(i = 0; i < max_clients; i++)
        client_socket[i] = 0;

    //create a master socket  
    if((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections, this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created  
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888  
    if(bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    print_name(NAME,"blue");
    cout << "Listener on port " << PORT << '\n';

    //try to specify maximum of 3 pending connections for the master socket  
    if(listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    print_name(NAME,"blue");
    cout << "Waiting for connections ...\n";

    while(true) {
        //clear the socket set  
        FD_ZERO(&readfds);

        //add master socket to set  
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set  
        for( i = 0 ; i < max_clients ; i++) { 
            sd = client_socket[i];              //socket descriptor 
            if(sd > 0) FD_SET( sd, &readfds);   //if valid socket descriptor then add to read list  
            if(sd > max_sd) max_sd = sd;        //highest file descriptor number, need it for the select function  
        }

        //wait for an activity on one of the sockets, timeout is NULL,so wait indefinitely  
        activity = select( max_sd + 1, &readfds, NULL, NULL, NULL);

        if((activity < 0) && (errno!=EINTR)) cout << "select error\n";

        //if something happened on the master socket,then its an incoming connection  
        if(FD_ISSET(master_socket, &readfds)) {
            if((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands 
            print_name(NAME,"green"); 
            cout << "New connection, socket fd is " << new_socket << ", ip is : " << inet_ntoa(address.sin_addr) << ", port : " << ntohs(address.sin_port) << '\n';

            int token = rand() % max_token;

            while(users.find(token) != users.end())
                token = rand() % max_token;

            //armazenando informções do novo usuário (username ainda não setado)
            users[token] = make_pair("", new_socket);

            //envia token de autenticação ao usuário
            string message = to_string(token);
            char *tmp_message = str_to_charA(message, message.length());
            send(new_socket, tmp_message, message.length(), 0);
            free(tmp_message);
            
            print_name(NAME,"blue");
            cout << "User token sent successfully\n";

            //add new socket to array of sockets  
            for(i = 0; i < max_clients; i++) {
                //if position is empty  
                if(client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    print_name(NAME,"blue");
                    cout << "Adding to list of sockets as " << i << '\n';
                    break;
                }
            }
        }

        //else its some IO operation on some other socket 
        for(i = 0; i < max_clients; i++) {
            sd = client_socket[i];

            if(FD_ISSET(sd, &readfds)) {
                //Verifica se é uma requisição de encerramento e retorna a mensagem adequada, 
                //colocando o primeiro caracter para identificação do cliente 
                char buffer[size_message + size_token + 1];
                valread = read(sd, buffer, size_message + size_token + 1);
                buffer[valread] = '\0'; 
                string buffer_str = &buffer[1 + size_token];

                if(valread == 0) {
                    //Alguém se desconectou, procura por suas informações e as imprime  
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    print_name(NAME,"red");
                    cout << "Host disconnected, ip " << inet_ntoa(address.sin_addr) << ", port " << ntohs(address.sin_port) << '\n';

                    //Fecha o socket e marca como 0 na lista para reutilização  
                    close(sd);
                    //Remove as informações relacionadas ao usuário
                    for(it = users.begin(); it != users.end(); it++) {
                         if(it->second.second == client_socket[i]) {
                            users.erase(it);
                            break;
                         }
                    }
                         
                    client_socket[i] = 0;
                }

                //transmite a mensagem recebida 
                else {
                    string tokenClient (buffer, size_token);    //token do cliente no formato de string
                    string username;
                    char op_code = buffer[size_token];          //código da operação requisitada
                    int token = stoi(tokenClient);

                    if(op_code == FLAG_CHANGE_USERNAME[0]) {
                        bool find = false;
                        string name;
                        char response;

                        concat_charA_to_str(&name,&buffer[size_token + 1],size_username);
                        for(it = users.begin(); it != users.end(); ++it) {  
                            if(name.compare(it->second.first) == 0) {
                                find = true;
                                break;
                            }
                        }

                        if(find == false) { 
                            response = VALID_USERNAME;
                            users[token].first = name;
                        }

                        else response = INVALID_USERNAME;

                        send(client_socket[i], &response, 1, 0);
                        continue;
                    }
                    
                    username = users[token].first;
                    //procura e imprime o nome do usuário
                    print_name(username, "green");

                    //executa a operação correspondente à solicitação do cliente
                    string response = "";       
                    //mensagem comum
                    if(op_code == FLAG_INCOMPLETE[0]) {
                        cache[token] += buffer_str;
                    }
                    else if(op_code == FLAG_COMPLETE[0]) {
                        cache[token] += buffer_str;
                        cout << cache[token] << endl;
                        cache[token] = CLIENT + users[token].first + cache[token];
                        char *tmp = str_to_charA(cache[token], 1 + size_username + size_message);
                        for(int j = 0; j < max_clients; j++)
                            send(client_socket[j], tmp, 1 + size_username + size_message, 0);
                        free(tmp);
                        cache[token] = "";
                    }

                    //alguém se desconectou 
                    else if(op_code == FLAG_EXIT[0]) {
                        string origin = SERVER;
                        print_text("/quit", "yellow", true);
                        //procura por suas informações e as imprime  
                        getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                        print_name(NAME,"red");
                        cout << "Host disconnected, ip " << inet_ntoa(address.sin_addr) << ", port " << ntohs(address.sin_port) << '\n';
                        //envia mensagem de fim de conexão para os clientes
                        response = origin + "\033[1;31m" + username.substr(0,username.find('\0')) + " left the chat" + "\033[0m"; 
                        cout << response << endl;
                        char *tmp = str_to_charA(response, 1 + size_message);
                        for (int j = 0; j < max_clients; j++)
                            send(client_socket[j], tmp, 1 + size_message, 0);
                        free(tmp);
                        //Fecha o socket e marca como 0 na lista para reutilização  
                        close(sd);
                        //Remove as informações relacionadas ao usuário
                        for(it = users.begin(); it != users.end(); it++) {
                            if(it->second.second == client_socket[i]) {
                                users.erase(it);
                                break;
                            }
                        } 
                        client_socket[i] = 0;
                    }

                    //comando "/ping"
                    else if(op_code == FLAG_PING[0]) {
                        string origin = SERVER;
                        print_text("/ping","yellow",true);
                        response = origin + "\033[1;314mP O N G\033[0m"; 
                        char *tmp = str_to_charA(response, 1 + size_message);
                        for (int j = 0; j < max_clients; j++)
                            send(client_socket[j], tmp, 1 + size_message, 0);
                        free(tmp);
                    }
                }
            } 
        }
    }

    return 0;
}