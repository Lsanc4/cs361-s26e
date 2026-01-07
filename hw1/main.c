#include<stdio.h>
#include"casing.h"

char 
 //__attribute__((visibility("default"))) 
 greeting[]="Hello World!";

int main() {
    printf("Original: %s\n",greeting);
    uppercase(greeting); 
    printf("Uppercased: %s\n",greeting);
    lowercase();
    printf("Lowercased: %s\n",greeting);
    changecase(greeting);
    printf("Changecased: %s\n",greeting);
}
