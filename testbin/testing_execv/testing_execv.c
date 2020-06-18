#include<unistd.h>
#include<stdio.h>

int main(int argc, char *argv[]){
    // char *args[3];
    // args[0] = "forktest";
    // args[1] = "hello";
    // args[2] = NULL; 
    char *args[1] = {"true", NULL};
    
    int pid = fork();
    if(pid == 0) {
        execv("bin/true", args);
    }
    else {
        return 0;
    }
}

