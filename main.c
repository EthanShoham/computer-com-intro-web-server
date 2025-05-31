#include "web-server/web_server.h"

int main() {
    WebServer *server = create_web_server();
    web_server_map_get("/", );
    web_server_map_get("/hello",);
    return web_server_run(server);
}
