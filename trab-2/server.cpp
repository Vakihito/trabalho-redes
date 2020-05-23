//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux  
#include <iostream>
#include <string>//strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>//close  
#include <arpa/inet.h> //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  

#define PORT 8888  

using namespace std;

char *str_to_charA(string str, int n) {
    char *char_array = new char[n];
    for (unsigned int i = 0; i < n and i < str.length(); i++)
        char_array[i] = str[i];
    char_array[n < str.length() ? n : str.length()] = '\0';
    return char_array;
}

int main(int argc, char *argv[]) {
    int opt = true;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    string buffer;

    //set of socket descriptors  
    fd_set readfds;

    //a message  
    string message;

    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)
        client_socket[i] = 0;

    //create a master socket  
    if((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created  
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    cout << "Listener on port " << PORT << '\n';

    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    cout << "Waiting for connections ...\n";

    while(true) {
        //clear the socket set  
        FD_ZERO(&readfds);

        //add master socket to set  
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++) {
            //socket descriptor  
            sd = client_socket[i];

        //if valid socket descriptor then add to read list  
        if(sd > 0)
            FD_SET( sd, &readfds);

        //highest file descriptor number, need it for the select function  
        if(sd > max_sd)
            max_sd = sd;
        }

        //wait for an activity on one of the sockets, timeout is NULL,
        //so wait indefinitely  
        activity = select( max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno!=EINTR))
        cout << "select error\n";

        //If something happened on the master socket,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands  
            cout << "New connection, socket fd is " << new_socket << ", ip is : " << inet_ntoa(address.sin_addr) << ", port : " << ntohs(address.sin_port) << '\n';

            char *tmp_message = str_to_charA(message, message.length());
            //send new connection greeting message  
            if (send(new_socket, tmp_message, message.length(), 0) != message.length()) 
                perror("send");

            free(tmp_message);

            cout << "Welcome message sent successfully\n";

            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++) {
                //if position is empty  
                if( client_socket[i] == 0 ) {
                    client_socket[i] = new_socket;
                    cout << "Adding to list of sockets as " << i << '\n';
                    break;
                }
            }
        }

        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];

            if (FD_ISSET( sd, &readfds)) {
                //Check if it was for closing, and also read the  
                //incoming message 
                char *tmp_buffer = str_to_charA(buffer, 1024);
                if ((valread = read( sd, tmp_buffer, 1024)) == 0) {
                    //Somebody disconnected, get his details and print  
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    cout << "Host disconnected, ip " << inet_ntoa(address.sin_addr) << ", port " << ntohs(address.sin_port) << '\n';

                    //Close the socket and mark as 0 in list for reuse  
                    close(sd);
                    client_socket[i] = 0;
                }

                //Echo back the message that came in  
                else {
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    tmp_buffer[valread] = '\0';
                    send(sd, tmp_buffer, valread, 0 );
                }
            }
        }
    }

    return 0;
}