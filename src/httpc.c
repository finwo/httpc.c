#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "openssl/err.h"
#include "openssl/ssl.h"

#include "finwo/em_inflate.h"
#include "finwo/http-parser.h"
#include "finwo/strtoupper.h"
#include "scyphus/url-parser.h"
#include "tidwall/buf.h"

#include "httpc.h"

#define VERSION "0.1"

#ifndef NULL
#define NULL (void*)0
#endif

const char * schemeport(const char *scheme) {
  if (!strcmp(scheme, "http" )) return "80";
  if (!strcmp(scheme, "https")) return "443";
  return 0;
}

SSL_CTX* InitCTX(void) {
  return SSL_CTX_new(TLS_method());
  /* OpenSSL_add_all_algorithms();  /1* Load cryptos, et.al. *1/ */
  /* SSL_load_error_strings();   /1* Bring in and register error messages *1/ */
  /* SSL_CTX *ctx = SSL_CTX_new(TLS_method()); */
  /* if (!ctx) { */
  /*   ERR_print_errors_fp(stderr); */
  /*   exit(1); */
  /* } */
  /* return ctx; */
}

struct http_parser_message * _httpc_fetch(const char *url, const struct httpc_fetch_options *options) {
  int n, i, sockfd;
  struct parsed_url *parsed = parse_url(url);
  if (!parsed->scheme) parsed->scheme = strdup("http");
  if (!parsed->path  ) parsed->path   = strdup("/");
  if (!parsed->port  ) parsed->port   = strdup(schemeport(parsed->scheme));

  struct httpc_fetch_options _opts = {};
  if (options) memcpy(&_opts, options, sizeof(_opts));

  struct http_parser_pair *reqres = http_parser_pair_init(NULL);
  reqres->request->version = strdup("1.1");
  reqres->request->method  = strtoupper(strdup(_opts.method ? _opts.method : "GET"));
  reqres->request->path    = strdup(parsed->path);

  // printf("%s %s\n", reqres->request->method, url);

  if (parsed->query) {
    reqres->request->path = strdup(parsed->query);
  }
  if (_opts.body) {
    reqres->request->body       = malloc(sizeof(struct buf));
    reqres->request->body->len  = _opts.body->len;
    reqres->request->body->cap  = _opts.body->cap;
    reqres->request->body->data = malloc(_opts.body->cap);
    memcpy(reqres->request->body->data, _opts.body->data, _opts.body->len);
  }

  // Default headers (mutable)
  http_parser_header_set(reqres->request, "Host", parsed->host);
  http_parser_header_set(reqres->request, "User-Agent", "httpc/" VERSION);

  if (_opts.headers != NULL) {
    for(i = 0; (&(_opts.headers[i]))->key != NULL ; i++) {
      http_parser_header_set(reqres->request, (&(_opts.headers[i]))->key, (&(_opts.headers[i]))->value);
    }
  }

  // Forced headers (unmutable)
  http_parser_header_set(reqres->request, "Connection", "close");

  // Deterministic headers
  if (_opts.compression) {
    http_parser_header_set(reqres->request, "Accept-Encoding", "gzip, deflate");
  }

  struct buf *sndBuff = http_parser_sprint_pair_request(reqres);
  char rcvBuff[1024];

