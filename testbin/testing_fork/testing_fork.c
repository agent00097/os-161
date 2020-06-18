#include<unistd.h>
#include<stdio.h>

int main(int args, char *argv[]){
    int pid = fork();
    int status;
    if(pid == 0){
        printf("I am child with pid = %d\n", getpid());
        return 0;
    }
    else{
        waitpid(pid, &status, 0);
        printf("I am parent with pid = %d\n", getpid());
        printf("Status = %d\n", status);
    }
    return 0;
}