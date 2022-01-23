/* 
  Clientul are la baza modelul de client TCP utlizat la curs : cliTCPIt.c
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h> //pt warning inaet_addr

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare

  // mesajul trimis
  int nr = 0;
  char buf[10];

  int id;
  int idThread;
  int refuz = 0;
  int comanda;
  int T;
  char raspuns_comanda[101];

  int seed = time(0);
  srand(seed);

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi(argv[2]);

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons(port);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  /*citirea id client de la server*/
  if (read(sd, &id, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server la citirea id-ului.\n");
    return errno;
  }
  
  /*citirea idThread client de la server*/
  if (read(sd, &idThread, sizeof(int)) < 0)
  {
    perror("[client]Eroare la read() de la server la citirea idThread-ului.\n");
    return errno;
  }

  printf("[client] Id asignat: %d\n", id);
  fflush(stdout);
  printf("[client] IdThread asignat: %d\n", idThread);
  fflush(stdout);

  while (refuz < 3)
  {
    comanda = rand() % 5 + 1;
    printf("[client] Am comandat mancarea %d\n", comanda);
    fflush(stdout);

    /* trimiterea comanda la server */
    if (write(sd, &comanda, sizeof(int)) <= 0)
    {
      perror("[client]Eroare la write() spre server la trimiterea comenzii.\n");
      return errno;
    }

    /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
    if (read(sd, &raspuns_comanda, sizeof(raspuns_comanda)) < 0)
    {
      perror("[client]Eroare la read() de la server pentru raspunsul la comanda.\n");
      return errno;
    }

    /* afisam mesajul primit */
    printf("[client] %s\n", raspuns_comanda);

    if (strcmp(raspuns_comanda, "Indisponibil!") == 0)
    {
      refuz += 1;
      if (refuz < 3)
      {
        T = rand() % 60 + 1;
        printf("[client] Revin in %d secunde\n", T);
        fflush(stdout);
        sleep(T);
      }
    }
    else if (strcmp(raspuns_comanda, "Masa e servita!") == 0)
    {
      printf("[client] Satul!\n");
      fflush(stdout);
      break;
    }
  } //end while servire

  if (refuz > 2)
  {
    printf("[client] Schimb cantina! Aici mor de foame!\n");
    fflush(stdout);
  }

  /* inchidem conexiunea, am terminat */
  close(sd);
}
