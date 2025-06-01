
#ifndef HTTP_QUERY_INTERNAL
#define HTTP_QUERY_INTERNAL
#include "../http_query.h"
#include <stdbool.h>

HttpQuery *create_http_query();
void destroy_http_query(HttpQuery *query);

bool query_add(HttpQuery *query, const char *key, const char *value);

#endif // !HTTP_QUERY
