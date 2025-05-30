#ifndef HTTP_HEADERS
#define HTTP_HEADERS

typedef struct HttpHeaders HttpHeaders;

const char* headers_get(const HttpHeaders* headers, const char* key);
size_t headers_add(HttpHeaders *headers, const char* key, const char* value);

#endif // !HTTP_HEADERS
