HTTP-Client
===========

Minimalistic http client, somewhat imitating the [fetch][fetch] API, but for
C

This library makes use of [dep](https://github.com/finwo/dep) to manage it's
dependencies and exports.

Installation
------------

```sh
dep add finwo/httpc
dep install
```

After that, simply add `include lib/.dep/config.mk` in your makefile and include
the header file by adding `#include "finwo/httpc.h`.

Features
--------

- Familiar interface resembling the [fetch][fetch] API
- Basic http/1.1 requests
- Custom request support
- SSL support
- Following of redirects optional

Exports
-------

### Data types

<details>
  <summary>struct httpc_header</summary>

  Represents a header to be sent in the request

```C
struct httpc_header {
 const char *key;
 const char *value;
};
```

</details>
<details>
  <summary>struct httpc_fetch_options</summary>

  The options passed to the fetch method, declaring how you want the
  request to be made

```C
struct httpc_fetch_options {
 char                *method;
 struct buf          *body;
 struct httpc_header *headers;
 bool                 follow_redirects;
 bool                 compression;
};
```

</details>

Methods
-------

<details>
  <summary>struct http_parser_message * http_fetch(url, options)</summary>

  Performs a synchronous http(s) request based on the given parameters

```C
struct http_parser_message * httpc_fetch(const char *url, const struct httpc_fetch_options *options);
```


Note: see the [http-parser][http-parser] project for how the
http_parser_message struct is defined
</details>

Usage
-----

The API is designed mostly to be simple to use, not to have a crazy amount
of features, and is therefor somewhat opiniated.

For example, defining headers resembles declaring a null-terminated
key-value list:

```C
struct httpc_header headers[] = {
  { "Accept"    , "*"          },
  { "Connection", "close"      },
  { "User-Agent", "httpc/0.1"  },
  { NULL        , NULL         },
};
```

And subsequently, the call to the actual fetch method is somewhat lackluster
in terms of features as well:

```C
struct http_parser_message *response = httpc_fetch(
  "http://example.com",
  &(struct httpc_fetch_options){
    .follow_redirects = true,
    .headers          = headers,
  }
);
```

Serialization of data for the body of the request is not part of this
library either, so you'll have to generate that yourself, although that does
grant some flexibility in how things are passed along.

```C
// Generate the postbody buffer
const char *postBodyRaw = "{\"foo\":\"bar\"}";
struct buf *postBody = calloc(1, sizeof(struct buf));
buf_append(postBody, postBodyRaw, strlen(postBodyRaw));

// Generate request headers
struct httpc_header headers[] = {
  { "Accept"      , "application/json" },
  { "Connection"  , "close"            },
  { "User-Agent"  , "httpc/0.1"        },
  { "Content-Type", "application/json" },
  { NULL          , NULL               },
};

// And perform a post request
struct http_parser_message *response = httpc_fetch(
  "https://example.com",
  &(struct httpc_fetch_options){
    .method  = "POST",
    .headers = headers,
    .body    = postBody,
  }
);
```

And if you want to get the response, you'll have to decode or handle it
yourself:

```C
const struct buf *responseBody = response->body;

printf("\n--- BEGIN RESPONSE ---\n");
write(STDOUT, responseBody->data, responseBody->len);
printf("\n--- END RESPONSE ---\n");

http_parser_message_free(response);
```

[fetch]: https://developer.mozilla.org/en-US/docs/Web/API/fetch
[http-parser]: https://github.com/finwo/http-parser.c
