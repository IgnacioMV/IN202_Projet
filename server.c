#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

#include "copie.h"

#define BACK_LOG 10            // nombre maximal de demandes de connexions en attente

int DFLAG;

/* Crée une socket d'écoute
 * srv_port: numéro de port d'écoute sous forme de chaîne de caractères
 * maxconn: nombre maximum de demandes de connexions en attente
 *
 * retourne la socket créée
 */
int create_a_listening_socket(char *srv_port, int maxconn){
  printf("creating a listening socket\n");
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int ret_code;
  int srv_sock;
//getaddrinfo, socket, bind, listen
  /* Récupération des paramètres de création de la socket */

  hints.ai_flags = AI_PASSIVE; //
  hints.ai_family = AF_INET; //IPv4, AF_INET6 pour IPv6
  hints.ai_socktype = SOCK_STREAM; //car on fait la tranfert d'un fichier
  hints.ai_protocol = 0;
  //hints.ai_canonname = NULL;
  //hints.ai_addr = NULL;
  //hints.ai_addrlen = NULL;
  //hints.ai_next = NULL;

  ret_code = getaddrinfo(NULL,srv_port,&hints,&result);

  if(ret_code == -1)
    PERROR("ret_code: -1");

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    /* Tentative de création de la socket */
    srv_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (srv_sock == -1)
      continue;

    /* Tentative d'appel à bind */
    if (bind(srv_sock, rp->ai_addr, rp->ai_addrlen) == 0) {
      printf("server bound\n");
      break;
    }

    close(srv_sock);
  }

  if (rp == NULL ) {               /* Aucune socket créée */
    fprintf(stderr, "Could not bind\n");
    return -1;
  }

  freeaddrinfo(result);
//listen
  /* Configuration de la socket en écoute passive */
  int listenCode = listen(srv_sock, maxconn);
  printf("listenCode: %i\n",listenCode);

  return srv_sock;
}

/* Attend l'arrivée de demandes de connexions
 * srv_sock: la socket d'écoute
 * clt_sockaddr: pointeur sur une structure sockaddr devant recevoir
 *               les informations de connexion du client
 *
 * retourne: le numéro de la socket créée pour le nouveau client
 */
int accept_clt_conn(int srv_sock, struct sockaddr_storage *clt_sockaddr){
  printf("accepting client connection\n");
  int clt_sock;
  socklen_t addrlen = sizeof(struct sockaddr_storage);

  /* mise en attente de connexion sur la socket */
  clt_sock = accept(srv_sock, 0, 0);
  printf("clt_sock: %i\n",clt_sock);

  //DEBUG("connexion accepted");

  return clt_sock;
}

/* Transfert le contenu du fichier par block de BUFF_SIZE octets
 * int fd: descripteur du fichier ouvert en lecture
 * int sd: descripteur de la socket
 *
 * retourne: nombre d'octets effectivement transmis
 */
ssize_t transfert_fichier(int sd, int fd) {
  char buff[BUFF_SIZE];

  ssize_t size;                  // nombre d'octets échangés
  ssize_t nb_sent = 0;           // nombre total d'octets envoyés

  /* lecture du fichier par clock de BUFF_SIZE octet
     et envoi du contenu au client */
  printf("---SEND FILE---\n");
  while ((size = read(fd, buff, BUFF_SIZE)) > 0)
  {
    
    nb_sent += size;
    printf("size: %zd\n", size);
    send_msg(sd, DATA, size, &buff);
  };
  printf("nb_sent total: %zd\n", nb_sent);
  /* Envoi du message de fin de fichier */
  send_msg(sd, END_OK, 0, NULL);

  return nb_sent;
}

/*
 * Analyse la demande d'un client
 */
int requete_client(int sock)
{
  char *buff;
  int file_fd;
  struct stat file_stat;
  unsigned char code;
  unsigned char size;
  char file_size[BUFF_SIZE];

  // Réception de la requête contenant le nom du fichier à télécharger
  int reqCode = recv_msg(sock, &code, &size, &buff);
  //printf("received %i octets\n", size);
  if (code == GET_FILE)
    printf("GET_FILE\n");
  
  printf("reqCode: %i\n",reqCode);
  if (reqCode == -1)
    PERROR("reqCode: -1");

  // Ouverture en lecture du fichier demandé
  printf("fileName: %s\n", &buff);
  file_fd = open(&buff, O_RDONLY);
  printf("file_fd: %i\n", file_fd);

  if ( file_fd == -1 )
    { // Gestion des cas d'erreur lors de l'ouverture du fichier
      return -1;
    }

  // récupération des informations relatives au fichier
  if (fstat(file_fd, &file_stat) < 0)
    printf("fstat failed\n");
  printf("File Size: \t\t%d bytes\n",file_stat.st_size);
  snprintf(file_size, BUFF_SIZE, "%llu", (unsigned long long)file_stat.st_size);
  // envoi de la réponse au client (code + taille du fichier)
  int sendCode = send_msg(sock, ACCESS_OK, strlen(file_size)+1, file_size);
  printf("sendCode: %i\n",sendCode);

  return file_fd;

}

int main(void)
{
  printf("running main\n");
  int socket_fd;             // socket correspondant au port sur lequel le serveur attend
  int con_fd;                // socket créée lors de l'établissement d'une connexion avec un client

  struct sockaddr_storage sockaddr_client;

  int file_fd;               // descripteur de fichier pour le fichier à transmettre

  // création de la socket

  socket_fd = create_a_listening_socket(SRV_PORT,BACK_LOG);

  for (;;) // boucle infinie: en attende de demande de connexion
    {
      printf("running main loop\n");
      // acceptation d'une demande de connexion
      con_fd = accept_clt_conn(socket_fd, &sockaddr_client);
      if (con_fd == -1)
        printf("con_fd = %i\n", con_fd);

      // lecture de la requête client
      file_fd = requete_client(con_fd);

      // transfert du fichier
      transfert_fichier(con_fd, file_fd);

      // fermeture du fichier

      // fermer le socket
    } // fin for

  return EXIT_SUCCESS;
}
