//
// Created by balbe on 28/05/2017.
//

#include "server.h"

int load_map(server_t *server, char *filename) {
    int row, col;
    int c;
    FILE *file = NULL;

    file = fopen(filename, "r");
    row = 0;
    col = 0;
    while (file != NULL) {
        c = fgetc(file);
        if (col >= MAP_WIDTH) {
            col = 0;
            row++;
        }
        if (feof(file) || row >= MAP_HEIGHT)
            break;

        if (c == 'b') {
            server->map[row][col][0] = MAP_BRICK;
            server->map[row][col++][1] = 0;
        } else if (c == 's') {
            server->map[row][col][0] = MAP_STEEL;
            server->map[row][col++][1] = 0;
        } else if (c == ' ') {
            server->map[row][col][0] = MAP_PATH;
            server->map[row][col++][1] = 0;
        } else if (c != '\n' && c != '\r')
            break;
    }

    fclose(file);
    return row == MAP_HEIGHT && col == 0;
}

/**
 * Permet d'initialiser les information du serveur.
 *
 * @param server
 */
void construct(server_t *server, int port) {

    int i, j;
    server->socket = create_server("0.0.0.0", port);

    server->start = 0;

    for (i = 0; i < MAX_CLIENTS; i++) {
        server->info[i][INFO_ALIVE] = 0;
        server->info[i][INFO_NEXT_DIRECTION] = -1;
        server->info[i][INFO_DIRECTION] = -1;
        server->clients[i] = INVALID_SOCKET;
    }

    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            server->map[i][j][0] = MAP_PATH;
            server->map[i][j][1] = 0;
        }
    }

    server->updated = 1;
#if  defined(WIN32)
    load_map(server, "map.txt");
#elif defined(linux) || defined(APPLE) || defined(__APPLE__)
    load_map(server, "/usr/share/bomberman/map.txt");
#endif
}

/**
 *
 * Verification de la validite d'une position dans la map.
 *
 * @param i
 * @param j
 * @return
 */
int is_map_coordinate(int i, int j) {
    return i >= 0 && i < MAP_HEIGHT && j >= 0 && j < MAP_WIDTH;
}

/**
 * Verification de la disponibilité d'une position dans la map.
 *
 * @param server
 * @param x
 * @param y
 * @return
 */
int can_move(server_t *server, int x, int y) {
    int i;

    if (!is_map_coordinate(x, y))
        return 0;

    switch (server->map[x][y][0]) {
        case MAP_BOMB:
        case MAP_BRICK:
        case MAP_STEEL:
            return 0;
        default:
            break;
    }

    for (i = 0; i < MAX_CLIENTS; i++)
        if (server->clients[i] != INVALID_SOCKET
            && server->info[i][INFO_X] == x
            && server->info[i][INFO_Y] == y) {
            return 0;
        }

    return (1);
}

/**
 * Envoi de la map au client.
 *
 * @param server
 * @param position
 */
void send_map(server_t *server, int position) {

    int i, j, k;
    server_request_t request;

    if (server->clients[position] == INVALID_SOCKET)
        return;

    request.alive = server->info[position][INFO_ALIVE];
    request.position[0] = server->info[position][INFO_X];
    request.position[1] = server->info[position][INFO_Y];

    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            for (k = 0; k < 2; k++) {
                request.map[i][j][k] = server->map[i][j][k];
                request.clients[i][j][k] = 0;
            }
        }
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i] != INVALID_SOCKET
            && server->info[i][INFO_ALIVE]
            && is_map_coordinate(server->info[i][INFO_X], server->info[i][INFO_Y])) {
            request.clients[server->info[i][INFO_X]][server->info[i][INFO_Y]][0] = 1;
            request.clients[server->info[i][INFO_X]][server->info[i][INFO_Y]][1] = server->info[i][INFO_DIRECTION];
        }
    }

    request.protocol = GP_UPDATE;

    int sending = send_server_request(server->clients[position], &request);
    if (sending) {
        printf("send to %d : (%d - %d) -> Alive %d\n", server->clients[position], server->info[position][INFO_X],
               server->info[position][INFO_Y], server->info[position][INFO_ALIVE]);
    } else {
        printf("can't send to %d\n", server->clients[position]);
    }
    fflush(stdout);
}

