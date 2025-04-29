#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <pwd.h>
#include <sys/wait.h>

char cwd[1024];
char input[1024];
int i;
char argumente[1024][1024];

void parseInput(){
    int i = 0;
    //String an Leerzeichen splitten 
    char *ptr = strtok(input, " ");
    while (ptr != NULL)     //bis der Teilstring abgehandelt ist
        {
            printf("splitted %s \n", ptr);

            //Teilstring auf Umgebungsvariable überprüfen.
            strcpy(argumente[i], ptr);  //Pointer in Array schreiben

            if(argumente[i][0]=='$'){
                strcpy(argumente[i], getenv(argumente[i]+1)); //pointer +1 um $ zu überspringen
            }
            ptr = strtok(NULL, " ");    //nächster Teilstring
            
            i = i +1; // i iterieren

        }
    return;
}

void forkProcess(){
    char * argumenteZeiger[1025];
    int j;
    for(j=0; j<1024;j++){
        if(argumente[j][0]=='\0'){
            break;
        }
    }
    for(i=0;i<j;i++){
        argumenteZeiger[i]=argumente[i];
    }
    argumenteZeiger[j]=NULL;
    pid_t status = fork();
    if(status<0){
        printf("Fehler du dödel");
    }
    else if(status==0){
        printf("Penis\n");
        execvp(argumenteZeiger[0], argumenteZeiger);
        exit(0);    
    }
    else if(status>0){
        printf("%d", status);
        waitpid(status, NULL, 0);
    }
}

void buildinFunctions(){
    if(!strcmp(argumente[0], "exit")){
        exit(0);
    }
    else if(!strcmp(argumente[0], "cd")){
        if(!strcmp(argumente[1],"")){
            printf("Usage: cd <DIR>\n");
        }
        else{
            chdir(argumente[1]);
        }
    }
    else if(!strcmp(argumente[0], "set")){
        if(!strcmp(argumente[1],"")){
            printf("Usage: set <VARIABLE>=<VALUE>\n");
        }
        else{
            char *dupe = strdup(argumente[1]);
            putenv(dupe);
        }
    }
    else {
        forkProcess();
    }
}


int main (){
	while(1){
		getcwd(cwd,sizeof(cwd));
		fprintf(stdout, "%s@%s$ ", getenv("USER"), cwd);
		fgets(input, 1024, stdin);
        input[strlen(input)-1] = '\0'; //Zeilenumbruch am ende entfernen und durch Endcharakter ersezten.
		parseInput(); //splitten
		fprintf(stdout, "Deine Eingabe: %s %s\n", argumente[0], argumente[1]);
        buildinFunctions();
        //Hier wird argumente invalidiert
        for (i=0; i<1024; i++){
            argumente[i][0]='\0'; //Jeder String in Argumente fängt mit dem Endcharakter an. 
        }
	}
	return 0; 
}

