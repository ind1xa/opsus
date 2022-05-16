#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <signal.h>
#include <math.h>

void obradi_sigusr1(int sig);
void obradi_sigterm(int sig);
void obradi_sigint(int sig);

int broj;
FILE *obrada;
FILE *status;

int main()
{
    struct sigaction act;

    /* 1. maskiranje signala SIGUSR1 */
    act.sa_handler = obradi_sigusr1; /* kojom se funkcijom signal obrađuje */
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTERM); /* blokirati i SIGTERM za vrijeme obrade */
    act.sa_flags = 0;                 /* naprednije mogućnosti preskočene */
    sigaction(SIGUSR1, &act, NULL);   /* maskiranje signala preko sučelja OS-a */
    
    /* 2. maskiranje signala SIGTERM */
    act.sa_handler = obradi_sigterm;
    sigemptyset(&act.sa_mask);
    sigaction(SIGTERM, &act, NULL);     /*konstante: SIG_DFL (default), SIG_IGN (ignore), adresa u našem slučaju */

    /* 3. maskiranje signala SIGINT */
    act.sa_handler = obradi_sigint;
    sigaction(SIGINT, &act, NULL);

    printf("Program s PID=%ld krenuo s radom\n", (long)getpid());

    status = fopen("status.txt", "r");
    fscanf(status, "%d", &broj);
    fclose(status);
    obrada = fopen("obrada.txt", "r");
    if (broj == 0) {
        while (fscanf(obrada, "%d", &broj) != -1);
        broj = sqrt(broj);
    }
    fclose(obrada);
    status = fopen("status.txt", "w");
    fprintf(status, "%d", 0);
    fclose(status);
    int x;
    while (1 == 1) {
        broj++;
        x = broj * broj;
        obrada = fopen("obrada.txt", "a");
        fprintf(obrada, "\n%d", x);
        fclose(obrada);
        for (int i = 0; i < 5; i++) {
            sleep(1);
        }
    }

    return 0;
}

void obradi_sigusr1(int sig)
{
    printf("%d\n", broj);
}

void obradi_sigterm(int sig)
{
    status = fopen("status.txt", "w");
    fprintf(status, "%d", broj);
    fclose(status);
    exit(1);
}

void obradi_sigint(int sig)
{
    exit(1);
}