routed: routDaemon.o linkedList.o shortestPath.o packet.o
	gcc -o routed routDaemon.o linkedList.o packet.o -g -Wall
routDaemon.o: routDaemon.c linkedList.h
	gcc -o routDaemon.o routDaemon.c -c -g -Wall
linkedList.o: linkedList.c linkedList.h
	gcc -o linkedList.o linkedList.c -c -g -Wall
shortestPath.o: shortestPath.c linkedList.h
	gcc -o shortestPath.o shortestPath.c -c -g -Wall
packet.o: packet.c linkedList.h
	gcc -o packet.o packet.c -c -g -Wall
clean:
	@rm routed routDaemon.o linkedList.o shortestPath.o packet.o
