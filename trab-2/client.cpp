// Client side C/C++ program to demonstrate Socket programming  
#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <errno.h>  
#include <string> 
#define PORT 8888 

using namespace std;

char *str_to_charA(string str, int n) {
    char *char_array = new char[n];
    for (unsigned int i = 0; i < n and i < str.length(); i++)
        char_array[i] = str[i];
    char_array[n < str.length() ? n : str.length()] = '\0';
    return char_array;
}
   
int main(int argc, char const *argv[]) { 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    string message; 
    string buffer; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)  { 
        perror("Socket creation error\n");
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) { 
        perror("Invalid or unsupported address\n");
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
        perror("Connection Failed\n");
        return -1; 
    }

    while(true) {
        cin >> message;
        char *tmp_message = str_to_charA(message, message.length());
        send(sock, tmp_message, message.length(), 0 );
        free(tmp_message);
        char *tmp_buffer = str_to_charA(buffer, 1024);
        valread = read( sock, tmp_buffer, 1024); 
        cout << buffer << '\n';
        free(tmp_buffer);
    }
    return 0; 
} 