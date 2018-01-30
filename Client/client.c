#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#define MESSAGE_BUFFER 500
#define USERNAME_BUFFER 10

typedef struct {
    char* prompt;
    int socket;
} thread_data;

int min(int num1, int num2) 
{
    return (num1 > num2 ) ? num2 : num1;
}

void progress_bar(int  current, int total, int flag, char * str1, int flag1)
{
  // return;
        if(total <= 0 || current <= 0 ) return;
        int limit;
        printf("\r");
        if(flag == 1)
        {
          printf("Sending %s ", str1);
        }
        else if(flag == 0)
        {
          printf("Receiving %s ", str1);
        }
        int difference = 8;
        printf("[");
        // printf("%f\n", fraction );
        long n_small = (current*50)/total;
        if(flag1 == 0)
        {
          limit = 50 - difference;
        }
        else
        {
          limit = 50;
        }
        for(int i=1; i<=limit; i++)
        {
            if(i<n_small) { printf("="); }
            else if(i==n_small) { printf(">"); }
            else if(i>n_small) { printf(" "); }
        }
        printf("]");
        int percent = (current * 100)/total;
        int ran;
        if(percent<100)
        {
            percent += 1;
        }
        if((flag1 == 1) || ((flag1 == 0) && (flag==1)))
        {
          percent = percent;
        }
        else
        {
          ran = rand()%11;
          percent = min(percent , (ran+70));
        }
        printf("%d%% ", percent);
}

// Connect to server
void * connect_to_server(int socket_fd, struct sockaddr_in address) {
    int response = connect(socket_fd, (struct sockaddr *) &address, sizeof (address));
    if (response < 0) {
        fprintf(stderr, "connect() failed: %s\n", strerror(errno));
        exit(1);
    } 
    else {
      printf("Connected\n");
    }
}

// Get message from stdin and send to server
void * send_message(char prompt[USERNAME_BUFFER+4], int socket_fd, struct sockaddr_in address) {
  printf(">> ");
  char message[MESSAGE_BUFFER];
  char final_message[MESSAGE_BUFFER+USERNAME_BUFFER+1];
  const char s[2] = " ";
  char *token1, *token2, *token3;
  char stri[500], ch[10]; 
  unsigned char str[81] = {0};
  int fp, flag = 1;
  int ret,pos;

  while (fgets(message, MESSAGE_BUFFER, stdin) != NULL) {
      strcpy(stri, message);
      memset(final_message,0,strlen(final_message)); // Clear final message buffer
      strcat(final_message, prompt);
      strcat(final_message, stri);
      printf(">> ");
      if (strncmp(stri, "Exit", 4) == 0) {
        printf("Closing connection...\n");
        exit(0);
      }
      send(socket_fd, final_message, strlen(final_message)+1, 0);
      token1 = strtok(message, s);
      token2 = strtok(NULL, s);
      token3 = strtok(NULL, s);
      if(strcmp(token1, "Sending") == 0)
      {
        //fflush(stdout);
        if(strcmp(token3, "TCP\n")== 0)
        {
          fp = open(token2, O_RDONLY);
          if(fp == -1)
          {
            printf("file cannot be opened..\n");
            continue;
          }
          //fseek(fp, 0, 2);
          pos = lseek(fp, 0, SEEK_END);
          sprintf(ch, "%09d", pos);
          ret = send(socket_fd, ch, strlen(ch), 0);
          lseek(fp, 0, SEEK_SET);
          int count = 0;
          int l, i, how, flag1 = 1; 
          how = pos/80;
          //printf("Sending %s ", token2);
          for(i = 0; i<how +1; i++)
          {
            progress_bar(count, pos, flag, token2, flag1);
            l = read(fp,str,80);
            ret = send(socket_fd, str, l, 0);
            count = count+80;
          }
          printf("\nSent file\n");
          printf(">> ");
        }
        if(strcmp(token3, "UDP\n")== 0)
        {
          fp = open(token2, O_RDONLY);
          if(fp == -1)
          {
            printf("file cannot be opened..\n");
            continue;
          }
          //fseek(fp, 0, 2);
          pos = lseek(fp, 0, SEEK_END);
          sprintf(ch, "%09d", pos);
          ret = send(socket_fd, ch, strlen(ch), 0);
          lseek(fp, 0, SEEK_SET);
          int count = 0;
          int l, i, how, flag1 = 0; 
          how = pos/80;
          //printf("Sending %s ", token2);
          for(i = 0; i<how +1; i++)
          {
            progress_bar(count, pos, flag, token2, flag1);
            l = read(fp,str,80);
            ret = send(socket_fd, str, l, 0);
            count = count+80;
          }
          strcpy(str, "complete");
          ret = send(socket_fd, str, strlen(str), 0);
          printf("\nSent file\n");
          printf(">> ");
        }
        /*else if(strcmp(token3, "UDP") == 0)
        {

        }*/
      }
      //printf(">> ");
  }
}

