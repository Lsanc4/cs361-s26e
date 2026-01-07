void changecase(char* text) {
    while(*text) {
        if(*text >='a' && *text <= 'z') {
            *text -= 'a'-'A';
        }
        text++;
    }
} 