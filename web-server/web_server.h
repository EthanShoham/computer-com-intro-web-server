#ifndef WEB_SERVER
#define WEB_SERVER
#include "http_request.h"
#include "http_response.h"

typedef struct WebServer WebServer;


WebServer* create_web_server();
void web_server_map_get(const char* route, void (*func)(HttpRequest* req, HttpResponse *res));
int web_server_run(WebServer* server);

#endif // !WEB_SERVER
