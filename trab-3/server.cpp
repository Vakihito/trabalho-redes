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
#include <vector>
#include <ctype.h>

#include "chat.h"

#define NAME "Server"    // nome do servidor

using namespace std;

map<int, pair<string, int>> users;          // <token, <nome, socket>>
map<int, string> ips;                       // <token, ip>
map<string, vector<int>> channels;          // <channel, <tokens>>
map<int, vector<string>> mutes;             // <tokens, channel>
map<int, vector<string>> invites;           // <tokens  , channel >; guarda em um mapa os invites recebidos por um usuario
map<string, bool> channel_status;           // <channel, status>; se status == true eh aberto, se nao eh fechado
map<int, pair<string, int>>::iterator it;   // iterador para busca dos usuários
map<int, string> cache;

void send_to_all(int client_socket[], int max_clients, char *message, int _size_message) {
    for(int j = 0; j < max_clients; j++){
        if(client_socket[j] == 0) continue;
        int max_attempts = 5, attempt = 0;
        while(attempt < max_attempts && send(client_socket[j], message, _size_message, 0) == -1) {
            cout << "erro" << endl;
            attempt++;
        }
        if(attempt == max_attempts){
            //Fecha o socket e marca como 0 na lista para reutilização  
            int remove_token;
            close(j);
            //Remove as informações relacionadas ao usuário
            for(it = users.begin(); it != users.end(); it++) {
                if(it->second.second == client_socket[j]) {
                    remove_token = it->first;
                    users.erase(it);
                    ips.erase(remove_token);
                    break;
                }
            } 
            client_socket[j] = 0;
        }
    }
}

bool check_if_mute_exist(int token){
    if (mutes.find(token) == mutes.end())
        return false;
    return true;
}

bool check_if_already_mute(int token, string chan ){
    // verifica se existe
    if (! check_if_mute_exist(token))
        return false;
    
    // percorre os canais que o usuário está mutado
    for (int i = 0; i < mutes[token].size(); i++) {
        // ele ja está mutado no canal
        if (mutes[token][i].compare(chan) == 0)
            return true;   
    }
    return false;
}

void send_to_channel(int client_socket[], int max_clients, char *message, int _size_message, string chan, int token_sender, bool isCommand = false) {

    if(!isCommand && check_if_already_mute(token_sender, chan)) {
        string origin = SERVER;
        string response = origin + "\033[1;31mYou are muted!\033[0m";
        char *tmp = str_to_charA(response, 1 + response.length());
        send(users[token_sender].second, tmp, response.size() + 1, 0);
        return;
    }
        
    int size_channel = int(channels[chan].size());
    // percorrendo usuários do canal
    for (int i = 0; i < size_channel; i++){
        send(users[channels[chan][i]].second, message, _size_message, 0);
    }
}

string find_channel(int token){
    std::map<std::string, vector<int>>::iterator ite = channels.begin();

    while(ite != channels.end()) {
        int size_channel = int(ite->second.size());
        // percorrendo usuários
        for (int i = 0; i < size_channel; i++) {
            if (token == ite->second[i]) {
                return ite->first;
            }
        }
        ite++;
    }
    return "";
}

bool remove_user_from_channel(int token, string chan){
    std::map<std::string, vector<int>>::iterator ite = channels.begin();
    // percorrendo os canais
    while (ite != channels.end()) {
        int size_channel = int(ite->second.size());
        // percorrendo usuários
        for (int i = 0; i < size_channel; i++) {
            if (token == ite->second[i]) {
                // removendo o elemento do canal
                ite->second.erase(ite->second.begin() + i);
                // caso o canal esteja vazio simplesmente elimine-o tambem elimine seu status
                if (ite->second.empty()) {
                    channels.erase(ite);
                    channel_status.erase(ite->first);
                }
                return true;
            }
        }
        ite++;
    }
    return false;
}

string clear_name(string namecmp){
    return namecmp.substr(0,namecmp.find('\0'));
}

int find_token_by_name(string user_name){
    map<int, pair<string, int>>::iterator ite = users.begin();
    // iterando em relação aos usuários
    string name_to_copare;

    while(ite != users.end()) {
        if (user_name.compare(clear_name(ite->second.first)) == 0)
            return ite->first;   
        ite ++;
    }
    return -1;
}

void remove_from_mute(int token_to_remove ,string chan){
    int vec_size = mutes[token_to_remove].size();
    for (int i = 0; i < vec_size; i++) {
        if (mutes[token_to_remove][i].compare(chan) == 0){
            mutes[token_to_remove].erase(mutes[token_to_remove].begin() + i);
            return;
        }
    }
}

