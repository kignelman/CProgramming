#include "socket_util.h"
#include "server.h"

int main() {
    start_socket();
    server_t server;
    construct(&server);
    run(&server);
    cleanup_socket();
    return 0;
}