/**
 *
 * @param server
 * @param x
 * @param y
 * @return
 */
int is_free(server_t *server, int x, int y) {
    int i;

    if (!(is_map_coordinate(x, y) && server->map[x][y][0] == MAP_PATH))
        return (0);

    for (i = 0; i < MAX_CLIENTS; i++)
        if (server->clients[i] != INVALID_SOCKET
            && server->info[i][INFO_X] == x
            && server->info[i][INFO_Y] == y)
            return (0);

    return (1);
}

/**
 *
 * @param server
 */
int reset_fd_set(server_t *server) {
    int i, m;
    FD_ZERO(&(server->set));
    FD_SET(server->socket, &(server->set));
    m = server->socket;

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i] != INVALID_SOCKET) {
            FD_SET(server->clients[i], &(server->set));
            if (m < server->clients[i])
                m = server->clients[i];
        }
    }
    return m;
}

/**
 * Lorsque le client envoi une requete.
 *
 * @param position
 * @param request
 * @param server
 */
void on_request(server_t *server, int position, client_request_t request) {
    if (request.protocol == GP_MOVE) {
        printf("Client %d wan't move\n", server->clients[position]);
        on_move(server, position, request.value);
    } else if (request.protocol == GP_BOMB){
        printf("Client %d wan't place bomb\n", server->clients[position]);
        on_place_bomb(server, position);
    }
}

/**
 * Lorsqu'un nouveau client se connecte.
 *
 * @param client
 * @param server
 * @return
 */
int on_connect(server_t *server, SOCKET client) {
    int i, j, k;

    printf("Connect()\n");
    fflush(stdout);
    server->updated = 1;

    for (i = 0; i < MAX_CLIENTS; i++)
        if (server->clients[i] == INVALID_SOCKET) {
            server->clients[i] = client;
            server->info[i][INFO_ALIVE] = 1;
            server->info[i][INFO_DIRECTION] = DOWN;
            for (j = 0; j < MAP_HEIGHT; j++)
                for (k = 0; k < MAP_WIDTH; k++)
                    if (is_free(server, j, k)) {
                        server->info[i][INFO_X] = j;
                        server->info[i][INFO_Y] = k;
                        j = MAP_HEIGHT;
                        break;
                    }
            break;
        }
    return 0;
}

/**
 * Lorsqu'un client se deconnecte.
 *
 * @param position
 * @param server
 * @return
 */
int on_disconnect(server_t *server, int position) {

    if (server->clients[position] != INVALID_SOCKET) {
        printf("client %d is disconnect\n", server->clients[position]);
        closesocket(server->clients[position]);
        fflush(stdout);
    }

    server->clients[position] = INVALID_SOCKET;
    server->info[position][INFO_ALIVE] = 0;
    server->info[position][INFO_X] = -1;
    server->info[position][INFO_Y] = -1;
    server->info[position][INFO_DIRECTION] = -1;
    server->info[position][INFO_NEXT_DIRECTION] = -1;
    server->updated = 1;

    return 1;
}

/**
 * Lorsque le client se deplace.
 *
 * @param server
 * @param position
 * @param direction
 * @return
 */
int on_move(server_t *server, int position, int direction) {
    if (!(server->clients[position] != INVALID_SOCKET && server->info[position][INFO_ALIVE]))
        return 0;

    switch (direction) {
        case UP:
        case RIGHT:
        case DOWN:
        case LEFT:
            server->info[position][INFO_NEXT_DIRECTION] = direction;
            return 1;
        default:
            break;
    }
    return 0;
}

/**
 * Lorsque le client veut poser une bombe.
 *
 * @param server
 * @param position
 * @param direction
 * @return
 */
int on_place_bomb(server_t *server, int position) {
    if (!(server->clients[position] != INVALID_SOCKET && server->info[position][INFO_ALIVE]))
        return 0;

    server->updated = 1;
    server->map[server->info[position][INFO_X]][server->info[position][INFO_Y]][0] = MAP_BOMB;
    server->map[server->info[position][INFO_X]][server->info[position][INFO_Y]][1] = 20;

    return 1;
}

