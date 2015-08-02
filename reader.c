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
#include <semaphore.h>
 
#define NUMBER_OF_THREADS           5
#define TIMES_TO_READ_STRING        1024
#define SIZE_OF_FILE_NAME           13
#define NUMBER_OF_FILES             5
 
char buffer[NUMBER_OF_FILES][SIZE_OF_FILE_NAME];
int indexIn = 0, indexOut = 0;
sem_t bufferOutSem, bufferInSem;
 
typedef struct thread_arguments{
    pthread_t thread;
    int result;
}thread_arguments;
 
void *reader_thread(void *args){
    int fd, i, var, out_sem_value;
    char* fileName;
    char *lineFromFile, *toReadFromFile;
    thread_arguments* arguments;
     
    arguments = (thread_arguments*) args;
    fileName = (char*) malloc(13);
    toReadFromFile = (char*) malloc(10);
    lineFromFile = (char*) malloc(10);
     
    while(1){
        sem_getvalue(&bufferOutSem,&out_sem_value);
        if(out_sem_value == 0)
            printf("Child thread %ld is waiting...\n",pthread_self());
             
        sem_wait(&bufferOutSem);    
        strcpy(fileName,buffer[indexOut]);
        strcpy(buffer[indexOut],"");
        indexOut = (indexOut + 1) % NUMBER_OF_FILES;    
        sem_post(&bufferInSem);
         
        fd = open(fileName, O_RDONLY);
        if(flock(fd, LOCK_SH) == -1)
            perror("Error on file lock");
         
        else{
            if(fd != 0){
                /*guarda a cadeia de cadeia de caracteres da primeira linha para usar como comparador*/
                read(fd, lineFromFile, 10);
                /*percorre todas as linhas do ficheiro e compara a cadeia de carateres de cada linha com 'lineFromFile'*/
                for(i=0; i < TIMES_TO_READ_STRING - 1; i++){     
                    var = read(fd, toReadFromFile, 10);
                    if(strncmp(toReadFromFile,lineFromFile,10) != 0 || var < 10){            
                        /*uma das linhas tem uma cadeia de caracteres diferente das restantes*/
                        arguments->result = -1;
                        break;
                    }                       
                }
                if(read(fd, toReadFromFile, 10) != 0){
                    /*numero de linhas inferior ou superior a TIMES_TO_READ_STRING*/
                    arguments->result = -1;
                }
                 
                if(flock(fd, LOCK_UN) == -1)
                    perror("Erro no file unlock");  
                                     
                close(fd);      
            }
             
            if(arguments->result > -1)
                arguments->result = 0;
 
            printf("Child thread %ld read file %s returning %d\n",pthread_self(),fileName,arguments->result);
            arguments->result = 1;
            sleep(1);
        }
    }
 
    free(toReadFromFile);
    free(lineFromFile); 
     
     
     
}
 
void *mainThread(void *args){
    int i = 0,j = 0,k = 0, in_sem_value;
    char *fileName, *input, *token;
    thread_arguments* arguments = malloc(sizeof(thread_arguments)*NUMBER_OF_THREADS);
    input = (char*) malloc(SIZE_OF_FILE_NAME*NUMBER_OF_FILES);
     
     
     
    for(i=0; i < NUMBER_OF_THREADS; i++){                        
        if(pthread_create(&(arguments[i].thread), NULL, reader_thread, (void*) &arguments[i])) {
            printf("Error creating thread %d\n",i);
             
            return;
        }       
    }   
     
    while(1){
        read(0, input, SIZE_OF_FILE_NAME*NUMBER_OF_FILES);  
        input = strtok(input, "\n");    
        token = strtok(input, " \n");
         
        while( token != NULL ){
            sem_getvalue(&bufferInSem,&in_sem_value);
            if(in_sem_value == 0)
                printf("Parent thread %ld is waiting...\n",pthread_self());
            sem_wait(&bufferInSem);
            strcpy(buffer[indexIn],token);
            indexIn = (indexIn + 1) % NUMBER_OF_FILES;
            sem_post(&bufferOutSem);              
            token = strtok(NULL, " \n");
                         
        }
                 
    }
     
    /*for(i=0; i < NUMBER_OF_THREADS; i++){
        if(pthread_join(arguments[i].thread, (void*) &arguments[i]) != 0) {
            printf("Error joining thread %d\n",i);
            return;         
        }
             
        else{
            printf("%d\n",arguments[i].result);
        }   
    }*/
     
 
     
}
 
 
int main(int argc, char *argv[]){
 
    pthread_t thread;
         
    sem_init(&bufferOutSem,0,0);
    sem_init(&bufferInSem,0,NUMBER_OF_FILES);   
     
    //timeinit
    if(pthread_create(&thread, NULL, mainThread, (void*) &pipe)) {
        printf("Error creating main thread\n");
             
        return 1;
    }
         
    if(pthread_join(thread, NULL) != 0) {
        printf("Error joining main thread\n");
        return 1;           
    }   
         
    //time end
     
    return 0;   
}
