Trabalho feito por :
 - Luis Felipe Ribeiro Chaves   - 10801221
 - Henrique Tadashi Tarzia      - 10692210
 - Victor Akihito Kamada Tomita - 10692082 

[@] Servidor:
Para executar o servidor do trabalho, abra a pasta trab-3 e insira o comando <code>g++ chat.cpp server.cpp -o server.out && ./server.out</code> no terminal.
Caso queira permitir a conexão com usuários de outras máquinas, modifique a variável "SERVER_IP" do aquivo "chat.h" com o endereço de ip externo da sua máquina.
Também será necessário especificar um servidor virtual no ponto de acesso. Para o teste do projeto, foi utilizado um modelo da marca TP-Link, empregando os seguintes passos:

1) Insira o endereço "192.168.0.1" em uma aba do seu navegador.
2) Será exibido um formulário com dois campos para preenchimento. Escreva "admin" para ambos.
3) Você será redirecionado para a interface do ponto de acesso. Selecione a aba "Redirecionamento de Portas".
4) Clique no botão "Adicionar".
5) Será fornecido um formulário para especificação do servidor. Insira os dados abaixo: 
</br>5.1) Porta de Serviço: 8888
</br>5.2) Porta Interna: 8888
</br>5.3) Endereço IP: endereço ip local da máquina
</br>5.4) Protocolo: TCP
</br>5.5) Estado: Habilitado
6) Finalize o processo ao clicar em "Salvar"

Obs: o método descrito acima varia de acordo com o ponto de acesso 

[@] Cliente:
Para criar uma instância de cliente, execute o comando <code>g++ chat.cpp client.cpp -pthread -o client.out && ./client.out</code> em um terminal presente na pasta trab-3.
No menu inicial, insira <code>/connect</code> para estabelecer conexão com o servidor. Feito isso, será requisitado um nickname para identificação do usuário. Caso queira 
visualizar os comandos disponíveis dentro de um canal, insira <code>/commands</code> enquanto não se encontra conectado a um canal. 

Nossa página no github : https://github.com/Vakihito/trabalho-redes 
Obs : a pasta referente a este trabalho é a pasta trab-3
