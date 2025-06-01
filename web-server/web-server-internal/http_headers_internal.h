#ifndef HTTP_HEADERS_INTERNAL
#define HTTP_HEADERS_INTERNAL
#include "../http_headers.h"

HttpHeaders *create_http_headers();
void destroy_http_headers(HttpHeaders *headers);

const char *headers_get_key_at_index(HttpHeaders *headers, size_t index);
const char *headers_get_value_at_index(HttpHeaders *headers, size_t index);

#endif // !HTTP_HEADERS
