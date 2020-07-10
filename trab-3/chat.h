#ifndef CHAT_H
#define CHAT_H

using namespace std;

//códigos presentes na requisição para identificar o tipo de operação solicitada
#define FLAG_EXIT "e"               //término de conexão com o servidor
#define FLAG_INCOMPLETE "i"         //mensagem incompleta
#define FLAG_CHANNEL "#"            //channel command
#define FLAG_COMPLETE "c"           //mensagem completa
#define FLAG_PING "p"               //instrução "ping-pong"
#define FLAG_KICK "k"               //instrução de kick de um usuário
#define FLAG_CHANGE_USERNAME "u"    //trocar username

//códigos presentes na resposta do servidor para os usuários
#define INVALID_USERNAME 'r'  //nome de usuário repetido
#define VALID_USERNAME 'a'    //nome de usuário aceito
#define SERVER "s"            //mensagem enviada pelo servidor
#define CLIENT "c"            //mensagem enviada por um cliente         

//constantes utilizadas para a construção e processamento das requisições dos usuários
#define PORT 8888            // número da porta de comunicação
#define QUIT -9999           // código de encerramento/falha na comunicação do cliente com o servidor
#define max_token 999999     // maior valor de token
#define size_message 100     // comprimento máximo de uma mensagem enviada pelo usuário
#define size_token 6         // número de caracteres do token do usuário
#define size_username 20     // número de caracteres do nome do usuário

/*
    Imprime nome no formato "[name]: " com a cor especificada
    >> name: nome a ser impresso
    >> color: cor da impressão 
*/
void print_name(string name, string color);

/*
    Imprime texto colorido
    >> text: conteúdo da impressão
    >> color: cor da impressão
    >> new_line: indica quebra de linha ao término da mensagem
*/
void print_text(string text, string color, bool new_line);

/*
    Converte vetor de caracteres para string
    >> str: ponteiro para a string que receberá o conteúdo do vetor
    >> char_array: vetor de entrada
    >> n: número de caracteres da conversão
*/
void concat_charA_to_str(string *str, char *char_array, unsigned int n);

/*
    Converte string para vetor de caracteres 
    >> char_array: vetor de alocação dos dados
    >> str: string de entrada
    >> n: número de caracteres da conversão
*/
void stoca(char *char_array, string str, unsigned int n);

/*
    Converte string para vetor de caracteres
    >> str: string de entrada
    >> n: número de caracteres da conversão
    << char*: vetor de caracteres resultante
*/
char *str_to_charA(string str, int n);

#endif