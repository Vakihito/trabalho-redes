server_command 	= $(shell g++ chat.o server.cpp -o server.o)
client_command 	= $(shell g++ chat.o client.cpp -pthread -o client.o)
chat_command 	= $(shell g++ -c -o chat.o chat.cpp)
server:
	$(chat_command) $(server_command) ./server.o
client:
	$(chat_command) $(client_command) ./client.o
run_c:
	./client.o
run_s:
	./server.o
clear:
	tput reset
server_v :
	$(chat_command) $(server_command) valgrind ./server.o
client_v :
	$(chat_command) $(client_command) valgrind ./client.o