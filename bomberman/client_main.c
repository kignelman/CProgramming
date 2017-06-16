//
// Created by balbe on 28/05/2017.
//
#include "client.h"

void help() {

    printf("+------------------------------------------+\n");
    printf("+ Welcome to bomberman client command line +\n");
    printf("+                                          +\n");
    printf("+ For run game execute following command   +\n");
    printf("+                                          +\n");
    printf("+$> bomberman-client [host [port]]         +\n");
    printf("+------------------------------------------+\n");
}

int main(int args, char **argv) {

    int help_is_show = 0;
    start_socket();
    client_t client;
    SDL_Init(SDL_INIT_VIDEO);
    if (args == 2 && strcmp("help", argv[1]) == 0) {
        help();
        help_is_show = 1;
    } else if (args >= 3) {
        construct(&client, argv[1], atoi(argv[2]));
    } else {
        construct(&client, "127.0.0.1", SERVER_PORT);
    }
    if (!help_is_show) {
        run(&client);
    }
    SDL_Quit();
    cleanup_socket();

    return 0;
}