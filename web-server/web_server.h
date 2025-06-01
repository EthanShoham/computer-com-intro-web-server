#ifndef WEB_SERVER
#define WEB_SERVER
#include "http_request.h"
#include "http_response.h"

typedef struct WebServer WebServer;

WebServer *create_web_server();
void web_server_map_get(WebServer *server, const char *route,
                        void (*func)(HttpRequest *req, HttpResponse *res),
                        void (*cleanup)(void *context));
void web_server_map_post(WebServer *server, const char *route,
                         void (*func)(HttpRequest *req, HttpResponse *res),
                         void (*cleanup)(void *context));
void web_server_map_put(WebServer *server, const char *route,
                        void (*func)(HttpRequest *req, HttpResponse *res),
                        void (*cleanup)(void *context));
void web_server_map_delete(WebServer *server, const char *route,
                           void (*func)(HttpRequest *req, HttpResponse *res),
                           void (*cleanup)(void *context));
int web_server_run(WebServer *server);

#endif // !WEB_SERVER
