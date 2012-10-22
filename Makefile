routed: routDaemon.o linkedList.o packet.o algo.o
	gcc -o routed routDaemon.o linkedList.o packet.o algo.o -g -Wall
routDaemon.o: routDaemon.c linkedList.h
	gcc -o routDaemon.o routDaemon.c -c -g -Wall
linkedList.o: linkedList.c linkedList.h
	gcc -o linkedList.o linkedList.c -c -g -Wall
packet.o: packet.c linkedList.h
	gcc -o packet.o packet.c -c -g -Wall
algo.o: algo.c algo.h linkedList.h
	gcc -o algo.o algo.c -c -g -Wall
clean:
	@rm routed routDaemon.o linkedList.o packet.o algo.o
