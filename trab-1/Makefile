server_command = $(shell g++ server2.cpp -pthread -o server.out)
client_command = $(shell g++ client2.cpp -pthread -o client.out)
server:
	$(server_command) ./server.out
client:
	$(client_command) ./client.out
run_c:
	./client.out
run_s:
	./server.out
clear:
	tput reset
server_v :
	$(server_command) valgrind ./server.out
client_v :
	$(client_command) valgrind ./client.out