// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <pthread.h>
#include <malloc.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

#define BACKLOG_LIMIT 5

char* getPageFromRequest(char* request){
  if(strstr(request, "GET") != NULL){
    printf("WEB GET REQUEST!\n");
  }
  else if(strstr(request, "POST") != NULL){
    printf("WEB POST REQUEST!\n");
  }
  else{
    return "<h1>404 Page Not Found!</h1>";
  }
  consoleUpdate(NULL);
  char* pageRequst = strstr(request, "/");
  printf("Page requested: %s\n", pageRequst);
  consoleUpdate(NULL);
  return "TEST\n";
}

int createWebService(){
  int sockfd, clifd;
  struct sockaddr_in server, client;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0){
    printf("Failed to create socket!\n");
    return 0;
  }
  printf("Socket created!\n");
  server.sin_addr.s_addr = gethostid();
  server.sin_port = htons(80);
  server.sin_family = AF_INET;
  char* ip = inet_ntoa(server.sin_addr);
  printf("Attempting to create a socket binding using ip %s and port 80...\n", ip);
  if(bind(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0){
    printf("FAILED! ERRNO: %s\n", strerror(errno));
    return 0;
  }
  printf("Success!\n");
  if(listen(sockfd, BACKLOG_LIMIT) < 0){
    printf("Failed to listen using the created socket! Closing!\n");
    return 0;
  }
  printf("Listening...\n");
  while(true){
    consoleUpdate(NULL);
    socklen_t length = sizeof(client);
    clifd = accept(sockfd, (struct sockaddr *) &client, &length); 
    if(clifd < 0){ 
      printf("FAILED TO ACCEPT CLIENT!\n");
      continue;
    }
    char buffer[256];
    //bzero(buffer, 256);
    printf("Accepted new client IP: %s!\n", inet_ntoa(client.sin_addr));
    int num_read = recv(clifd, buffer, 255, 0);
    if(num_read < 0){
      printf("ERROR WHILE READING!\n");
      continue;
    }
    printf("Read %d bytes\n", num_read);
    buffer[num_read] = '\0';
    printf("Received: %s\n", buffer);

    char* page = getPageFromRequest(buffer);
    //printf("I want to respond with %s\n", buffer);
    write(clifd, page, strlen(page));
    close(clifd);
    }
  return 0;
}

// Main program entrypoint
int main(int argc, char* argv[])
{
    // This example uses a text console, as a simple way to output text to the screen.
    // If you want to write a software-rendered graphics application,
    //   take a look at the graphics/simplegfx example, which uses the libnx Framebuffer API instead.
    // If on the other hand you want to write an OpenGL based application,
    //   take a look at the graphics/opengl set of examples, which uses EGL instead.
    consoleInit(NULL);
    socketInitializeDefault();
    int shouldStartWebService = 0;
    // Other initialization goes here. As a demonstration, we print hello world.
    printf("Hello World!\n");

    // Main loop
    while (appletMainLoop())
    {
      if(shouldStartWebService == 1){
        shouldStartWebService = 0;
        //TODO find an appropriate entry point for the stack. Maybe there is a good example!
        /* Thread* thread; */
        /* svcCreateThread(thread, createWebService, NULL, 1024, 0x2c, -2);  */
        /* svcStartThread(thread); */
        createWebService();
      }
        // Scan all the inputs. This should be done once for each frame
        hidScanInput();

        // hidKeysDown returns information about which buttons have been
        // just pressed in this frame compared to the previous one
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
            break; // break in order to return to hbmenu

        // Your code goes here

        if(kDown & KEY_X){
          printf("Starting web server!\n");
          shouldStartWebService = 1;
        }

        // Update the console, sending a new frame to the display
        consoleUpdate(NULL);
    }
    //Free up socket resources
    socketExit();

    // Deinitialize and clean up resources used by the console (important!)
    consoleExit(NULL);
    return 0;
}
