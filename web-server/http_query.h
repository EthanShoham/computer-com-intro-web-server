#ifndef HTTP_QUERY
#define HTTP_QUERY
#include <stdint.h>

// list of key value pairs extracted from the url
typedef struct HttpQuery HttpQuery;

// the key is case sensetive, returns null if key not found,
const char *query_get(const HttpQuery *query, const char *key);
const char *query_get_key_at_index(HttpQuery *query, size_t index);
const char *query_get_value_at_index(HttpQuery *query, size_t index);
size_t query_count(HttpQuery *query);

#endif // !HTTP_QUERY
