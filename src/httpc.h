#ifndef __FINWO_HTTPC_H__
#define __FINWO_HTTPC_H__

#include "tidwall/buf.h"

struct httpc_response {

};

struct httpc_header {
  const char *key;
  const char *value;
};

struct httpc_fetch_options {
  char                *method;
  struct buf          *body;
  struct httpc_header *headers;
};

struct httpc_response * httpc_fetch(const char *url, const struct httpc_fetch_options *options);

#endif // __FINWO_HTTPC_H__
