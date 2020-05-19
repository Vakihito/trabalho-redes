server_command = $(shell g++ server2.cpp -pthread -o server2.out)
client_command = $(shell g++ client2.cpp -pthread -o client2.out)
server:
	$(server_command) ./server2.out
client:
	$(client_command) ./client2.out
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