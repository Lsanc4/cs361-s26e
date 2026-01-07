extern char greeting[];
void lowercase() {
    char* text = greeting;
    while(*text) {
        if(*text>='A' && *text <= 'Z') {
            *text += 'a'-'A';
        }
        text++;
    }
} 