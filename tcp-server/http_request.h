#ifndef HTTP_REQUEST
#define HTTP_REQUEST
#include <stdbool.h>
#include "http_method.h"
#include "http_query.h"
#include "http_headers.h"

typedef struct HttpRequest HttpRequest;

enum HttpMethod req_get_method(const HttpRequest* req);
const HttpQuery* req_get_query(const HttpRequest* req);
const HttpHeaders* req_get_headers(const HttpRequest* req);

bool req_can_read(const HttpRequest* req);
size_t req_read(HttpRequest* req, char* buffer, size_t length);

#endif
