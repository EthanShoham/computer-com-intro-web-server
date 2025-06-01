#ifndef HTTP_HEADERS
#define HTTP_HEADERS
#include <stdbool.h>
#include <stdint.h>

typedef struct HttpHeaders HttpHeaders;

const char *headers_get(const HttpHeaders *headers, const char *key);
bool headers_add(HttpHeaders *headers, const char *key, const char *value);
size_t headers_count(HttpHeaders *headers);

#endif // !HTTP_HEADERS
