#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char* createPacket(int numNeighbors, int numFiles){
	char* buf;
	// at most each objectName is 9 characters
	buf = malloc(20*sizeof(char)+numNeighbors*sizeof(int)+numFiles*9);
	return buf;
}

char* addInt(int val, char* buf){
	memcpy(buf, &val, sizeof(val));
	return buf+sizeof(val);
}
char* addChar(char val, char* buf){
	memcpy(buf, &val, sizeof(val));
	return buf+sizeof(val);
}
char* addString(char* str, char* buf){
	memcpy(buf, str, 9);
	return buf+9;
}

void printBuf(char* buf, int len){
	
	int i;
	for( i = 0 ; i < len ; i++){
		printf("%c", buf[i]);
	}
	printf("\nend\n");
}


int main(){
	
	char* buf = createPacket(3, 5);
	
	char* ptr = buf;
	
	ptr = addChar('1', ptr);
	// there is only one byte for this filed, really?
	ptr = addChar('9',ptr);
	ptr = addChar('1', ptr);
	ptr = addInt(50, ptr);
	ptr = addInt(0, ptr);
	ptr = addInt(3, ptr);
	ptr = addInt(5, ptr);
	
	printBuf(buf, 20);
}