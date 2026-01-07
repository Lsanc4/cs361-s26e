void changecase(char* text) {
    while(*text) {  
        if(((long long)text)&1==1 && *text >='a' && *text <= 'z') {
            *text -= 'a'-'A';
        }
        else if(((long long)text)&1==0 && *text >='A' && *text <= 'Z') {
            *text += 'a'-'A';
        }
        text++;
    }
} 