all:
	gcc -o client.bin client.c 
	gcc -o server.bin server.c -pthread
clean:
	rm -f client.bin server.bin
client: all
	./client.bin 127.0.0.1 2910