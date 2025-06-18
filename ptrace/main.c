#include <stdlib.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>


//Deklarationen 
int opt;
char argumente[1024][1024];
struct user_regs_struct ptrace_register;  
int status_2;
char path[1024];


void readPathPtrace(pid_t child, unsigned long addr) {
    int i = 0;
    unsigned long word;

    //Schleife bricht ab, wenn der pfad aus dem Register größer sein sollte als der Speicherbereich des Strings
    while (i < sizeof(path)) {
        //PEEKDATA und fehlerbehandlung
        word = ptrace(PTRACE_PEEKDATA, child, addr + i, NULL);
        if (errno != 0) {
            break;
        }

        //Der aktuelle char wird in path an die richtige stelle kopiert. Es wird abgebrochen, wenn der char Leer ist und somit der String zuende.
        memcpy(path + i, &word, sizeof(word));
        if (memchr(&word, 0, sizeof(word)) != NULL) {
            break; 
        }
        i += sizeof(word);
    }
    printf("%s\n", path);
}

int main(int argc, char* argv[]){
    if (argc < 2){
        printf("Du musst ein Argument übergeben! \n");
        exit(0);
    }
    //Kindprozess erzeugen und behandeln
    pid_t status = fork();
    if(status<0){
        printf("Fehler beim erzeugen des Prozesses \n");
    }
    else if(status==0){
        //tracee
        ptrace(PTRACE_TRACEME, NULL, NULL, NULL);
        execvp(argv[1], &argv[1]);
        exit(0);
    }
    else if(status>0){
        //tracer
        printf("PID-Child: %d \n", status);
        
        //Warten bis tracee beendet
        while (waitpid(status, &status_2, 0) > 0 ){

            if(WIFSTOPPED(status_2)){
                ptrace(PTRACE_GETREGS, status, NULL, &ptrace_register);

                if(ptrace_register.orig_rax == 231){ //exit_group
                    printf("exit_group \n");
                    exit(0);
                }
                else if (ptrace_register.orig_rax == 60){ //exit
                    printf("exit \n");
                    exit(0);
                }
                else if (ptrace_register.orig_rax == 1){ //write
                    printf("write: %d \n", ptrace_register.rax);
                }
                else if (ptrace_register.orig_rax == 2){ //open
                    printf("open: ");
                    readPathPtrace(status, ptrace_register.rdi);
                }
                else if (ptrace_register.orig_rax == 257){ //openat
                    printf("openat: ");
                    readPathPtrace(status, ptrace_register.rsi);
                }
                else {
                    printf("Systemaufruf: %lld \n",ptrace_register.orig_rax);
                }
            }
            //PTRACE_SYSCALL wird erst hier unten aufgerufen, da sonst probleme beim auslesen der Register mit PEEKDATA enstehen
            ptrace(PTRACE_SYSCALL, status, NULL, NULL);
        }
    }
}