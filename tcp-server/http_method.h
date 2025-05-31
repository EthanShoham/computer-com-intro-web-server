#ifndef HTTP_METHOD
#define HTTP_METHOD

#define MAX_HTTP_METHOD_LENGTH 7
typedef enum HttpMethod {
  HTTP_GET,
  HTTP_PUT,
  HTTP_POST,
  HTTP_HEAD,
  HTTP_TRACE,
  HTTP_DELETE,
  HTTP_OPTIONS,
  HTTP_METHOD_LENGTH
} HttpMethod;

inline const char *http_method_name(HttpMethod method) {
  switch (method) {
  case HTTP_GET:
    return "GET";
  case HTTP_PUT:
    return "PUT";
  case HTTP_POST:
    return "POST";
  case HTTP_HEAD:
    return "HEAD";
  case HTTP_TRACE:
    return "TRACE";
  case HTTP_DELETE:
    return "DELETE";
  case HTTP_OPTIONS:
    return "OPTIONS";
  default:
    return 0;
  }
}

#endif // !HTTP_METHOD
