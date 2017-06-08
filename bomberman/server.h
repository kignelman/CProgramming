//
// Created by balbe on 28/05/2017.
//

#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "socket_util.h"
#define INFO_ALIVE 0
#define INFO_X 1
#define INFO_Y 2
#define INFO_DIRECTION 3
#define INFO_NEXT_DIRECTION 4

typedef struct server_s {
    int start;
    fd_set set;
    SOCKET socket;
    SOCKET clients[MAX_CLIENTS];
    int info[MAX_CLIENTS][5];
    int map[MAP_HEIGHT][MAP_WIDTH][2];
} server_t;

/**
 * Permet de creer le server.
 *
 * @param server
 */
void construct(server_t *server);

/**
 *
 * @param i
 * @param j
 * @return
 */
int is_map_coordinate(int i, int j);

/**
 *
 * @param server
 * @param x
 * @param y
 * @return
 */
int is_free(server_t *server,  int x, int y);


/**
 *
 * @param server
 */
int reset_fd_set(server_t *server);

/**
 * Lorsque le client envoi une requete.
 *
 * @param position
 * @param request
 * @param server
 */
void on_request(server_t * server, int position, client_request_t request);

/**
 *
 * @param client
 * @param server
 * @return
 */
int on_connect(server_t *server, SOCKET client);

/**
 * Lorsqu'un client se deconnecte.
 *
 * @param position
 * @param server
 * @return
 */
int on_disconnect(server_t *server, int position);

/**
 * Lorsque le client se deplace.
 *
 * @param server
 * @param position
 * @param direction
 * @return
 */
int on_move(server_t * server, int position, int direction);

/**
 * Lorsque le client veut poser une bombe.
 *
 * @param server
 * @param position
 * @param direction
 * @return
 */
int on_place_bomb(server_t * server, int position);

/**
 *
 * @param server
 */
void listen_accept_client(server_t *server);

/**
 *
 * @param server
 */
void listen_receive_client(server_t *server, int position);

/**
 *
 * @param server
 * @param i
 * @param j
 */
void explode_bomb(server_t *server, int i, int j);

/**
 *
 * @param i
 * @param j
 * @return
 */
int is_map_coordinate(int i, int j);

/**
 * Mettre a jour le server
 *
 * @param server
 */
void update(server_t *server);

/**
 *
 * Demarre le serveur.
 *
 * @param server
 */
void run(server_t * server);

#endif //GAME_SERVER_H