  // Get address of host
  struct addrinfo *res = NULL;
  struct addrinfo *ref = NULL;
  // char str[INET6_ADDRSTRLEN];
  getaddrinfo(parsed->host, parsed->port, 0, &res);
  ref = res;
  for( ref = res ; ref != NULL ; ref = ref->ai_next ) {
    // Here: loop through available addresses, try them all
    if (ref->ai_socktype != SOCK_STREAM) continue;

    /* // Debug */
    /* if (ref->ai_addr->sa_family == AF_INET) { */
    /*     struct sockaddr_in *p = (struct sockaddr_in *)ref->ai_addr; */
    /*     printf("ipv4: %s\n", inet_ntop(AF_INET, &p->sin_addr, str, sizeof(str))); */
    /* } else if (ref->ai_addr->sa_family == AF_INET6) { */
    /*     struct sockaddr_in6 *p = (struct sockaddr_in6 *)ref->ai_addr; */
    /*     printf("ipv6: %s\n", inet_ntop(AF_INET6, &p->sin6_addr, str, sizeof(str))); */
    /* } */

    // Setup connection to the host
    sockfd = socket(ref->ai_family, ref->ai_socktype, ref->ai_protocol);
    if (sockfd < 0) continue;
    if (connect(sockfd, ref->ai_addr, ref->ai_addrlen) < 0) {
      close(sockfd);
      continue;
    }

    // Handle https
    if (!strcmp(parsed->scheme, "https")) {
      SSL_CTX *ctx = InitCTX();
      SSL *ssl = SSL_new(ctx);
      SSL_set_fd(ssl, sockfd);
      SSL_set_tlsext_host_name(ssl, parsed->host);
      if (SSL_connect(ssl) < 0) {
        ERR_print_errors_fp(stderr);
        exit(1);
      }
      n = SSL_write(ssl, sndBuff->data, sndBuff->len);
      while((n = SSL_read(ssl, rcvBuff, sizeof(rcvBuff)))) {
        http_parser_pair_response_data(reqres, &((struct buf){
          .data = rcvBuff,
          .len  = n,
          .cap  = n
        }));
        if (reqres->response->ready) break;
      }
      SSL_free(ssl);
      SSL_CTX_free(ctx);
      close(sockfd);
      break;
    }

    // TODO: merge (non-)ssl reader/writer

    // Handle bare http
    n = write(sockfd, sndBuff->data, sndBuff->len);
    while((n = read(sockfd, rcvBuff, sizeof(rcvBuff)))) {
      http_parser_pair_response_data(reqres, &((struct buf){
        .data = rcvBuff,
        .len  = n,
        .cap  = n
      }));
      if (reqres->response->ready) break;
    }
    close(sockfd);
    break;

  }

  // No longer using received addresses
  freeaddrinfo(res);

  // Keep the response intact
  struct http_parser_message *resp = reqres->response;
  reqres->response = NULL;

  // Free used memory
  http_parser_pair_free(reqres);
  parsed_url_free(parsed);
  buf_clear(sndBuff);
  free(sndBuff);

  // Handle decompression
  if (_opts.compression) {
    const char *encoding = http_parser_header_get(resp, "content-encoding");

    // em_inflate supports both gzip & deflate
    if (encoding) {
      if ((!strcmp(encoding, "gzip")) || (!strcmp(encoding, "deflate"))) {
        struct buf *inflatedBody = em_inflate(resp->body);

        if (inflatedBody) {
          buf_clear(resp->body);
          free(resp->body);
          resp->body = inflatedBody;
        }
      }
    }

    // TODO: brotli?
  }

  return resp;
}

struct http_parser_message * httpc_fetch(const char *url, const struct httpc_fetch_options *options) {
  char * _url = strdup(url);
  struct httpc_fetch_options *opts = (struct httpc_fetch_options *)options;
  struct http_parser_message *response;

  do {
    response = _httpc_fetch(_url, opts);

    if (options->follow_redirects) {
      switch(response->status) {
        case 301:
        case 302:
        case 303:
          /* printf(": %s\n", http_parser_header_get(response, "location")); */
          free(_url);
          _url = strdup(http_parser_header_get(response, "location"));
          http_parser_message_free(response);
          opts = NULL;
          /* printf("fin\n"); */
          continue;
        case 307:
        case 308:
          /* printf(": %s\n", http_parser_header_get(response, "location")); */
          free(_url);
          _url = strdup(http_parser_header_get(response, "location"));
          http_parser_message_free(response);
          /* printf("fin\n"); */
          continue;
        default:
          /* printf("\n"); */
          break;
      }
    } else {
      /* printf("\n"); */
    }

    break;
  } while(1);

  return response;
}
