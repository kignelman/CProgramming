//
// Created by balbe on 28/05/2017.
//

#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H
#include "socket_util.h"
#include <SDL2/SDL.h>
#if defined(WIN32)
#include <SDL2/SDL_Image.h>
#elif defined(linux)
#include <SDL2/SDL_image.h>
#endif

typedef struct client_s {
    int start;
    fd_set set;
    SOCKET socket;
    SDL_Surface * up_image;
    SDL_Surface * right_image;
    SDL_Surface * down_image;
    SDL_Surface * left_image;
    SDL_Surface * enemy_up_image;
    SDL_Surface * enemy_right_image;
    SDL_Surface * enemy_down_image;
    SDL_Surface * enemy_left_image;
    SDL_Surface * bomb_image;
    SDL_Surface * steel_image;
    SDL_Surface * brick_image;
    SDL_Surface * fire_image;
    SDL_Window *window;
} client_t;

/**
 *
 * @param client
 */
void construct(client_t *client);

/**
 * Ecoute les evenements du clavier.
 *
 * @param client
 */
void listen_key_event(client_t *client);

/**
 *
 * @param client
 */
void listen_receive_server(client_t *client);

/**
 *  Lorsque le client se deconnecte.
 * @param client
 */
void on_disconnect(client_t *client);

/**
 * Dessine la map.
 *
 * @param client
 * @param request
 */
void draw(client_t *client, server_request_t request);

/**
 *
 * @param client
 */
void run(client_t * client);

#endif //GAME_CLIENT_H
