#ifndef HTTP_QUERY
#define HTTP_QUERY

// list of key value pairs extracted from the url
typedef struct HttpQuery HttpQuery;

// the key is case sensetive, returns null if key not found,
const char* query_get(const HttpQuery* query, const char* key);

#endif // !HTTP_QUERY
