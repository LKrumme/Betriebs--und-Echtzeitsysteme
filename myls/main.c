#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>


char fileContent[1000];
char tmpString[1024];
char tmpStringtok[1000];
char fileEnd[1000];
char* tok;
int readFinished=0;
int show_hidden=0, listing_format=0;
char opt;
struct stat path_stat;
char rwx_string[1000] = {"rwxrwxrwx"};
char months[1000][1000] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};
struct dirent *entry;
int isExecutable=0;

typedef struct job{
    char pfad[1000];
    char inhalt[1000];  
} Job;

int printListingFormat(const char *path)
{

    //Owner S_IRWXU
    //Group S_IRWXG
    //Other S_IRWXO
    struct stat path_stat;
    int tmp_st_mode; 
    lstat(path, &path_stat);
    tmp_st_mode = path_stat.st_mode;

    //drwxrwxrwx
    if (S_ISDIR(path_stat.st_mode)){
        printf("d");
    }
    else{
        printf("-");
    }
    tmp_st_mode = tmp_st_mode << 4; //Die ersten Vier Bits geben den Dateityp an
    for(int i=0; i<9; i++){ //Die nächsten 8 Bits geben die berechtigungen an
        if(tmp_st_mode&(1<<12)){
            printf("%c", rwx_string[i]);
            if (i == 2 || i == 5 || i == 8){ //Die Bits, die angeben ob eine Datei executable sind
                isExecutable=1;
            }
        }
        else {
            printf("-");
        }
        tmp_st_mode = tmp_st_mode << 1;
    }
    //Number of links
    printf(" %d", path_stat.st_nlink);

    //Filesize
    printf(" %d", path_stat.st_size);

    //Time of last modification dd. MMM hh:mm
    struct tm * time_mtime;
    time_mtime=localtime(&path_stat.st_mtime);
    printf(" %d. %s %d:%d", time_mtime->tm_mday, months[time_mtime->tm_mon], time_mtime->tm_hour, time_mtime->tm_min);

    //name
    if (!strcmp(fileEnd, "c")){
        printf("\033[1;32m %s \033[0m", entry->d_name); //grün
    }
    else if (isExecutable==1){
        printf("\033[0;31m %s \033[0m", entry->d_name); //rot
        isExecutable=0;
    }
    else {
        printf(" %s", entry->d_name);
    }
    printf("\n");
}


void* readDirectory(void *arg){
    struct RetValues *ret = NULL;
    DIR *folder;
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
        struct stat path_stat;
        //Dateinamen zu Pfad formatieren
        strcpy(tmpString, arg);
        strcat(tmpString, "/");
        strcat(tmpString, entry->d_name);

        //String splitten
        tok="";
        strcpy(tmpStringtok, tmpString);
        strtok(tmpStringtok, ".");
        while (tok != NULL){
            //Letzter Teil nach dem letzten . bleibt in fileEnd
            strcpy(fileEnd, tok);
            tok = strtok(NULL, ".");
        }

        //Eintrag überspringen, wenn dieser mit einem . anfängt und -a nicht gesetzt wurde
        if (!strcmp(tmpStringtok, "./") && show_hidden == 0){
            continue;
        }

        //Eintrag formatieren, wenn -l gesetzt wurde
        if (listing_format==1){
            printListingFormat(tmpString);
        }
        
        else{
            printf("%s ", entry->d_name);
        }

    }
    printf("\n");
    readFinished=1;
    closedir(folder);
    return (void *)ret;
}


int main(int argc, char *argv[]){

    //Optionen -al handeln
    while ((opt = getopt(argc, argv, "al")) != -1) {
        switch (opt) {
            case 'a':
                show_hidden=1;
                break;
            case 'l':
                listing_format=1;
                break;
            default: /* '?' */
                argv[1] = ".";
        }
    }

    //Falls kein Argument angegeben wurde soll die Directory auf "." gesetzt werden
    if (optind >= argc) {
        argv[optind] = ".";
    }   

    readDirectory(argv[optind]);
    exit(0);
}