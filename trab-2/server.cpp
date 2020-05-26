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
#include <map>

#define PORT 8888        // número da porta de comunicação
#define NAME "Server"    // nome do servidor
#define max_token 596947 // maior valor de token
#define size_message 10  // comprimento máximo de uma mensagem enviada pelo usuário
#define size_token 6     // número de caracteres do token do usuário

//códigos presentes na requisição para identificar o tipo de operação solicitada
#define FLAG_EXIT 'e'           //término de conexão com o servidor
#define FLAG_INCOMPLETE 'i'     //mensagem incompleta
#define FLAG_COMPLETE 'c'       //mensagem completa
#define FLAG_PING 'p'           //instrução "ping-pong"

using namespace std;

map<int, pair<string, int>> users;          // estrutura para armazenamento das informações de cada usuário
map<int, pair<string, int>>::iterator it;   // iterador para busca dos usuários

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
    for(unsigned int i = 0; i < n and i < str.length(); i++)
        char_array[i] = str[i];
    char_array[n < str.length() ? n : str.length()] = '\0';
    return char_array;
}

void concat_charA_to_str(string *str, char *char_array, unsigned int n) {
    for(unsigned int i = 0; i < n; i++) 
        (*str).push_back(char_array[i]);

    return;
}

int main(int argc, char *argv[]) {
    int opt = true;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    //inicializa a semente do gerador aleatório 
    srand(time(NULL));

    string buffer;

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

            int token = rand() % max_token + 1;

            while(users.find(token) != users.end()) {
                token = rand() % max_token + 1;
            }

            //armazenando informções do novo usuário
            pair<string, int> user = make_pair("User #" + to_string(token), new_socket);
            users.insert(make_pair(token,user));

            //envia token de autenticação ao usuário
            string message = to_string(token);
            char *tmp_message = str_to_charA(message, message.length());
            if(send(new_socket, tmp_message, message.length(), 0) != message.length()) perror("send");

            free(tmp_message);
            
            print_name(NAME,"blue");
            cout << "Welcome message sent successfully\n";

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

            if(FD_ISSET( sd, &readfds)) {
                //Verifica se é uma requisição de encerramento e retorna a mensagem adequada, 
                //colocando o primeiro caracter para identificação do cliente 
                char *tmp_buffer = str_to_charA(buffer, 1024);
                if((valread = read( sd, tmp_buffer, 1024)) == 0) {
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
                    tmp_buffer[valread] = '\0';                  //insere o byte indicador de término de string no fim do conteúdo lido
                    string tokenClient (tmp_buffer, size_token); //token do cliente no formato de string
                    char op_code = tmp_buffer[size_token];       //código da operção requisitada

                    //procura e imprime o nome do usuário
                    string nickname = users.find(stoi(tokenClient))->second.first;
                    print_name(nickname, "green");

                    //executa a operação correspondente à solicitação do cliente
                    string response;        //resposta geral
                    string response_self;   //resposta específica para o cliente

                    //mensagem comum
                    if(op_code == FLAG_INCOMPLETE || op_code == FLAG_COMPLETE) {
                        string text;    
                        while(op_code == FLAG_INCOMPLETE) 
                            text = text + buffer.substr(size_token + 1);
                        cout << text << endl;
                        
                        //imprime a mensagem recebida
                        cout << &tmp_buffer[size_token + 1] << endl;
                        for (int j = 0; j < max_clients; j++)
                            send(client_socket[j], tmp_buffer, valread, 0); 
                    }
                    //alguém se desconectou
                    else if(op_code == FLAG_EXIT) {
                        print_text("/exit","yellow",true);
                        //procura por suas informações e as imprime  
                        getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                        print_name(NAME,"red");
                        cout << "Host disconnected, ip " << inet_ntoa(address.sin_addr) << ", port " << ntohs(address.sin_port) << '\n';
                        //envia mensagem de fim de conexão para os clientes
                        response = tokenClient + "\033[1;31m" + nickname + " left the chat" + "\033[0m"; 
                        tmp_buffer = str_to_charA(response, size_message);
                        for (int j = 0; j < max_clients; j++)
                            send(client_socket[j], tmp_buffer, size_message, 0);
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
                    else if(op_code == FLAG_PING) {
                        print_text("/ping","yellow",true);
                        response = tokenClient + "\033[1;314mP O N G\033[0m"; 
                        tmp_buffer = str_to_charA(response, size_message);
                        for (int j = 0; j < max_clients; j++)
                            send(client_socket[j], tmp_buffer, size_message, 0);
                    }   

                    free(tmp_buffer);
                }
            } 
        }
    }

    return 0;
}