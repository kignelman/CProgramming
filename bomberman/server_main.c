#include "socket.h"
#include "server.h"

void help() {
    printf("+------------------------------------------+\n");
    printf("+ Welcome to bomberman server command line +\n ");
    printf("+                                          +\n");
    printf("+ For run server execute following command +\n");
    printf("+                                          +\n");
    printf("+$> bomberman-server [port]                +\n");
    printf("+------------------------------------------+\n");
}

int main(int args, char **argv) {
    start_socket();
    server_t server;

    if (args >= 2) {
        if (strcmp("help", argv[1]) == 0) {
            help();
        } else {
            construct(&server, atoi(argv[1]));
            run(&server);
        }
    } else {
        construct(&server, SERVER_PORT);
        run(&server);
    }
    cleanup_socket();
    return 0;
}
