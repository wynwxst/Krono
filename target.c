#include <stdio.h>

void somefunc(char* prompt){
    printf("Maybe not...: %s", prompt);
}

int main(void) {
    printf("Hello, World!\n");
    somefunc("ok");
    return 0;
}

