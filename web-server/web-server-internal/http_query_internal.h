
#ifndef HTTP_QUERY_INTERNAL
#define HTTP_QUERY_INTERNAL
#include <stdbool.h>
#include "../http_query.h"

HttpQuery* create_http_query();
void destroy_http_query(HttpQuery* query);

bool query_add(HttpQuery* query, const char* key, const char* value);

#endif // !HTTP_QUERY
