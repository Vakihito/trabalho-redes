#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#define PORT 8080
#define LEN_STRING 4100
using namespace std;


int main(int argc, char const *argv[])
{
    /* create a socket  */

    int socket_listen = socket(AF_INET, SOCK_STREAM, 0);
    // checking if it created the right way
    if (socket_listen == -1)
    {
        cerr << "Não consegui criar a socket" << endl;
        return -1;
    }

    /* ligando o socket com um ip */

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    // converte
    hint.sin_port = htons(54000);
    // conectando com um  ip qualquer
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); // convertendo um número ( network address )
    // socket_listen - porta que está escutando (número da porta)
    // no formato AF_INET
    // conectando ao hint 
    if (bind(socket_listen,(sockaddr *) &hint, sizeof(hint)) == -1)
    {
        cerr << "Não consegui conectar a uma porta IP" << endl;
        return -2;
    }
    
    /* mark the socket for listening in */

    if (listen(socket_listen, SOMAXCONN) == -1)
    {
        cerr << "Não consegui receber/escutar a mensagem" << endl;
        return -3;
    }

    /* accept a call */

    // mostrando certas informações do cliente
    sockaddr_in  client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];
    int clientSocket = accept(socket_listen, (sockaddr*) &client, &clientSize);
    
    if (clientSocket == -1)
    {
        cerr << "Não consegui conectar com o cliente" << endl;
        return -4;
    }

    /* close the listening socket */

    close(socket_listen);

    // limpando o que havia no host e no svc
    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);
    
    // pegando dados do cliente, (no caso o nome)
    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST,svc, NI_MAXSERV,0);

    // verificando se deu certo, caso não, faz manual
    if (result)
    {
        cout << host << " conectado ao : "<< svc << endl;
    }
    else{
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on : " << ntohs(client.sin_port) << endl;
    }

    /* while reciving display message, echo, message */
    
    char buf[LEN_STRING];
    while (true)
    {
        // limpa o buffer
        memset(buf, 0, LEN_STRING);
        // espera pela mensagem
        int bytesRecv = recv(clientSocket, buf, LEN_STRING, 0);
        if (bytesRecv == -1)  
        {
            cerr << " houve um problema na conexão" << endl;
            break;
        }
        if (bytesRecv == 0)
        {
            cout << " O cliente desconectou" << endl;
            break;
        }
        cout << " Foi Recebido : " << string(buf, 0 , bytesRecv) << endl;
        send(clientSocket, buf, bytesRecv + 1, 0);    
    }
    

    /* close socket */

    close(clientSocket);

    return 0;
}

