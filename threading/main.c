#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "miniz.h"
#include "queue.h"

char input[1024];
char fileContent[1000];
char tmpString[1024];
char tmpStringtok[1000];
char fileEnd[1000];
char* tok;
pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
int readFinished=0;

typedef struct job{
    char pfad[1000];
    char inhalt[1000];  
} Job;

static Queue queue;

int isRegularFile(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}


void* readThread(void *arg){
    struct RetValues *ret = NULL;
    DIR *folder;
    struct dirent *entry;
    FILE *fptr;
    Job *job;
    tok = malloc(sizeof(tok));


    folder = opendir(arg);
    if(folder == NULL)
    {
        perror("Unable to read directory");
        return (void *)ret;
    }

    while( (entry=readdir(folder)) )
    {
        strcpy(tmpString, arg);
        strcat(tmpString, "/");
        strcat(tmpString, entry->d_name);

        if(isRegularFile(tmpString)){
            tok="";
            strcpy(tmpStringtok, tmpString);
            strtok(tmpStringtok, ".");
            while (tok != NULL){
                strcpy(fileEnd, tok);
                tok = strtok(NULL, ".");
            }

            if (strcmp(fileEnd, "compr")){
                printf("File: %s\n", entry->d_name);
                fptr = fopen(tmpString, "r");
                fgets(fileContent, 1000, fptr);
                printf("Inhalt: %s \n", fileContent);
                fclose(fptr);
                
                Job *a = (Job *) malloc(sizeof(Job));
                strcpy(a->pfad, tmpString);
                strcpy(a->inhalt, fileContent);
                //TODO queue sperren
                pthread_mutex_lock(&mutex_queue);
                queue_insert(queue, a);
                pthread_mutex_unlock(&mutex_queue);
            }
        }
        sleep(1);
    }
    readFinished=1;
    closedir(folder);
    return (void *)ret;
}

void* comprThread(void *arg){
    printf("Thread %d gestartet! \n", *(int *)arg);
    while(1){
        struct RetValues *ret = NULL;
        FILE *fptr;
        char tmpPfad[1000]="";

        //TODO QUEUE SPERREN
        pthread_mutex_lock(&mutex_queue);
        Job *job = queue_head(queue);
        if (job==NULL){
            if (&readFinished){
                pthread_mutex_unlock(&mutex_queue);
                return (void *)ret;
            }
        }
        queue_delete(queue);
        pthread_mutex_unlock(&mutex_queue);
        
        Result *result = compress_string(job->inhalt);
        strcat(tmpPfad,job->pfad);
        fptr = fopen(strcat(tmpPfad,".compr"), "w");

        fprintf(fptr, result->data);
        fclose(fptr);

        free(result);
        sleep(3);
    }
}

int main(int argc, char* argv[]){
    time_t anfangszeit, endzeit;
    struct RetValues *ret = NULL;
    pthread_t threadLeser;
    int status = 0;
    int statusarr[100];
    int anzThreads=3;
    pthread_t comprThreads[anzThreads];
    int threadNums[anzThreads];

    time(&anfangszeit);
    //input
    printf("Ordnerpfad: ");
    fgets(input, 1024, stdin);
    input[strlen(input)-1] = '\0';

    //Leserthread
    queue = queue_create();
    status = pthread_create (&threadLeser,
        NULL,
        readThread,
        &input);
    if (status!=0){
        printf("Thread konnte nicht gestartet werden");
        return 1; 
    }

    for(int i=0; i<anzThreads; i++){
        threadNums[i] = i;
        statusarr[i] = pthread_create (&comprThreads[i],
        NULL,
        comprThread,
        &threadNums[i]);
        if (statusarr[i]!=0){
            printf("Thread konnte nicht gestartet werden");
            return 1; 
        }
    }

    pthread_join(threadLeser, NULL);
    //Warte bis alle komprimierungs Threads abgeschlossen sind 
    for(int i=0; i<anzThreads; i++){
        pthread_join(comprThreads[i], NULL);
    }
    time(&endzeit);
    printf("Laufzeit: %f \n", difftime(endzeit, anfangszeit));

    pthread_mutex_destroy(&mutex_queue);
    
    return 0;
}