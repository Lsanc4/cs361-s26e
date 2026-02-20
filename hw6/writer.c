int main() {
    int count=361;
    long header=count*3;
    int values[3]={3,6,1};
    write(1,&header,8);
    while(count--) {
        write(1,values,12);
    }
}