/**
 *
 * Ecoute des nouvelles connections.
 *
 * @param server
 */
void listen_accept_client(server_t *server) {
    socklen_t len;
    struct sockaddr_in sin;

    if (FD_ISSET(server->socket, &(server->set))) {
        len = sizeof(sin);
        SOCKET client = accept(server->socket, (struct sockaddr *) &sin, &len);
        if (client != INVALID_SOCKET) {
            on_connect(server, client);
        }
    }
}

/**
 * Ecoute des requetes envoyer par les clients.
 *
 * @param server
 * @param position
 */
void listen_receive_client(server_t *server, int position) {

    int reading = 0;

    client_request_t client_request;

    if (!(server->clients[position] != INVALID_SOCKET && FD_ISSET(server->clients[position], &(server->set)))) {
        return;
    }

    reading = recv_client_request(server->clients[position], &client_request);

    if (reading > 0) {
        on_request(server, position, client_request);
    } else {
        on_disconnect(server, position);
    }
}


/**
 * Mettre le feu a une position.
 *
 * @param server
 * @param i
 * @param j
 */
void make_fire(server_t *server, int i, int j) {
    if (!is_map_coordinate(i, j)) {
        return;
    }

    server->map[i][j][0] = MAP_FIRE;
    server->map[i][j][1] = 5;
}

/**
 * Explosion d'une bombe.
 *
 * @param server
 * @param i
 * @param j
 */
void explode_bomb(server_t *server, int i, int j) {
    int fire_size = 3, k;
    make_fire(server, i, j);
    server->updated = 1;

    for (k = 1; k < fire_size; k++) {
        if (is_map_coordinate(i + k, j)) {
            switch (server->map[i + k][j][0]) {
                case MAP_PATH:
                case MAP_FIRE:
                    make_fire(server, i + k, j);
                    break;
                case MAP_BRICK:
                    make_fire(server, i + k, j);
                    k = fire_size;
                    break;
                case MAP_STEEL:
                    k = fire_size;
                    break;
                case MAP_BOMB:
                    explode_bomb(server, i + k, j);
                    break;
                default:
                    break;
            }
        }
    }

    for (k = 1; k < fire_size; k++) {
        if (is_map_coordinate(i - k, j)) {
            switch (server->map[i - k][j][0]) {
                case MAP_PATH:
                case MAP_FIRE:
                    make_fire(server, i - k, j);
                    break;
                case MAP_BRICK:
                    make_fire(server, i - k, j);
                    k = fire_size;
                    break;
                case MAP_STEEL:
                    k = fire_size;
                    break;
                case MAP_BOMB:
                    explode_bomb(server, i - k, j);
                    break;
                default:
                    break;
            }
        }
    }

    for (k = 1; k < fire_size; k++) {
        if (is_map_coordinate(i, j + k)) {
            switch (server->map[i][j + k][0]) {
                case MAP_PATH:
                case MAP_FIRE:
                    make_fire(server, i, j + k);
                    break;
                case MAP_BRICK:
                    make_fire(server, i, j + k);
                    k = fire_size;
                    break;
                case MAP_STEEL:
                    k = fire_size;
                    break;
                case MAP_BOMB:
                    explode_bomb(server, i, j + k);
                    break;
                default:
                    break;
            }
        }
    }

    for (k = 1; k < fire_size; k++) {
        if (is_map_coordinate(i, j + k)) {
            switch (server->map[i][j - k][0]) {
                case MAP_PATH:
                case MAP_FIRE:
                    make_fire(server, i, j - k);
                    break;
                case MAP_BRICK:
                    make_fire(server, i, j - k);
                    k = fire_size;
                    break;
                case MAP_STEEL:
                    k = fire_size;
                    break;
                case MAP_BOMB:
                    explode_bomb(server, i, j - k);
                    break;
                default:
                    break;
            }
        }
    }
}

/**
 * Mise a jour bombes.
 *
 * @param server
 */
void update_bombs(server_t *server) {

    int i, j;

    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            if (server->map[i][j][0] != MAP_BOMB)
                continue;
            server->map[i][j][1] = server->map[i][j][1] - 1;
            if (server->map[i][j][1] <= 0)
                explode_bomb(server, i, j);
        }
    }
}

