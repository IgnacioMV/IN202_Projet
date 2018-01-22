#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "copie.h"

/* send_msg send a message on socket sock
   sock: the socket
   code: message's protocol code
   size: message's size
   msg: message to be sent
*/
int send_msg(int sock, unsigned char code, unsigned char size, char *body)
{
  msg_t msg;
  msg.code = code;
  msg.size = size;
  /* sending message head */
  send(sock, msg.code, BUFF_SIZE, 0);
  send(sock, msg.size, BUFF_SIZE, 0);
  /* sending message body if any */
  send(sock, body, BUFF_SIZE, 0);

  return size+HEADSIZE;
}

/* recv_msg recv a message from the socket sock
   sock: the socket
   code: message's protocol code
   size: message's size
   msg: message to be received
*/
int recv_msg(int sock, unsigned char *code, unsigned char *size, char **body)
{
  msg_t msg;

  /*receiving message head */
  recv(sock, msg.code, BUFF_SIZE, 0);
  printf("msg.code == %i\n",msg.code);
  if (msg.code == 11)
    printf("GET_FILE\n");
  recv(sock, msg.size, BUFF_SIZE, 0);
  printf("msg.size == %i\n",msg.size);
  /* receiving message body if any */
  recv(sock, body, BUFF_SIZE, 0);

  return *size+HEADSIZE;
}
