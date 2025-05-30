#ifndef HTTP_RESPONSE
#define HTTP_RESPONSE
#include <stdbool.h>
#include "http_headers.h"
#include "http_status_code.h"

typedef struct HttpResponse HttpResponse;

void res_set_status_code(HttpResponse *res,enum HttpStatusCode code);
HttpHeaders* res_get_headers(const HttpResponse* res);
void res_finish_headers(HttpResponse* res);
void res_finish_write(HttpResponse* res);
void res_set_context(HttpResponse* res, const void* context);
void* res_get_context(const HttpResponse* res);

bool res_can_write(const HttpResponse* res);
size_t res_wirte(HttpResponse* res, const char* buffer, size_t length);

#endif // !HTTP_RESPONSE
