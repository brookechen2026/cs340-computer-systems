#include "http.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define BUFFER_SIZE 4096

void *client_thread(void *vptr) {
  int fd = (int)(ssize_t)vptr;
  
  HTTPRequest req = {0};
  
  ssize_t bytes_read = httprequest_read(&req, fd);
  if (bytes_read <= 0) {
    close(fd);
    return NULL;
  }
  
  const char *path = httprequest_get_path(&req);
  if (!path) {
    httprequest_destroy(&req);
    close(fd);
    return NULL;
  }
  
  char file_path[256] = {0};
  
  if (strcmp(path, "/") == 0) {
    snprintf(file_path, sizeof(file_path) - 1, "static/index.html");
  } else {
    snprintf(file_path, sizeof(file_path) - 1, "static%s", path);
  }
  
  int file_fd = open(file_path, O_RDONLY);
  
  if (file_fd < 0) {
    const char *response = 
      "HTTP/1.1 404 Not Found\r\n"
      "Content-Type: text/html\r\n"
      "Content-Length: 95\r\n"
      "Connection: close\r\n"
      "\r\n"
      "<html><head><title>404 Not Found</title></head>"
      "<body><h1>404 Not Found</h1></body></html>";
    
    write(fd, response, strlen(response));
  } else {
    const char *content_type = "application/octet-stream";
    const char *ext = strrchr(path, '.');
    if (ext) {
      if (strcasecmp(ext, ".html") == 0) {
        content_type = "text/html";
      } else if (strcasecmp(ext, ".png") == 0) {
        content_type = "image/png";
      }
    }
    
    struct stat st;
    if (fstat(file_fd, &st) < 0) {
      close(file_fd);
      httprequest_destroy(&req);
      close(fd);
      return NULL;
    }
    
    char header_buffer[512] = {0};
    
    int header_len = snprintf(header_buffer, sizeof(header_buffer) - 1,
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: %s\r\n"
      "Content-Length: %ld\r\n"
      "Connection: close\r\n"
      "\r\n",
      content_type, (long)st.st_size);
    
    if (header_len > 0) {
      write(fd, header_buffer, header_len);
    }
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
      write(fd, buffer, bytes_read);
    }
    
    close(file_fd);
  }
  
  httprequest_destroy(&req);
  close(fd);
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return 1;
  }
  
  char *endptr;
  long port = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0' || port <= 0 || port > 65535) {
    fprintf(stderr, "Invalid port number\n");
    return 1;
  }
  
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    return 1;
  }
  
  int opt = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    close(sockfd);
    return 1;
  }
  
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons((unsigned short)port);
  
  if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind");
    close(sockfd);
    return 1;
  }
  
  if (listen(sockfd, 10) < 0) {
    perror("listen");
    close(sockfd);
    return 1;
  }
  
  printf("Binding to port %ld. Visit http://localhost:%ld/ to interact with your server!\n", port, port);
  
  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(client_addr));
    
    int client_fd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
      if (errno == EINTR) continue;
      perror("accept");
      continue;
    }
    
    pthread_t thread;
    int rc = pthread_create(&thread, NULL, client_thread, (void*)(ssize_t)client_fd);
    if (rc != 0) {
      fprintf(stderr, "pthread_create failed: %s\n", strerror(rc));
      close(client_fd);
      continue;
    }
    
    pthread_detach(thread);
  }
  
  close(sockfd);
  return 0;
}