string check_if_admin(int token, bool *isAdm){
    std::map<std::string, vector<int>>::iterator ite = channels.begin();
    
    while (ite != channels.end()) {
        if (ite->second.size() > 0 && ite->second[0] == token) {
            (*isAdm) = true; 
            return ite->first;
        }
        ite ++;
    }
    (*isAdm) = false;
    return "";
}

bool check_if_already_invited (string chan, int token_to_invite ){
    vector <string> channels_invited = invites[token_to_invite];
    int size_vet_channelsi = int(channels_invited.size());
    for (int i = 0; i < size_vet_channelsi; i++)
        if (channels_invited[i].size() > 0 && channels_invited[i].compare(chan) == 0)
            return true;
    
    return false;
}

/* Retorna o token no formato padrão para inserção no cabeçalho das respostas */  
string fill_nickname(int token) {
    string stoken_s = to_string(token);
    while (stoken_s.length() < size_token)
        stoken_s = "0" + stoken_s;
    return stoken_s;
}

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
    if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created  
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

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
            
            string new_ip = inet_ntoa(address.sin_addr); 

            //inform user of socket number - used in send and receive commands 
            print_name(NAME,"green"); 
            cout << "New connection, socket fd is " << new_socket << ", ip is : " << new_ip << ", port : " << ntohs(address.sin_port) << '\n';

            int token = rand() % max_token;

            while(users.find(token) != users.end())
                token = rand() % max_token;

            //armazenando informações do novo usuário (username ainda não definido)
            users[token] = make_pair("", new_socket);
            ips[token] = new_ip;

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
                //Verifica se é uma requisição de encerramento e retorna a mensagem adequada, colocando o primeiro caracter para identificação do cliente 
                char buffer[size_message + size_token + 1];
                valread = read(sd, buffer, size_message + size_token + 1);
                buffer[valread] = '\0'; 
                string buffer_str = &buffer[1 + size_token];
                int remove_token;

                if(valread == 0) {
                    //Alguém se desconectou, procura por suas informações e as imprime  
                    string tokenClient (buffer, size_token);    //token do cliente no formato de string
                    int token = stoi(tokenClient);
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    
                    print_name(NAME,"red");
                    cout << "Host disconnected, ip " << inet_ntoa(address.sin_addr) << ", port " << ntohs(address.sin_port) << '\n';

                    //Fecha o socket e marca como 0 na lista para reutilização  
                    close(sd);
                    //Remove as informações relacionadas ao usuário
                    for(it = users.begin(); it != users.end(); it++) {
                         if(it->second.second == client_socket[i]) {
                            remove_token = it->first;
                            users.erase(it);
                            ips.erase(remove_token);
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

                    //executa a operação correspondente à solicitação do cliente
                    string response = ""; 

                    //mensagem comum
                    if(op_code == FLAG_INCOMPLETE[0]){
                        cache[token] += buffer_str;
                    }

                    // commando para criar/entrar em um canal
                    else if(op_code == FLAG_CHANNEL[0]){
                        // o canal ainda não existia
                        string responseAux = "";
                        string origin = SERVER;
                        // se um usuário sair e entar no mesmo canal ele vira o último da fila
                        bool check_if_invited = check_if_already_invited(buffer_str, token);
                        if(channels.find(buffer_str) == channels.end()) {
                            print_name(NAME, "green");
                            remove_user_from_channel(token,buffer_str);
                            // cria o vector e insere no canal
                            vector <int> users_channel = {token};
                            channels[buffer_str] = users_channel;

                            // marcando o canal como aberto
                            channel_status[buffer_str] = true;
                            origin = CONNECTED;

                            // enviando mensagem de resposta
                            responseAux = clear_name(users[token].first) + " created " + buffer_str;
                            response = origin + "\033[1;33m" + responseAux + "\033[0m";
                            cout  << responseAux << endl;
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send_to_channel(client_socket, max_clients, tmp, 1 + response.length(), buffer_str, token);
                            free(tmp);
                        }
                        else if(!check_if_invited && !channel_status[buffer_str]){
                            origin = DISCONNECTED;
                            response = origin + "\033[1;31mYOU WERE NOT INVITED!\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);   
                            free(tmp);  
                        }
                        else{
                            remove_user_from_channel(token,buffer_str);
                            print_name(NAME, "green");
                            channels[buffer_str].push_back(token);
                            responseAux = clear_name(users[token].first) + " joined " + buffer_str;
                            response = origin + "\033[1;33m" + responseAux + "\033[0m";
                            cout  << responseAux << endl;
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send_to_channel(client_socket, max_clients, tmp, 1 + response.length(), buffer_str, token, true);
                            free(tmp);

                            response = CONNECTED;
                            char *tmp2 = str_to_charA(response, 1 + response.length());
                            send(sd, tmp2, response.size() + 1, 0);
                            free(tmp2);
                        }                    
                    }
                    
                    // comando para kickar uma pessoa
                    else if(op_code == FLAG_KICK[0]){
                        //procura e imprime o nome do usuário
                        print_name(clear_name(username), "green");
                        cout << "/kick " << buffer_str << endl;

                        bool isAdm = false;
                        string adm_channel = check_if_admin(token, &isAdm);
                        int token_to_remove = find_token_by_name(buffer_str);
                        string channel_user_to_remove = find_channel(token_to_remove);

                        if(isAdm && token_to_remove != -1 && channel_user_to_remove.compare(adm_channel) == 0) {
                            string origin = SERVER;
                            string disconnected_response = DISCONNECTED;

                            response = origin + "\033[1;31m" + buffer_str + " have been kicked from " + adm_channel + "\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send_to_channel(client_socket, max_clients, tmp, 1 + size_username + size_message,find_channel(token), token);
                            
                            char *disconnected = str_to_charA(disconnected_response, disconnected_response.length());
                            send(users[token_to_remove].second, disconnected, disconnected_response.length(), 0);

                            remove_user_from_channel(token_to_remove,adm_channel);
                            free(tmp);
                            free(disconnected);
                        }

                        else if(!isAdm) { // induvíduo nao eh adm
                            string origin = SERVER;
                            response = origin + "\033[1;31mPERMISSION DENIED,YOU ARE NOT THE ADM!\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);
                            free(tmp);          
                        }

                        else {   // nao foi encontrado o usuário a ser removido
                            string origin = SERVER;
                            response = origin + "\033[1;33mUser not found in your channel! \033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);    
                            free(tmp);
                        } 
                    }
                    
                    // comando para se mutar uma pessoa
                    else if(op_code == FLAG_MUTE[0]){
                        //procura e imprime o nome do usuário
                        print_name(clear_name(username), "green");
                        cout << "/mute " << buffer_str << endl;

                        bool isAdm = false;
                        string adm_channel = check_if_admin(token, &isAdm);
                        int token_to_mute = find_token_by_name(buffer_str);
                        string channel_user_to_remove = find_channel(token_to_mute);
                        bool check_if_mute_already_on = check_if_already_mute(token_to_mute, adm_channel);
                        
                        if(isAdm && token_to_mute != -1 && channel_user_to_remove.compare(adm_channel)  == 0 && !check_if_mute_already_on) {
                            // insere no vetor de mutes.
                            if(! check_if_mute_exist(token_to_mute)) {
                                vector <string> channels_muted = {adm_channel};
                                mutes[token_to_mute] = channels_muted;
                            }
                            
                            else {
                                mutes[token_to_mute].push_back(adm_channel);
                            }

                            string origin = SERVER;
                            response = origin + "\033[1;31m" + adm_channel + " muted " + buffer_str + "\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            int size_channel = int(channels[adm_channel].size());
                            // percorrendo usuários do canal
                            for (int i = 0; i < size_channel; i++)
                                send(users[channels[adm_channel][i]].second, tmp, 1 + response.length(), 0);
                            free(tmp);
                        }

                        else if(!isAdm) { // induvíduo nao eh adm
                            string origin = SERVER;
                            response = origin + "\033[1;31mPERMISSION DENIED,YOU ARE NOT THE ADM!\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);              
                        }

                        else if( check_if_mute_already_on) {
                            string origin = SERVER;
                            response = origin + "\033[1;33m" + buffer_str + " is already muted.\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);
                            free(tmp);
                        }

                        else {   // nao foi encontrado o usuário a ser removido
                            
                            string origin = SERVER;
                            response = origin + "\033[1;33mUser not found in your channel!\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);
                            free(tmp);    
                        } 
                    }
                    
                    // comando para se desmutar uma pessoa
                    else if(op_code == FLAG_UNMUTE[0]){
                        //procura e imprime o nome do usuário
                        print_name(clear_name(username), "green");
                        cout << "/unmute " << buffer_str << endl;

                        bool isAdm = false;
                        string adm_channel = check_if_admin(token, &isAdm);
                        int token_to_unmute = find_token_by_name(buffer_str);
                        string channel_user_to_remove = find_channel(token_to_unmute);
                        bool check_if_mute_already_on = check_if_already_mute(token_to_unmute, adm_channel);
                        
                        if(isAdm && token_to_unmute != -1 && channel_user_to_remove.compare(adm_channel)  == 0 && check_if_mute_already_on) {
                            // insere no vetor de mutes.
                            remove_from_mute(token_to_unmute, adm_channel);
                            string origin = SERVER;
                            response = origin + "\033[1;34m" + adm_channel + " unmuted " + buffer_str + "\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            int size_channel = int(channels[adm_channel].size());
                            // percorrendo usuários do canal
                            for (int i = 0; i < size_channel; i++)
                                send(users[channels[adm_channel][i]].second, tmp, 1 + response.length(), 0);
                            free(tmp);
                        }

                        else if(!isAdm) { // induvíduo nao eh adm
                            // cout << "here 6" << endl;
                            string origin = SERVER;
                            response = origin + "\033[1;31mPERMISSION DENIED,YOU ARE NOT THE ADM!\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);   
                            free(tmp);           
                        }

                        else if( !check_if_mute_already_on) {
                            // cout << "here 7" << endl;
                            string origin = SERVER;
                            response = origin + "\033[1;33m" + buffer_str + " is already unmuted." + "\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);
                            free(tmp);
                        }

                        else{   // nao foi encontrado o usuário a ser removido
                            // cout << "here 5" << endl;
                            
                            string origin = SERVER;
                            response = origin + "\033[1;33mUser not found in your channel!\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);
                            free(tmp);              
                        } 
                    }
                    
                    // comando para se convidar alguem
                    else if(op_code == FLAG_INVITE[0]){
                        cout << "/invite " << buffer_str << endl;

                        bool isAdm = false;
                        string adm_channel = check_if_admin(token, &isAdm);
                        // cout << "here 1" << endl;

                        int token_to_invite = find_token_by_name(buffer_str);
                        // cout << "here 2" << endl;
                        bool check_already_invited = check_if_already_invited(adm_channel, token_to_invite);

                        
                        if (isAdm && token_to_invite != -1 &&  !check_already_invited){
                            // se o usuario nao tiver nenhum invite cria o vetor de invites para ele
                            if (invites[token_to_invite].empty()){
                                vector <string> invites_to_create;
                                invites[token_to_invite] = invites_to_create;
                            }
                            
                            invites[token_to_invite].push_back(adm_channel);
                            int socket_to_invite  = users[token_to_invite].second;

                            string origin = SERVER;
                            response = origin + "\033[1;34m" + adm_channel + " invited " + buffer_str + " to join the channel\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(socket_to_invite, tmp, 1 + response.length(), 0);
                        }
                        else if( !isAdm){ // induvíduo nao eh adm
                            // cout << "here 6" << endl;

                            string origin = SERVER;
                            response = origin + "\033[1;31mPERMISSION DENIED,YOU ARE NOT THE ADM!\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);   
                            free(tmp);           
                        }
                        else if (check_already_invited)
                        {
                            string origin = SERVER;
                            response = origin + "\033[1;31mTHE USER HAS ALREADY BEEN INVITED \033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);   
                            free(tmp);  
                        }
                        else{   // nao foi encontrado o usuário a ser removido
                            // cout << "here 5" << endl;
                            
                            string origin = SERVER;
                            response = origin + "\033[1;33mUser not found! \033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);
                            free(tmp);              
                        } 
                        
                    }

                    // comando para se trocar o servidor de modo
                    else if(op_code == FLAG_MODE[0]){
                        cout << "/mode" << buffer_str << endl;

                        bool isAdm = false;
                        string adm_channel = check_if_admin(token, &isAdm);
                        
                        if(isAdm){
                            string mode_change = "";
                            if (buffer_str.compare("0") == 0){
                                channel_status[adm_channel] = false;
                                mode_change = "close";
                            }
                            else if(buffer_str.compare("1") == 0){
                                channel_status[adm_channel] = true;
                                mode_change = "open";
                            }
                            
                            string origin = SERVER;
                            response = origin + "\033[1;34m"+ adm_channel + " mode changed to " + mode_change + "\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send_to_channel(client_socket, max_clients, tmp, 1 + response.length(), adm_channel, token, true);
                        }
                        else{ // induvíduo nao eh adm
                            string origin = SERVER;
                            response = origin + "\033[1;31mPERMISSION DENIED,YOU ARE NOT THE ADM! \033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send(sd, tmp, response.size() + 1, 0);   
                            free(tmp);           
                        }       
                    }

                    // comando para identificar o ip de uma pessoa
                    else if(op_code == FLAG_WHOIS[0]){
                        //procura e imprime o nome do usuário
                        print_name(clear_name(username), "green");
                        cout << "/whois " << buffer_str << endl;

                        bool isAdm = false;
                        string adm_channel = check_if_admin(token, &isAdm);
                        int token_to_identify = find_token_by_name(buffer_str);

                        string origin = SERVER;

                        if(isAdm && token_to_identify != -1) {
                            string user_ip = ips[token_to_identify];
                            response = origin + "\033[1;314m" + user_ip + "\033[0m"; 
                        }

                        // induvíduo nao eh adm
                        else if(!isAdm) response = origin + "\033[1;31mPERMISSION DENIED,YOU ARE NOT THE ADM!\033[0m";             

                        // nao foi encontrado o usuário a ser removido 
                        else if(token_to_identify == -1) response = origin + "\033[1;33mUser not found in your channel! \033[0m";   
                        
                        char *tmp = str_to_charA(response, 1 + response.length());
                        send(sd, tmp, response.size() + 1, 0);
                        free(tmp);  
                    } 

                    // mensagem completa
                    else if(op_code == FLAG_COMPLETE[0]){
                        //procura e imprime o nome do usuário
                        print_name(clear_name(username), "green");
                        cache[token] += buffer_str;
                        cout << cache[token] << endl;
                        cache[token] = CLIENT + users[token].first + cache[token];
                        char *tmp = str_to_charA(cache[token], 1 + size_username + size_message);
                        send_to_channel(client_socket, max_clients, tmp, 1 + size_username + size_message,find_channel(token), token);
                        // send_to_all(client_socket, max_clients, tmp, 1 + size_username + size_message);
                        free(tmp);
                        cache[token] = "";
                    }

                    // alguém se desconectou 
                    else if(op_code == FLAG_EXIT[0]){
                        //procura e imprime o nome do usuário
                        print_name(clear_name(username), "green");
                        string origin = SERVER;
                        print_text("/quit", "yellow", true);
                        //procura por suas informações e as imprime  
                        getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

                        response = origin + "\033[1;31m" + username.substr(0,username.find('\0')) + " left the chat" + "\033[0m"; 
                        char *tmp = str_to_charA(response, 1 + size_message);
                        string userChannel = find_channel(token);

                        if(userChannel.length() > 0) send_to_channel(client_socket, max_clients, tmp, 1 + size_message, userChannel, token, true);
                        else send(sd, tmp, response.size() + 1, 0);

                        remove_user_from_channel(token,find_channel(token));
                           
                        print_name(NAME,"red");
                        cout << "Host disconnected, ip " << inet_ntoa(address.sin_addr) << ", port " << ntohs(address.sin_port) << '\n';
                        //envia mensagem de fim de conexão para os clientes

                        free(tmp);
                        //Fecha o socket e marca como 0 na lista para reutilização  
                        close(sd);
                        //Remove as informações relacionadas ao usuário
                        int remove_token;
                        for(it = users.begin(); it != users.end(); it++) {
                            if(it->second.second == client_socket[i]) {
                                remove_token = it->first;
                                ips.erase(remove_token);
                                users.erase(it);
                                break;
                            }
                        } 
                        client_socket[i] = 0;
                    }

                    // comando para sair de um canal
                    else if(op_code == FLAG_LEAVE[0]){
                        //procura e imprime o nome do usuário
                        print_name(clear_name(username), "green");
                        string currentChannel = find_channel(token);
                        print_text("/part " + buffer_str,"yellow",true);

                        //o canal especificado na requisição é o mesmo no qual o cliente se encontra no momento
                        if(currentChannel.compare(buffer_str) == 0) {
                            string origin = SERVER;
                            string disconnected_response = DISCONNECTED;

                            response = origin + "\033[1;33m" + clear_name(username) + " left " + currentChannel + "\033[0m";
                            char *tmp = str_to_charA(response, 1 + response.length());
                            send_to_channel(client_socket, max_clients, tmp, 1 + size_username + size_message,currentChannel, token, true);

                            char *disconnected = str_to_charA(disconnected_response, disconnected_response.length());
                            send(users[token].second, disconnected, disconnected_response.length(), 0);

                            remove_user_from_channel(token,currentChannel);
                            free(tmp);
                            free(disconnected);
                        }
                    }

                    // comando "/ping"
                    else if(op_code == FLAG_PING[0]){
                        //procura e imprime o nome do usuário
                        print_name(clear_name(username), "green");
                        string origin = SERVER;
                        print_text("/ping","yellow",true);
                        response = origin + "\033[1;314mP O N G\033[0m"; 
                        char *tmp = str_to_charA(response, 1 + response.length());
                        send(sd, tmp, response.size() + 1, 0);
                        free(tmp);
                    }
                    
                    response.clear();
                }
            } 
        }
    }

    return 0;
}