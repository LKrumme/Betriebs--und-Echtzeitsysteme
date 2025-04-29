//Imports
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <pwd.h>
#include <sys/wait.h>

//Deklarationen
char cwd[1024];
char input[1024];
int i;
char argumente[1024][1024];

//Input verarbeiten
void parseInput(){
    i = 0;
    //String an Leerzeichen splitten 
    char *ptr = strtok(input, " ");
    while (ptr != NULL)     //bis der Teilstring abgehandelt ist. Wenn strtok keinen String mehr hat sollte NULL kommen
        {
            strcpy(argumente[i], ptr);  //Pointer in Array schreiben
            
            //Teilstring auf Umgebungsvariable überprüfen.
            if(argumente[i][0]=='$'){
                strcpy(argumente[i], getenv(argumente[i]+1)); //Pointer +1 um $ zu überspringen
            }
            ptr = strtok(NULL, " ");    //Nächster Teilstring
            
            i = i +1; // i hochzählen

        }
    return;
}

//Prozessmagie
void forkProcess(){
    //Input für execvp() formatieren. Array sollte so aussehen ["ls", "/ordner", NULL, ...]
    char * argumenteZeiger[1025];
    int j;
    //J hochzählen bis zum ersten leeren String
    for(j=0; j<1024;j++){
        if(argumente[j][0]=='\0'){
            break;
        }
    }
    //Die ersten j Elemente in argumenteZeiger übertragen
    for(i=0;i<j;i++){
        argumenteZeiger[i]=argumente[i];
    }
    //Das j. Element als NULL angeben, damit execvp() weiß wann schluss ist.
    argumenteZeiger[j]=NULL;

    //Kindprozess erzeugen und behandeln
    pid_t status = fork();
    if(status<0){
        printf("Fehler beim erzeugen des Prozesses");
    }
    else if(status==0){
        execvp(argumenteZeiger[0], argumenteZeiger);
        exit(0);    
    }
    else if(status>0){
        waitpid(status, NULL, 0);
    }
}

void buildinFunctions(){
    //exit funktion ruft exit funktion auf. Funktioniert nicht, wenn Kind-Prozess noch läuft, aber dafür wird ja gewartet.
    if(!strcmp(argumente[0], "exit")){
        exit(0);
    }
    //change dir
    else if(!strcmp(argumente[0], "cd")){
        if(!strcmp(argumente[1],"")){
            printf("Usage: cd <DIR>\n");
        }
        else{
            chdir(argumente[1]);
        }
    }
    //set befehl. Vielleicht etwas faul implementiert. 
    else if(!strcmp(argumente[0], "set")){
        if(!strcmp(argumente[1],"")){
            printf("Usage: set <VARIABLE>=<VALUE>\n");
        }
        else{
            //Hier wird der String dupliziert, weil putenv einfach nur den Pointer übernimmt. Deswegen brauchen wir einen neuen Speicherbereich, den wir von strdup() bekommen.
            char *dupe = strdup(argumente[1]);
            putenv(dupe);
        }
    }
    //forkProcess wird hier aufgerufen, damit die buildinFunctions() vorrang haben. 
    else {
        forkProcess();
    }
}


//Main loop
int main (){
	while(1){
		getcwd(cwd,sizeof(cwd));
		fprintf(stdout, "%s@%s$ ", getenv("USER"), cwd); //getenv("USER") soll anscheinend sehr unschön sein, aber andere Möglichkeiten habe ich nicht hinbekommen.
		fgets(input, 1024, stdin);
        input[strlen(input)-1] = '\0'; //Zeilenumbruch am ende entfernen und durch Endcharakter ersezten. Der Zeilenumbruch wird bei fgets() mitgeliefert.
		parseInput(); //splitten
        buildinFunctions();
        //Hier wird argumente[][] invalidiert
        for (i=0; i<1024; i++){
            argumente[i][0]='\0'; //Jeder String in Argumente fängt mit dem Endcharakter an. 
        }
	}
	return 0; 
}

