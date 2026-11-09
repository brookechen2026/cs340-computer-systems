#include "lib/catch.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "../http.h"

HTTPRequest *_parse_vptr(const void *s, size_t len) {
  char *buf = (char *) malloc(len + 1);
  memcpy(buf, s, len);
  buf[len] = 0;

  HTTPRequest *req = (HTTPRequest *) malloc(sizeof(HTTPRequest));
  httprequest_parse_headers(req, buf, len);
  free(buf);
  return req;
}

HTTPRequest *_parse(const char *s) {
  return _parse_vptr(s, strlen(s));
}


TEST_CASE("httprequest_parse_headers - Request Line", "[weight=3][part=1]") {
  HTTPRequest *req = _parse("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");

  REQUIRE( req->action != NULL );
  CHECK( strcmp(req->action, "GET") == 0 );

  REQUIRE( req->path != NULL );
  CHECK( strcmp(req->path, "/") == 0 );

  REQUIRE( req->version != NULL );
  CHECK( strcmp(req->version, "HTTP/1.1") == 0 );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_parse_headers - No Header", "[weight=3][part=1]") {
  HTTPRequest *req = _parse("GET / HTTP/1.1\r\n\r\n\r\n");

  const char *host = httprequest_get_header(req, "");
  CAPTURE( host );
  REQUIRE( host == NULL );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_parse_headers - First Header", "[weight=3][part=1]") {
  HTTPRequest *req = _parse("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");

  const char *host = httprequest_get_header(req, "Host");
  CAPTURE( host );
  REQUIRE( host != NULL );
  CHECK( strcmp(host, "localhost") == 0 );

  httprequest_destroy(req);
  free(req);
}

TEST_CASE("httprequest_parse_headers - Two Headers", "[weight=3][part=1]") {
  HTTPRequest *req = _parse("GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\n\r\n");

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

TEST_CASE("httprequest_parse_headers - Content-Length Header", "[weight=3][part=1]") {
  HTTPRequest *req = _parse("GET / HTTP/1.1\r\nHello: World\r\nHost: 127.0.0.1\r\nContent-Length: 10\r\n\r\n0123456789");

  const char *content_length = httprequest_get_header(req, "Content-Length");
  CAPTURE( content_length );
  REQUIRE( content_length != NULL );
  CHECK( strcmp(content_length, "10") == 0 );

  httprequest_destroy(req);
  free(req);
}


TEST_CASE("httprequest_parse_headers - N Headers", "[weight=5][part=1]") {
  std::string headers = "GET / HTTP/1.1\r\n";
  const int n = 10;
  for(int i = 0; i < n; i++) {
    headers += "Header" + std::to_string(i+1) + ": Value" + std::to_string(i+1) + "\r\n";
  }
  headers += "\r\n";

  HTTPRequest *req = _parse(headers.c_str());

  for(int i = 0; i < n; i++) {
    std::string header_name = "Header" + std::to_string(i+1);
    std::string header_value = "Value" + std::to_string(i+1);

    const char *header = httprequest_get_header(req, header_name.c_str());
    CAPTURE(header_name, header_value);
    REQUIRE(header != NULL);
    CHECK(strcmp(header, header_value.c_str()) == 0);
  }

  httprequest_destroy(req);
  free(req);
}
