#include "lib/catch.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>

#include "../http.h"




static int pipe_errors = 0;
void catch_sigpipe(int) {
    // SIGPIPE is sent if one end of socket closes before the other. For these tests, we just ignore that, but we also count it here so that you can use a breakpoint on this line to discover if it's happening in your code.
    pipe_errors += 1;
}

void *socketGetting_helper(void *arg) {
    struct { int fd;  const void *data; size_t len; } *p = arg;
    size_t so_far = 0;
    while (so_far < p->len) {
        ssize_t sent = write(p->fd, p->data, p->len);
        if (sent <= 0) break;
        so_far += sent;
    }
    close(p->fd);
    free(arg);
    return NULL;
}
int socketGetting(const void *data, size_t len) {
    int fd[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    
    struct { int fd; const void *data; size_t len; } *p = malloc(3*sizeof(size_t));
    p->fd = fd[0];
    p->data = data;
    p->len = len;
    
    pthread_t tid;
    pthread_create(&tid, NULL, socketGetting_helper, p);
    pthread_detach(tid);
    return fd[1];
}

HTTPRequest *_readpipe_vptr(const void *s, size_t len) {
  char *buf = (char *) malloc(len + 1);
  memcpy(buf, s, len);
  buf[len] = 0;

  HTTPRequest *req = (HTTPRequest *) malloc(sizeof(HTTPRequest));

  int readfd = socketGetting(buf, len);

  httprequest_read(req, readfd);
  close(readfd);

  free(buf);

  return req;
}

HTTPRequest *_readpipe(const char *s) {
  return _readpipe_vptr(s, strlen(s));
}


TEST_CASE("httprequest_read - Request Line", "[weight=2][part=2]") {
  HTTPRequest *req = _readpipe("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");

  CAPTURE( req->action );
  REQUIRE( req->action != NULL );
  CHECK( strcmp(req->action, "GET") == 0 );

  CAPTURE( req->path );
  REQUIRE( req->path != NULL );
  CHECK( strcmp(req->path, "/") == 0 );

  CAPTURE( req->version );
  REQUIRE( req->version != NULL );
  CHECK( strcmp(req->version, "HTTP/1.1") == 0 );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_read - First Header", "[weight=2][part=2]") {
  HTTPRequest *req = _readpipe("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");

  const char *host = httprequest_get_header(req, "Host");
  CAPTURE( host );
  REQUIRE( host != NULL );
  CHECK( strcmp(host, "localhost") == 0 );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_read - Two Headers", "[weight=2][part=2]") {
  HTTPRequest *req = _readpipe("GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\n\r\n");

  const char *hello = httprequest_get_header(req, "Hello");
  CAPTURE( hello );
  REQUIRE( hello != NULL );
  CHECK( strcmp(hello, "World") == 0 );

  const char *host = httprequest_get_header(req, "Host");
  CAPTURE( host );
  REQUIRE( host != NULL );
  CHECK( strcmp(host, "127.0.0.1") == 0 );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_read - Content-Length Header", "[weight=2][part=2]") {
  HTTPRequest *req = _readpipe("GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\nContent-Length: 10\r\n\r\n0123456789");

  const char *content_length = httprequest_get_header(req, "Content-Length");
  CAPTURE( content_length );
  REQUIRE( content_length != NULL );
  CHECK( strcmp(content_length, "10") == 0 );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_read - Invalid Content-length", "[weight=2][part=2]") {
  HTTPRequest *req = _readpipe("GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\nContent-Length: -10\r\n\r\n0123456789");
  REQUIRE( req->payload == NULL );
  httprequest_destroy(req);
  free(req);
}


TEST_CASE("httprequest_read - No Content-Length results in a NULL payload", "[weight=2][part=2]") {
  HTTPRequest *req = _readpipe("GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\n\r\n");

  CHECK( req->payload == NULL );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_read - ASCII Payload Data", "[weight=2][part=2]") {
  HTTPRequest *req = _readpipe("GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\nContent-Length: 10\r\n\r\n0123456789");

  REQUIRE( req->payload != NULL );
  CHECK( memcmp(req->payload, "0123456789", 10) == 0 );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_read - Payload Binary Payload", "[weight=4][part=2]") {
  HTTPRequest *req = _readpipe_vptr(
    "GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\nContent-Length: 23\r\n\r\n0123456789-\x00-0123456789",
    92
  );

  REQUIRE( req->payload != NULL );
  CAPTURE( ((char *)req->payload)[0] );
  CAPTURE( ((char *)req->payload)[9] );
  CAPTURE( ((char *)req->payload)[10] );
  CAPTURE( ((char *)req->payload)[11] );
  CAPTURE( ((char *)req->payload)[12] );
  CAPTURE( ((char *)req->payload)[13] );
  CAPTURE( ((char *)req->payload)[21] );
  CAPTURE( ((char *)req->payload)[22] );
  CHECK( memcmp(req->payload, "0123456789-\x00-0123456789", 23) == 0 );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_read - Arbitrary Small Binary Payload", "[weight=4][part=2]") {
  int Length = 1024;
  char* payload = malloc(Length);
  char* full_request_begin, *full_request;
  full_request_begin = malloc(1024);
  sprintf(full_request_begin, "GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\nContent-Length: %d\r\n\r\n", Length);
  full_request = malloc(strlen(full_request_begin) + Length + 1);

  // generate a random payload
  for (int i = 0; i < Length; i++) {
    payload[i] = rand() % 256;
  }
  // concatenate the payload and the full_request
  memcpy(full_request, full_request_begin, strlen(full_request_begin));
  memcpy(full_request + strlen(full_request_begin), payload, Length);
  full_request[(int)strlen(full_request_begin) + Length] = '\0';

  HTTPRequest *req = _readpipe_vptr(
    full_request,
    strlen(full_request_begin) + Length
  );

  REQUIRE( req->payload != NULL );
  CHECK( memcmp(req->payload, payload, Length) == 0 );

  free(payload);
  free(full_request_begin);
  free(full_request);
  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_read - Arbitrary Long Binary Payload", "[weight=4][part=2]") {
  int Length = 32 * 1024;
  char* payload = malloc(Length);
  char* full_request_begin, *full_request;
  full_request_begin = malloc(1024);
  sprintf(full_request_begin, "GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\nContent-Length: %d\r\n\r\n", Length);
  full_request = malloc(strlen(full_request_begin) + Length + 1);

  // generate a random payload
  for (int i = 0; i < Length; i++) {
    payload[i] = rand() % 256;
  }
  // concatenate the payload and the full_request
  memcpy(full_request, full_request_begin, strlen(full_request_begin));
  memcpy(full_request + strlen(full_request_begin), payload, Length);
  full_request[(int)strlen(full_request_begin) + Length] = '\0';
  HTTPRequest *req = _readpipe_vptr(
    full_request,
    strlen(full_request_begin) + Length
  );
  REQUIRE( req->payload != NULL );
  CHECK( memcmp(req->payload, payload, Length) == 0 );
  free(payload);
  free(full_request_begin);
  free(full_request);
  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_read - Binary Payload with HTTP Payload Delimiter", "[weight=5][part=2]") {
  int Length = 32 * 1024;
  char* payload = malloc(Length);
  char* full_request_begin, *full_request;
  full_request_begin = malloc(1024);
  sprintf(full_request_begin, "GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\nContent-Length: %d\r\n\r\n", Length);
  full_request = malloc(strlen(full_request_begin) + Length + 1);

  // generate a random payload
  for (int i = 0; i < Length; i++) {
    if (i == Length / 2) {
      payload[i++] = "\r";
      payload[i++] = "\n";
      payload[i++] = "\r";
      payload[i] = "\n";
      continue;
    }
    payload[i] = rand() % 256;
  }
  // concatenate the payload and the full_request
  memcpy(full_request, full_request_begin, strlen(full_request_begin));
  memcpy(full_request + strlen(full_request_begin), payload, Length);
  full_request[(int)strlen(full_request_begin) + Length] = '\0';
  HTTPRequest *req = _readpipe_vptr(
    full_request,
    strlen(full_request_begin) + Length
  );
  REQUIRE( req->payload != NULL );
  CHECK( memcmp(req->payload, payload, Length) == 0 );
  free(payload);
  free(full_request_begin);
  free(full_request);
  httprequest_destroy(req);
  free(req);
}

void payload_test(long size) {
  // Allocate memory for the string
  char* reqStr = (char*)malloc(size + 100);
  int offset = sprintf(reqStr, "GET / HTTP/1.1\r\nkey1: value1\r\nkey2: value2\r\nContent-Length: %d\r\n\r\n", size);

  // Fill the string with random characters
  for (long long i = 0; i < size; i++) {
      reqStr[i + offset] = (rand() % 26) + 'a';
  }

  HTTPRequest *req = _readpipe_vptr(reqStr, offset + size);

  const char *content_length = httprequest_get_header(req, "Content-Length");
  CAPTURE( content_length );
  REQUIRE( content_length != NULL );
  REQUIRE( atoi(content_length) == size );

  REQUIRE( req->payload != NULL );
  REQUIRE( memcmp(req->payload, reqStr + offset, size) == 0 );

  httprequest_destroy(req);

  free(req);
  free(reqStr);
}

TEST_CASE("httprequest_read - Small Payload (50 B)", "[weight=2][part=2]") {
  payload_test(50);
}

TEST_CASE("httprequest_read - Medium Payload (50 KiB)", "[weight=3][part=2]") {
  payload_test(50 * 1024);
}

TEST_CASE("httprequest_read - Large Payload (5 MiB)", "[weight=4][part=2]") {
  payload_test(5 * 1024 * 1024);
}
