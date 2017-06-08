//
// Created by balbe on 28/05/2017.
//
#include "client.h"

int main(int args, char **argv) {

    start_socket();
    client_t client;
    SDL_Init(SDL_INIT_VIDEO);
    construct(&client);
    run(&client);
    SDL_Quit();
    cleanup_socket();

    return 0;
}