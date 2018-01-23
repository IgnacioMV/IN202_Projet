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
  printf("msg_send.code == %i\n", msg.code);
  printf("msg_send.size == %i\n", msg.size);
  /* sending message head */
  send(sock, &msg, HEADSIZE, 0);
  /* sending message body if any */
  if (size != 0)
    send(sock, body, size, 0);

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
  recv(sock, &msg, HEADSIZE, 0);
  printf("msg_recv.code == %i\n",msg.code);
  
  *code = msg.code;
  *size = msg.size;

  /* receiving message body if any */
  if (msg.size != 0) {
    recv(sock, body, msg.size, 0);
    printf("body_recv == %s\n", body);
  }
  printf("msg_recv.size == %i\n",msg.size);
  printf("msg.size+HEADSIZE == %i\n", msg.size+HEADSIZE);
  return msg.size+HEADSIZE;
}
