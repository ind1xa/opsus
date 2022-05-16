#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>

int Id; /* identifikacijski broj segmenta */
int prvaZajednickaVarijabla;            //između ulazne i radne dretve
int *drugaZajednickaVarijabla;          //između radne i izlazne dretve
FILE *ispis;
 

void *ulaznaDretva(void *x)
{
   int brojPonavljanja = *((int*)x);
   for (int i = 0; i < brojPonavljanja; i++) {
      do {
         sleep(1);
      } while (prvaZajednickaVarijabla != 0);
      prvaZajednickaVarijabla = (rand() % 100) + 1;
      printf("ULAZNA DRETVA: broj %d\n", prvaZajednickaVarijabla);
      //sleep(5);
   }
   do {
         sleep(1);
   } while (prvaZajednickaVarijabla != 0);
   prvaZajednickaVarijabla = -1;
   return NULL;
}

void drugiProces(void)
{
   ispis = fopen("ispis.txt", "w");
   fclose(ispis);
   while(1) {
      int i;
      do {
         i = *drugaZajednickaVarijabla;
         sleep(1);
      } while (i == 0);

      if (i == -1) break;

      ispis = fopen("ispis.txt", "a");
      fprintf(ispis, "%d\n", i);
      fclose(ispis);
      printf("IZLAZNI PROCES: broj upisan u datoteku %d\n", i);
      *drugaZajednickaVarijabla = 0;
   }
   printf("Završio IZLAZNI PROCES\n");
}

void brisi(int sig)
{
   /* oslobađanje zajedničke memorije */
   (void) shmdt((char *) drugaZajednickaVarijabla);
   (void) shmctl(Id, IPC_RMID, NULL);
   (void) semctl(Id, 0, IPC_RMID, 0);    //brise skup semafora
   (void) msgctl(Id, IPC_RMID, NULL);    //brise red poruka
   exit(0);
}

int main(int argc, char *argv[])
{
   int brojPonavljanja = atoi(argv[1]);
   srand(time(0));

   /* zauzimanje zajedničke memorije */
   Id = shmget(IPC_PRIVATE, sizeof(int), 0600);       //0600 znači da korisnik može čitati i pisati, a grupa i ostali ne mogu
 
   if (Id == -1)
      exit(1);  /* greška - nema zajedničke memorije */
 
   drugaZajednickaVarijabla = (int *) shmat(Id, NULL, 0);   //NULL znači da jezgra sama bira adresu
   *drugaZajednickaVarijabla = 0;
   sigset(SIGINT, brisi);       //u slučaju prekida briši memoriju
 
   /* pokretanje paralelnih procesa */
   
   //ovo je radna dretva
   printf("Pokrenuta RADNA DRETVA\n");
        
   switch (fork()) {
    case -1:
        printf("Nije moguće stvoriti proces.");
    case 0:
        printf("Pokrenut IZLAZNI PROCES\n");
        drugiProces();
        exit(0);           //0-ok 1-greska
   }
   
   pthread_t thr_id;                   //mjesto u memoriji gdje se sprema id dretve
   prvaZajednickaVarijabla = 0;

   /* pokretanje ulazne dretve */
   if (pthread_create(&thr_id, NULL, ulaznaDretva, &brojPonavljanja) != 0) {
      printf("Greska pri stvaranju dretve!\n");
      exit(1);
   } else {
      printf("Pokrenuta ULAZNA DRETVA\n");
   }

   //ovo je radna dretva
   while(1) {
      int i;
      do {
         i = prvaZajednickaVarijabla;
         sleep(1);
      } while (i == 0);
      if (i == -1) {
         *drugaZajednickaVarijabla = i;
         break;
      }
      i++;
      printf("RADNA DRETVA: broj povećan na %d\n", i);
      *drugaZajednickaVarijabla = i;
      do {
         sleep(1);
      } while(*drugaZajednickaVarijabla != 0);
      prvaZajednickaVarijabla = 0;
   }

   pthread_join(thr_id, NULL);
   printf("Završila ULAZNA DRETVA\n");
   printf("Završila RADNA DRETVA\n");

   (void) wait(NULL);      //umjesto NULL bi mogli spremit status
   brisi(0);
 
   return 0;
}