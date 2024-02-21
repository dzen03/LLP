//#include "backend.h"
//#include "dynamic_store.h"
//#include "utils.h"
//
//#include <stdio.h>
//
//
//
//int main(int argc, char** argv)
//{
//  if (argc != 2)
//    exit_with_error("Pass filename as an argument!");
//  backend_start(argv[1]);
//
//  printf("Started with %s", argv[1]);
//
//
//
//
//  backend_stop();
//  return 0;
//}
//


#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

#include "query.pb-c.h"

#define BUFFER_SIZE 1024
#define PORT 50001

// Function designed for chat between client and server.
void server_loop(int conn_fd)
{
  uint8_t buff[BUFFER_SIZE];
  int64_t n;
  int64_t msg_size;
  // infinite loop for chat
  for (;;) {

    // read the message from client and copy it in buffer

    if ((msg_size = read(conn_fd, buff, sizeof(buff))) == 0) {
      printf("Client exited");
      break;
    }

    struct Lab3__Query* query;
    query = lab3__query__unpack(NULL, msg_size, buff);



    // print buffer which contains the client contents
    printf("From client: %s\t To client : ", buff);


    // and send that buffer to client
    write(conn_fd, buff, sizeof(buff));

  }
}

// Driver function
int main()
{
  int sock_fd, conn_fd;
  uint len;
  struct sockaddr_in servaddr, cli;

  // socket create and verification
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  }
  else
    printf("Socket successfully created..\n");
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);

  // Binding newly created socket to given IP and verification
  if ((bind(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
    printf("socket bind failed...\n");
    exit(0);
  }
  else
    printf("Socket successfully binded..\n");

  // Now server is ready to listen and verification
  if ((listen(sock_fd, 5)) != 0) {
    printf("Listen failed...\n");
    exit(0);
  }
  else
    printf("Server listening..\n");
  len = sizeof(cli);

  // Accept the data packet from client and verification
  conn_fd = accept(sock_fd, (struct sockaddr*)&cli, &len);
  if (conn_fd < 0) {
    printf("server accept failed...\n");
    exit(0);
  }
  else
    printf("server accept the client...\n");

  // Function for chatting between client and server
  server_loop(conn_fd);

  // After chatting close the socket
  close(sock_fd);
}
