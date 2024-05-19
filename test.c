#include <stdio.h>

#include "lib/finwo/http-parser/src/http-parser.h"
#include "lib/tidwall/buf/buf.h"
#include "src/httpc.h"

#ifndef NULL
#define NULL (void*)0
#endif

int main() {

  struct httpc_header headers[] = {
    { "Accept"      , "*"          },
    { "Connection"  , "close"      },
    // { "Content-Type", "text/plain" },
    { "User-Agent"  , "httpc/0.1"  },
    { NULL          , NULL         },
  };

  struct http_parser_message *response = httpc_fetch("http://finwo.nl", &(struct httpc_fetch_options){
      // .method = "POST",
      .follow_redirects = true,
      .headers = headers,
      // .body    = &(struct buf){
      //   .cap = 12,
      //   .len = 11,
      //   .data = "Hello World",
      // },
  });

  printf("---BEGIN---\n%*s\n---END---\n", (int)(response->body->len), response->body->data);
  /* struct buf *responseBuff = http_parser_sprint_response(response); */
  /* printf("---BEGIN---\n%*s\n---END---\n", (int)(responseBuff->len), responseBuff->data); */
  http_parser_message_free(response);
  /* buf_clear(responseBuff); */
  /* free(responseBuff); */


  return 0;
}