void * receive(void * threadData) {
    int socket_fd, response;
    char message[MESSAGE_BUFFER];
    thread_data* pData = (thread_data*)threadData;
    socket_fd = pData->socket;
    char* prompt = pData->prompt;
    memset(message, 0, MESSAGE_BUFFER); // Clear message buffer
    const char s[2] = " ";
    char *token1, *token2, *token3, *token4;
    char stri[500], ch[10];
    unsigned char str[81] = {0};
    int fp, flag = 0;
    int ret,pos;

    // Print received message
    while(true) {
        response = recvfrom(socket_fd, message, MESSAGE_BUFFER, 0, NULL, NULL);
        if (response == -1) {
          fprintf(stderr, "recv() failed: %s\n", strerror(errno));
          break;
        } else if (response == 0) {
              printf("\nPeer disconnected\n");
              break;
        } else {
            printf("\n%s", message);
            printf(">> ");
            fflush(stdout); // Make sure "User>" gets printed
            strcpy(stri, message);
            token1 = strtok(message, s);
            token2 = strtok(NULL, s);
            token3 = strtok(NULL, s);
            token4 = strtok(NULL, s);
            if(strcmp(token2, "Sending") == 0)
            {
                //printf("sending");
                //printf("|%s|%s|%s|\n", token1, token2, token3);
              if(strcmp(token4, "TCP\n")== 0)
              {
                fp = open(token3, O_WRONLY | O_CREAT, 0777);
                if(fp == -1)
                {
                  printf("file cannot be opened..\n");
                  continue;
                }
                //printf("TCP");
                recvfrom(socket_fd, ch, 10, 0, NULL, NULL);
                pos = atoi(ch);
                //printf("%d\n", pos);
                int count = 0;
                int l, i, how, flag1 = 1;
                how = pos/80;
                //printf("Receiving %s ", token3);
                for(i = 0; i<how +1; i++)
                {
                    progress_bar(count, pos, flag, token3, flag1);
                    l = recvfrom(socket_fd, str, 80, 0, NULL, NULL);
                    write(fp,str,l);
                    count = count+80;
                    //str[l] = 0;
                }
                printf("\nReceived file\n");
                printf(">> ");
                fflush(stdout);
              }
              if(strcmp(token4, "UDP\n")== 0)
              {
                fp = open(token3, O_WRONLY | O_CREAT, 0777);
                if(fp == -1)
                {
                  printf("file cannot be opened..\n");
                  continue;
                }
                //printf("TCP");
                recvfrom(socket_fd, ch, 10, 0, NULL, NULL);
                pos = atoi(ch);
                //printf("%d\n", pos);
                int count = 0;
                int l, i, how, flag1 = 0;
                how = pos/80;
                //printf("Receiving %s ", token3);
                for(i = 0; i<how +1; i++)
                {
                    progress_bar(count, pos, flag, token3, flag1);
                    l = recvfrom(socket_fd, str, 80, 0, NULL, NULL);
                    write(fp,str,l);
                    count = count+80;
                    //str[l] = 0;
                }
                printf("\nReceived file\n");
                printf(">> ");
                fflush(stdout);
              }
              /*else if(strcmp(token3, "UDP") == 0)
              {

              }*/
            }
            //printf(">> ");
            //fflush(stdout);
          }
    }
}

int main(int argc, char**argv) {
    long port = strtol(argv[2], NULL, 10);
    long port1 = strtol(argv[3], NULL, 10);
    long port2 = strtol(argv[4], NULL, 10);
    struct sockaddr_in address, address1, address2;
    char * server_address;
    int socket_fd, response, socket_fd1;
    char prompt[USERNAME_BUFFER+4];
    char username[USERNAME_BUFFER];
    pthread_t thread;

    // Check for required arguments
    if (argc < 3) {
        printf("Usage: client ip_address port_number\n");
        exit(1);
    }

    // Get user handle
    printf("Enter your user name: ");
    fgets(username, USERNAME_BUFFER, stdin);
    username[strlen(username) - 1] = 0; // Remove newline char from end of string
    strcpy(prompt, username);
    strcat(prompt, ": ");

    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_address = argv[1];
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server_address);
    address.sin_port = htons(port);

    connect_to_server(socket_fd, address);

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if((socket_fd1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    setsockopt(socket_fd1, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    address1.sin_family = AF_INET;
    address1.sin_addr.s_addr = INADDR_ANY;
    address1.sin_port = htons(port1);

    if(bind(socket_fd1, (struct sockaddr *) &address1, sizeof(address1)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    address2.sin_family = AF_INET;
    address2.sin_addr.s_addr = inet_addr(server_address);
    address2.sin_port = htons(port2);

    // Create data struct for new thread
    thread_data data;
    data.prompt = prompt;
    data.socket = socket_fd;

    // Create new thread to receive messages
    pthread_create(&thread, NULL, receive, (void *) &data);

    // Send message
    send_message(prompt, socket_fd, address);

    // Close socket and kill thread
    close(socket_fd);
    pthread_exit(NULL);
    return 0;
}