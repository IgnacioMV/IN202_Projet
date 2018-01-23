#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <netdb.h>

#include <fcntl.h>

#include <string.h> // pour memcpy

#include "copie.h"

int DFLAG;

char *srv_name = "localhost";

/* Établit une session TCP vers srv_name sur le port srv_port
 * char *srv_name: nom du serveur (peut-être une adresse IP)
 * char *srv_port: port sur lequel la connexion doit être effectuée
 *
 * retourne: descripteur vers la socket
 */
int connect_to_server(char *srv_name, char *srv_port){
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int ret_code;
  int clt_sock;

  /* Récupération des informations de connexion au serveur */
  hints.ai_flags = 0;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  //hints.ai_canonname = NULL;
  //hints.ai_addr = NULL;
  //hints.ai_addrlen = NULL;
  //hints.ai_next = NULL;

  ret_code = getaddrinfo(srv_name,srv_port,&hints,&result);

  if(ret_code == -1)
    PERROR("ret_code: -1");

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    DEBUG("before for");
    /* Tentative création de la socket */
    clt_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (clt_sock == -1) {
      printf("clt_sock: -1\n");
      continue;
    }

    /* Tentative connexion au serveur */
    if (connect(clt_sock, rp->ai_addr, rp->ai_addrlen) != -1) {
      printf("connection established\n");
      break; /* Success */
    }

    close(clt_sock);
    /* Sinon, essayer l'entrée suivante */
  }

  if (rp == NULL){  /* Aucune connexion effectuée */
    PERROR("socket");
    return -1;
  }

  freeaddrinfo(result);

  return clt_sock;
}

/* Lit le contenu du fichier par block de taille BUFF_SIZE
 * int fd: descripteur du fichier ouvert en écriture
 * int sd: descripteur de la socket
 * int fz: taille du fichier à recevoir en octets
 *
 * retourne: nombre d'octets effectivement reçus ou -1 en cas d'erreur
 */
int reception_fichier(int fd, int sd, int fz)
{
  printf("receiving file\n");
  int nb_recv = 0;                    // nombre total d'octet reçus

  unsigned char code, size;
  char *data;

  // si fichier vide, retourner 0
  if (fz == 0) 
    return 0;
  printf("---RECEIVE FILE---\n");
  for (;;) {
    int recvCode = recv_msg(sd, &code, &size, &data);
    printf("bytes received: %i", recvCode);
    printf("size: %s\n", size);
    nb_recv += size;
    
    if (size == 0)
      return 1;
  };

  return -1;
}

int main(int argc, char *argv[])
{
  int clt_sock;                       // descripteur de la socket
  char *srv_port = SRV_PORT;

  int file_fd;                        // descripteur du fichier créé
  size_t len;
  ssize_t size_sent;                  // nombre d'octets échangés

  int file_size;                      // taille du fichier à télécharger
  int nb_recv = 0;                    // nombre total d'octet reçus

  char file_name[256];


  unsigned char code, size;           // code et taille des données reçues
  char *data;                         // données reçues


  DFLAG = 1;

  if (argc < 2)
    {
      printf("%s prends comme argument le nom d'un fichier\n", argv[0]);
      exit(-1);
    }

  strncpy(file_name, argv[1], 256);

  // Création de la socket et connexion au serveur
  clt_sock = connect_to_server(srv_name, SRV_PORT);
  if (clt_sock == -1)
    printf("clt_sock: -1");

  // Envoi de la requête contenant le nom du fichier à télécharger
  int sendCode = send_msg(clt_sock, GET_FILE, (unsigned char) strlen(file_name), 
    &file_name);
  printf("sendCode: %i\n", sendCode);

  // Réception de la réponse du serveur
  int recvCode = recv_msg(clt_sock, &code, &size, &data);
  printf("file_size: %s\n", &data);
  file_size = atoi(&data);
  int recCode = reception_fichier(file_fd, clt_sock, file_size);
  printf("recCode: %i\n", recCode);
  /*file_size = atoi(file_size);
  printf("File size: %s octets\n",data);
*/

  // Test du code d'erreur retourné par le serveur
  // TODO
/*
  printf("Le fichier %s fait %d octets\n", file_name, file_size);

  // Création du fichier destination
  file_fd = open(file_name, O_CREAT|O_WRONLY|O_TRUNC, 00644);

  if ( file_fd == -1 )
    {
      perror("client: erreur lors de la création du fichier");
      send_msg(clt_sock, END_ERROR, 0, NULL);
      exit(-1);
    }
*/
  // Récupération du contenu du fichier

  return EXIT_SUCCESS;
}
