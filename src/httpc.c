#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <unistd.h>

#include "finwo/http-parser.h"
#include "scyphus/url-parser.h"
#include "tidwall/buf.h"

#include "httpc.h"

#ifndef NULL
#define NULL (void*)0
#endif

char * _strtoupper(char *str) {
  char *ref = str;
  for(;*ref;++ref) {
    *ref=toupper((unsigned char)*ref);
  }
  return str;
}

struct httpc_response * httpc_fetch(const char *url, const struct httpc_fetch_options *options) {
  int i;
  struct parsed_url *parsed = parse_url(url);
  if (!parsed->path) parsed->path = strdup("/");

  /* printf("parsed->\n"); */
  /* printf("  scheme: %s\n", parsed->scheme); */
  /* printf("  host  : %s\n", parsed->host); */
  /* printf("  path  : %s\n", parsed->path); */
  /* printf("  query : %s\n", parsed->query); */
  /* return NULL; */

  struct http_parser_pair *reqres = http_parser_pair_init(NULL);
  reqres->request->version = strdup("1.1");
  reqres->request->method  = _strtoupper(strdup(options->method ? options->method : "GET"));
  reqres->request->path    = strdup(parsed->path);

  if (parsed->query) {
    reqres->request->path = strdup(parsed->query);
  }
  if (options->body) {
    reqres->request->body       = malloc(sizeof(struct buf));
    reqres->request->body->len  = options->body->len;
    reqres->request->body->cap  = options->body->cap;
    reqres->request->body->data = malloc(options->body->cap);
    memcpy(reqres->request->body->data, options->body->data, options->body->len);
  }

  if (options->headers != NULL) {
    for(i = 0; (&(options->headers[i]))->key != NULL ; i++) {
      http_parser_header_set(reqres->request, (&(options->headers[i]))->key, (&(options->headers[i]))->value);
    }
  }

  struct buf * dinges = http_parser_sprint_pair_request(reqres);
  write(1, dinges->data, dinges->len);

  http_parser_pair_free(reqres);
  parsed_url_free(parsed);
  return NULL;
}
