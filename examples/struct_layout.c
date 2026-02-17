#include <stdalign.h>
#include <stdio.h>

struct innerstruct {
    int a;
    char b;
};

struct outerstruct {
    char c;
    struct innerstruct inner;    
};

int main(int argc, char **argv) {
    struct outerstruct o;
    printf("Size of outerstruct: %ld, size of innerstruct %ld\n", sizeof(o), sizeof(o.inner));
    printf("Align of outerstruct: %ld, align of innerstruct %ld\n", alignof(o), alignof(o.inner));
}