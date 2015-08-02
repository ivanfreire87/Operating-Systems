#include <stdio.h>
#include "proj.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
 
#define TEMPMSG                     1000
#define SIZE_OF_FILE_NAME           13
#define NUMBER_OF_FILES             5
 
char *input;
char tmp[TEMPMSG];
int first = 0;
 
 
int main(int argc, char *argv[]){
    int fds[2], wait_pid, reader_pid, writer_pid;
    char *input, arg[13];
    input = (char*) malloc(SIZE_OF_FILE_NAME*NUMBER_OF_FILES);
     
    if(pipe(fds) < 0)
        exit(-1);
     
    writer_pid = fork();
    if(writer_pid == 0) 
        execl("writer","writer",(char *)0);     
    else if(writer_pid < 0){
        perror("Writer fork failed");
        abort();
    }
    else{
        reader_pid = fork();
        if(reader_pid == 0){
            close(0);
            dup(fds[0]);
            close(fds[0]);
            close(fds[1]);
            execl("reader","reader",(char *)0); 
        }   
        else if(reader_pid < 0){
            perror("Reader fork failed");
            abort();
        }
        else{
            sleep(2);
             
            while(1){
                read(0, input, 100);
                input = strtok(input,"\n");
                 
                if(!strcmp(input,"il"))
                    kill(writer_pid,SIGUSR1);
                else if(!strcmp(input,"ie"))
                    kill(writer_pid,SIGUSR2);
                else if(!strcmp(input,"sair")) 
                    kill(writer_pid,SIGTSTP);
                else{
                    if(first == 0){
                    close(1);
                    dup(fds[1]);
                    close(fds[0]);
                    close(fds[1]);
                    first = 1;
                    write(1,input,SIZE_OF_FILE_NAME*NUMBER_OF_FILES);   }
                    else
                        write(1,input,SIZE_OF_FILE_NAME*NUMBER_OF_FILES);
                }               
            }
             
            wait_pid = wait();
        }   
    }
         
 
        /*while(1){
            close(1);
            dup(fds[1]);
            close(fds[0]);
            close(fds[1]);
            read(0, input, SIZE_OF_FILE_NAME*NUMBER_OF_FILES);  
            write(1,input,sizeof(input));           
        }*/
         
     
     
    return 0;
     
}
