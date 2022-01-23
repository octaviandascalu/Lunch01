/* 
  Server bazat pe servTCPPreTh.c
  servTCPPreTh.c - Exemplu de server TCP concurent care deserveste clientii 
  printr-un mecanism de prethread-ing; cu blocarea mutex de protectie a lui accept(); 
  Serverul mai foloseste in plus inca un mutex pentru protejarea array-ului in care se salveaza comenzile neprocesate
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

/* portul folosit */
#define PORT 2910

/* codul de eroare returnat de anumite apeluri */
extern int errno;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
//void raspunde(void *);

typedef struct
{
  pthread_t idThread; //id-ul thread-ului
  int thCount;        //nr de conexiuni servite
} Thread;

Thread *threadsPool; //un array de structuri Thread

int sd;                                                   //descriptorul de socket de ascultare
int nthreads;                                             //numarul de threaduri N
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;        // variabila mutex ce va fi partajata de threaduri
pthread_mutex_t comenzi_lock = PTHREAD_MUTEX_INITIALIZER; // variabila mutex ce va fi partajata de threaduri

int T;
int comenzi[6];
int comanda_finala = -1;

void raspunde(int cl, int idThread);
int decideComanda();

int main(int argc, char *argv[])
{
  struct sockaddr_in server; // structura folosita de server
  void threadCreate(int);

  if (argc < 2)
  {
    fprintf(stderr, "Eroare: Primul argument este numarul de fire de executie...\n");
    exit(1);
  }
  nthreads = atoi(argv[1]);
  if (nthreads <= 0)
  {
    fprintf(stderr, "Eroare: Numar de fire invalid...\n");
    exit(1);
  }

  if (argc < 3)
  {
    fprintf(stderr, "Eroare: Al doilea argument este numarul de secunde...\n");
    exit(1);
  }
  T = atoi(argv[2]);
  if (T <= 5 || T >= 120)
  {
    fprintf(stderr, "Eroare: Numar de secunde invalid...\n");
    exit(1);
  }

  threadsPool = calloc(sizeof(Thread), nthreads);

  /* crearea unui socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */
  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  /* pregatirea structurilor de date */
  bzero(&server, sizeof(server));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }

  printf("Nr threaduri %d \n", nthreads);
  fflush(stdout);
  int i;
  for (i = 0; i < nthreads; i++)
    threadCreate(i);

  sleep(1);
  while (1)
  {
    printf("Incepe luarea de comenzi\n");
    sleep(T - 1);
    pthread_mutex_lock(&comenzi_lock);
    printf("Se decide felul servit\n");
    comanda_finala = decideComanda();
    sleep(1);
    pthread_mutex_unlock(&comenzi_lock);
    comanda_finala = -1;
  }
};

int decideComanda()
{
  int finala = 1;
  int i;
  for (i = 1; i < 6; i++)
  {
    if (comenzi[i] > comenzi[finala])
    {
      finala = i;
    }
  }

  /*resetez istoricul comenzilor*/
  for (i = 1; i < 6; i++)
  {
    comenzi[i] = 0;
    // printf("Comenzi[%d]: %d\n", i, comenzi[i]);
  }

  return finala;
}

void threadCreate(int i)
{
  void *treat(void *);

  pthread_create(&threadsPool[i].idThread, NULL, &treat, (void *)(intptr_t)i);
  return; /* threadul principal returneaza */
}

void *treat(void *arg)
{
  int client;

  struct sockaddr_in from;
  bzero(&from, sizeof(from));
  printf("[thread]- %ld - pornit...\n", (intptr_t)arg);
  fflush(stdout);

  for (;;)
  {
    int length = sizeof(from);
    pthread_mutex_lock(&mlock);
    printf("Thread %ld trezit\n", (intptr_t)arg);
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[thread] Eroare la accept().\n");
    }
    pthread_mutex_unlock(&mlock);
    threadsPool[(intptr_t)arg].thCount++;

    raspunde(client, (intptr_t)arg); //procesarea cererii
    /* am terminat cu acest client, inchidem conexiunea */
    close(client);
  }
}

void raspunde(int cl, int idThread)
{
  printf("Am intrat in Thread %d\n", idThread);

  /* returnam fd_id-ul clientului */
  if (write(cl, &cl, sizeof(int)) <= 0)
  {
    printf("[Thread %d] ", idThread);
    perror("[Thread] Eroare la write() catre client pentru id.\n");
  }
  /* returnam thread_id-ul clientului */
  if (write(cl, &idThread, sizeof(int)) <= 0)
  {
    printf("[Thread %d] ", idThread);
    perror("[Thread] Eroare la write() catre client pentru idThread.\n");
  }

  /*procesam comenzile clientului*/

  int comanda; //mesajul primit de trimis la client
  int decizie_server;
  int bytes;

  while ((bytes = read(cl, &comanda, sizeof(int))) > 0)
  {
    printf("[Thread %d] Mesajul a fost receptionat...%d\n", idThread, comanda);

    pthread_mutex_lock(&comenzi_lock);
    comenzi[comanda] += 1;
    pthread_mutex_unlock(&comenzi_lock);

    printf("Comenzi: ");
    for (int i = 1; i < 6; i++)
      printf("%d ", comenzi[i]);
    printf("\n");

    while (comanda_finala == -1)
    {
      ;
    }

    decizie_server = comanda_finala;
    printf("Comanda finala este %d\n", decizie_server);

    if (comanda == decizie_server)
    {
      /*pregatim mesajul de raspuns */
      printf("[Thread %d] Trimitem mesajul inapoi...Masa e servita!\n", idThread);
      char msg[101] = "Masa e servita!";
      /* returnam mesajul clientului */
      if (write(cl, &msg, sizeof(msg)) <= 0)
      {
        printf("[Thread %d] ", idThread);
        perror("[Thread] Eroare la write() catre client.\n");
      }
      else
        printf("[Thread %d] Mesajul a fost trasmis cu succes.\n", idThread);
    }
    else
    {
      /*pregatim mesajul de raspuns */
      printf("[Thread %d] Trimitem mesajul inapoi...Indisponibil!\n", idThread);
      char msg[101] = "Indisponibil!";
      /* returnam mesajul clientului */
      if (write(cl, &msg, sizeof(msg)) <= 0)
      {
        printf("[Thread %d] ", idThread);
        perror("[Thread] Eroare la write() catre client.\n");
      }
      else
      {
        printf("[Thread %d] Mesajul a fost trasmis cu succes.\n", idThread);
        // break;
      }
    }
  }

  if (bytes <= 0)
  {
    close(cl);
  }
}
