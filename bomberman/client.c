//
// Created by balbe on 28/05/2017.
//

#include "client.h"

void construct(client_t *client) {
    client->socket = create_client("127.0.0.1",SERVER_PORT);
    client->window = NULL;
    client->window = SDL_CreateWindow("B O M B E R M A N",
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      MAP_WIDTH * 32,
                                      MAP_HEIGHT * 32,
                                      SDL_WINDOW_SHOWN);
#if defined (WIN32)
    client->bomb_image = IMG_Load("sprites/Bomb.png");
    client->up_image = IMG_Load("sprites/BUp.png");
    client->right_image = IMG_Load("sprites/BRight.png");
    client->down_image = IMG_Load("sprites/BDown.png");
    client->left_image = IMG_Load("sprites/BLeft.png");
    client->enemy_up_image = IMG_Load("sprites/EUp.png");
    client->enemy_right_image = IMG_Load("sprites/ERight.png");
    client->enemy_down_image = IMG_Load("sprites/EDown.png");
    client->enemy_left_image = IMG_Load("sprites/ELeft.png");
    client->fire_image = IMG_Load("sprites/Fire.png");
    client->steel_image = IMG_Load("sprites/Steel.png");
    client->brick_image = IMG_Load("sprites/Brick.png");
#elif defined (linux)
    client->bomb_image = IMG_Load("/usr/share/bomberman/sprites/Bomb.png");
    client->up_image = IMG_Load("/usr/share/bomberman/sprites/BUp.png");
    client->right_image = IMG_Load("/usr/share/bomberman/sprites/BRight.png");
    client->down_image = IMG_Load("/usr/share/bomberman/sprites/BDown.png");
    client->left_image = IMG_Load("/usr/share/bomberman/sprites/BLeft.png");
    client->enemy_up_image = IMG_Load("/usr/share/bomberman/sprites/EUp.png");
    client->enemy_right_image = IMG_Load("/usr/share/bomberman/sprites/ERight.png");
    client->enemy_down_image = IMG_Load("/usr/share/bomberman/sprites/EDown.png");
    client->enemy_left_image = IMG_Load("/usr/share/bomberman/sprites/ELeft.png");
    client->fire_image = IMG_Load("/usr/share/bomberman/sprites/Fire.png");
    client->steel_image = IMG_Load("/usr/share/bomberman/sprites/Steel.png");
    client->brick_image = IMG_Load("/usr/share/bomberman/sprites/Brick.png");
#endif
}

int reset_fd_set(client_t *client) {

    FD_ZERO(&(client->set));
    FD_SET(client->socket, &(client->set));
    return client->socket;
}

void on_disconnect(client_t *client) {
    if (client->socket != INVALID_SOCKET)
        closesocket(client->socket);
    client->socket = INVALID_SOCKET;
    client->start = 0;
    printf("Disconnect \n");
}

void listen_receive_server(client_t *client) {

    int reading;
    server_request_t request;

    if (!FD_ISSET(client->socket, &(client->set)))
        return;

    reading = recv_server_request(client->socket, &request);

    if (reading == 0)
        on_disconnect(client);
    if (reading > 0) {
        if (request.protocol == GP_UPDATE)
            draw(client, request);
    }

}

void draw_image_at(client_t *client, SDL_Surface *image, int i, int j)
{
    SDL_Rect rect = {j * 32, i * 32, 32, 32};
    SDL_BlitSurface(image, NULL, SDL_GetWindowSurface(client->window), &rect);
}


void draw(client_t *client, server_request_t request) {
    int i, j;
    SDL_Surface *surface = SDL_GetWindowSurface(client->window);
    SDL_Rect rect = {0, 0, MAP_WIDTH * 32, MAP_HEIGHT * 32};
    SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 0, 0));
    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            if (request.clients[i][j][0] == 1) {
                if (request.position[0] == i && request.position[1] == j && request.alive) {
                    switch (request.clients[i][j][1]) {
                        case UP:
                            draw_image_at(client, client->up_image, i, j);
                            break;
                        case RIGHT:
                            draw_image_at(client, client->right_image, i, j);
                            break;
                        case DOWN:
                            draw_image_at(client, client->down_image, i, j);
                            break;
                        case LEFT:
                            draw_image_at(client, client->left_image, i, j);
                            break;
                        default:
                            break;
                    }
                } else {
                    switch (request.clients[i][j][1]) {
                        case UP:
                            draw_image_at(client, client->enemy_up_image, i, j);
                            break;
                        case RIGHT:
                            draw_image_at(client, client->enemy_right_image, i, j);
                            break;
                        case DOWN:
                            draw_image_at(client, client->enemy_down_image, i, j);
                            break;
                        case LEFT:
                            draw_image_at(client, client->enemy_left_image, i, j);
                            break;
                        default:
                            break;
                    }
                }
            } else {
                switch (request.map[i][j][0]) {
                    case MAP_BRICK:
                        draw_image_at(client, client->brick_image, i , j);
                        break;
                    case MAP_STEEL:
                        draw_image_at(client, client->steel_image, i, j);
                        break;
                    case MAP_BOMB:
                        draw_image_at(client, client->bomb_image, i, j);
                        break;
                    case MAP_FIRE:
                        draw_image_at(client, client->fire_image, i, j);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    SDL_UpdateWindowSurface(client->window);
}

void listen_key_event(client_t *client) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        client_request_t client_request;
        client_request.protocol = GP_MOVE;
        client_request.value = -1;
        switch (event.type) {
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_z:
                        client_request.value = UP;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        client_request.value = RIGHT;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        client_request.value = DOWN;
                        break;
                    case SDLK_LEFT:
                    case SDLK_q:
                        client_request.value = LEFT;
                        break;
                    case SDLK_SPACE:
                        client_request.protocol = GP_BOMB;
                        client_request.value = 1;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_QUIT:
                client->start = 0;
                break;
            default:
                break;
        }

        if (client_request.value != -1) {
            send_client_request(client->socket, &client_request);
        }
    }
}

void run(client_t *client) {

    int reading;
    struct timeval time;
    client->start = 1;
    while (client->start && client->socket != INVALID_SOCKET) {
        time.tv_sec = 0;
        time.tv_usec = 10;
        reading = select(reset_fd_set(client) + 1, &(client->set), NULL, NULL, &time);
        if (reading > 0) {
            listen_receive_server(client);
        } else if (reading == 0) {
            listen_key_event(client);
        }
    }

    if (client->socket != INVALID_SOCKET) {
        on_disconnect(client);
    }
}
