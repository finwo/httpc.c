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

[fetch]: https://developer.mozilla.org/en-US/docs/Web/API/fetch
[http-parser]: https://github.com/finwo/http-parser.c
