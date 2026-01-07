extern char greeting[];
void changecase() {
    char* text = greeting;
    while(*text) {
        if(*text>='A' && *text <= 'Z') {
            *text += 'a'-'A';
        }
        text++;
    }
} 