/**
 * Mise a jour des feux.
 *
 * @param server
 */
void update_fires(server_t *server) {
    int i, j;

    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            if (server->map[i][j][0] != MAP_FIRE)
                continue;
            server->map[i][j][1] = server->map[i][j][1] - 1;
            if (server->map[i][j][1] <= 0) {
                server->map[i][j][0] = MAP_PATH;
                server->map[i][j][1] = 0;
                server->updated = 1;
            }
        }
    }
}

/**
 * Mise a jour de la vie des joueurs.
 *
 * @param server
 */
void update_alive(server_t *server) {
    int i;

    server_request_t request;

    for (i = 0; i < MAX_CLIENTS; i++)
        if (server->clients[i] != INVALID_SOCKET && server->info[i][INFO_ALIVE]
            && is_map_coordinate(server->info[i][INFO_X], server->info[i][INFO_Y])
            && server->map[server->info[i][INFO_X]][server->info[i][INFO_Y]][0] == MAP_FIRE) {
            server->info[i][INFO_ALIVE] = 0;
            server->info[i][INFO_X] = -1;
            server->info[i][INFO_Y] = -1;
            request.protocol = GP_GAME_OVER;
            server->updated = 1;
            send_server_request(server->clients[i], &request);
        }
}

/**
 * Mise a jour de la direction des joueurs.
 *
 * @param server
 */
void update_directions(server_t *server) {

    int i;

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i] != INVALID_SOCKET && server->info[i][INFO_ALIVE]) {
            switch (server->info[i][INFO_NEXT_DIRECTION]) {
                case UP:
                    if (can_move(server, server->info[i][INFO_X] - 1, server->info[i][INFO_Y])) {
                        server->info[i][INFO_X]--;
                        server->info[i][INFO_DIRECTION] = UP;
                        server->updated = 1;
                    }
                    break;
                case DOWN:
                    if (can_move(server, server->info[i][INFO_X] + 1, server->info[i][INFO_Y])) {
                        server->info[i][INFO_X]++;
                        server->info[i][INFO_DIRECTION] = DOWN;
                        server->updated = 1;
                    }
                    break;
                case LEFT:
                    if (can_move(server, server->info[i][INFO_X], server->info[i][INFO_Y] - 1)) {
                        server->info[i][INFO_Y]--;
                        server->info[i][INFO_DIRECTION] = LEFT;
                        server->updated = 1;
                    }
                    break;
                case RIGHT:
                    if (can_move(server, server->info[i][INFO_X], server->info[i][INFO_Y] + 1)) {
                        server->info[i][INFO_Y]++;
                        server->info[i][INFO_DIRECTION] = RIGHT;
                        server->updated = 1;
                    }
                    break;
                default:
                    break;
            }
        }

        server->info[i][INFO_NEXT_DIRECTION] = -1;
    }
}

/**
 * Mise a jours des informations du serveur.
 *
 * @param server
 */
void update(server_t *server) {
    int i;
    update_bombs(server);
    update_fires(server);
    update_alive(server);
    update_directions(server);

    if (server->updated) {
        for (i = 0; i < MAX_CLIENTS; i++)
            send_map(server, i);

        server->updated = 0;
    }
}

/**
 *
 * Demarre le server.
 *
 * @param server
 */
void run(server_t *server) {

    int i, t = 0, reading;
    struct timeval tv;
    server->start = 1;
    printf("Server is running ...\n");

    while (server->start && server->socket != INVALID_SOCKET) {
        tv.tv_sec = 0;
        tv.tv_usec = 10;
        reading = select(reset_fd_set(server) + 1, &(server->set), NULL, NULL, &tv);
        if (reading == 0) {
            if (t == 0)
                update(server);
            t = (t + 1) % 200;
        } else if (reading > 0) {
            listen_accept_client(server);
            for (i = 0; i < MAX_CLIENTS; i++)
                listen_receive_client(server, i);
        }
    }

    for (i = 0; i < MAX_CLIENTS; i++)
        on_disconnect(server, i);

    if (server->socket != INVALID_SOCKET)
        closesocket(server->socket);

    printf("Server is shutdown ...\n");
}