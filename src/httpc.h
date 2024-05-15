#ifndef __FINWO_HTTPC_H__
#define __FINWO_HTTPC_H__

struct httpc_response {

};

struct httpc_fetch_options {

};

struct httpc_response * httpc_fetch(const char *url, const struct httpc_fetch_options *options);

#endif // __FINWO_HTTPC_H__
