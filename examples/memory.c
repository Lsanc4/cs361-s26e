#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int babe = 0xCAFEBABE;
long hungry = 0xFEEDFACEBEEFF00D;

char empty[1000000];
char *letters = "CAFEBABES";

int main() {	
	char *hungryptr = (char*)&hungry;
	printf("The string is %s, pointer %p %p the int is %X the long is %lX hungryptr[0]=%02x\n",letters,letters,&letters,babe,hungry,hungryptr[0]);

	// void* chunk = malloc(100);
	// printf("Chunk is at %p, points at %p, has string '%s' after strncpy\n",&chunk,chunk,(char*)chunk);
	// char* charchunk = (char*)chunk;

	// printf("Charchunk is at %p, points at %p, has string '%s' before strncpy\n",&charchunk,charchunk,charchunk);
	// strncpy(charchunk,letters,100);

	// printf("Charchunk is at %p, points at %p, has string '%s' after strncpy\n",&charchunk,charchunk,charchunk);
	// printf("Chunk is at %p, points at %p, has string '%s' after strncpy\n",&chunk,chunk,(char*)chunk);

	// free(charchunk);
	// printf("Charchunk is at %p, points at %p, has string '%s' after free\n",&charchunk,charchunk,charchunk);
}
