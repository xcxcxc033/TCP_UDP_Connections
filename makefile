.PHONY:servers

all:
	gcc -o myservers servers.c -lsocket -lnsl -lresolv
	gcc -o client Client.c -lsocket -lnsl -lresolv

servers:
	./myservers 24145 21145 22145 23145 25145




