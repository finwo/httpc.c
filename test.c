#include "src/httpc.h"

#ifndef NULL
#define NULL (void*)0
#endif

int main() {
  struct httpc_header headers[] = {
    { "Accept", "*" },
    { "Content-Type", "application/octet-stream" },
    { "User-Agent", "httpc/0.1" },
    { NULL, NULL },
  };
  httpc_fetch("http://finwo.nl", &(struct httpc_fetch_options){
      .method = "GET",
      .headers = headers,
  });

  return 43;
}
