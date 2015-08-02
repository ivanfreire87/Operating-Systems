#include <stdio.h>
#include "proj.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
 
#define NUMBER_OF_WRITER_THREADS    10
#define TIMES_TO_WRITE_STRING       1024
 
int writing = 1;
int locking = 1;
int misspelling = 0;
 
void new_handler(int signal) {
    switch(signal){
        case SIGUSR1:   locking ? (locking = 0) : (locking = 1);
                        break;
        case SIGUSR2:   misspelling ? (misspelling = 0) : (misspelling = 1);
                        break;
        case SIGTSTP:   writing = 0;
                        break;
 
    }
}
 
void *writer_thread(void *args){
    int fd, i, randomFileNumber, randomStringNumber;
    int cicles = 0;
    char *toWriteToFile, *toWriteToFileWithError;
    int locking_status = 3;
 
    toWriteToFile = (char*) malloc((sizeof(char)*10)*TIMES_TO_WRITE_STRING+1);
    toWriteToFileWithError = (char*) malloc((sizeof(char)*10)*TIMES_TO_WRITE_STRING+1);
     
    while(writing){
         
        /*gera numero de ficheiro aleatorio*/
        srand(time(NULL)+cicles);
        randomFileNumber = rand() % 5;
         
        /*gera numero de cadeia aleatorio*/
        srand(time(NULL)+cicles);
        randomStringNumber = rand() % 9;
        fd = open(allFiles[randomFileNumber], O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
         
        if(locking)
            locking_status = flock(fd, LOCK_EX);
         
         
        if(locking_status == -1)
            perror("Erro no file lock");
        else{
            if(fd != 0){
                /*limpa a string 'toWriteToFile'*/
                strcpy(toWriteToFile,"");
                strcpy(toWriteToFileWithError,"");  
                /*preenche a string 'toWriteToFile' com a cadeia de caracteres seleccionada TIMES_TO_WRITE_STRING vezes*/
                for(i = 0; i < TIMES_TO_WRITE_STRING; i++){
                    if(!misspelling)
                        strcat(toWriteToFile,allStrings[randomStringNumber]);
                    else{
                        if(i % 2 == 0)
                            strcat(toWriteToFileWithError,allStrings[randomStringNumber]);
                        else{
                            strcat(toWriteToFileWithError,allStrings[randomStringNumber]);
                            toWriteToFileWithError[i*10] = toWriteToFileWithError[1]+1;
 
                        }
                             
                    }
                }
                if(!misspelling)    
                    write(fd,toWriteToFile,strnlen(toWriteToFile,10*TIMES_TO_WRITE_STRING));
                else{
                    write(fd,toWriteToFileWithError,strnlen(toWriteToFileWithError,10*TIMES_TO_WRITE_STRING));}
                 
            }
             
            if(flock(fd, LOCK_UN) == -1)
                perror("Erro no file unlock");
                 
            close(fd);
         
            cicles++;
        }
    }
     
    free(toWriteToFile);
     
    return NULL;        
}
 
int main(int argc, char *argv[]){
    int i;
    pthread_t writer_threads[NUMBER_OF_WRITER_THREADS];      
    struct sigaction new_action;
 
    new_action.sa_handler = new_handler;
    sigemptyset (&new_action.sa_mask);
    sigaddset(&new_action.sa_mask, SIGUSR1);
    sigaddset(&new_action.sa_mask, SIGUSR2);
    sigaddset(&new_action.sa_mask, SIGTSTP);
    new_action.sa_flags = 0;
    sigaction(SIGUSR1, &new_action, NULL);
    sigaction(SIGUSR2, &new_action, NULL);
    sigaction(SIGTSTP, &new_action, NULL);
         
    for(i=0; i < NUMBER_OF_WRITER_THREADS; i++){         
        if(pthread_create(&writer_threads[i], NULL, writer_thread, NULL)) {
            printf("Error creating thread %d\n",i);         
            return -1;
        }       
    }
     
    for(i=0; i < NUMBER_OF_WRITER_THREADS; i++){
        if(pthread_join(writer_threads[i], NULL)) {
            printf("Error joining thread %d\n",i);
            return;         
        }
    }
     
 
    return 0;   
}
