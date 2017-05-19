#include "request.h"
#include "socket_util.h"

int posx = 1;

void init_request(request_t *source, char *filename)
{
  int row, col, i;
  char c;
  FILE *file;
  for (i = 0; i < MAX_CLIENTS; i++)
    {
      source->clients[i].id = -1;
      source->clients[i].posx = -1;
      source->clients[i].posy = -1;
    }


  file = fopen(filename, "r");
  row = 0;
  col = 0;
  while (1)
    {
      c = fgetc(file);
      if (col >= MAP_COL)
	{
	  col = 0;
	  row++;
	}
      if (feof(file) || row >= MAP_ROW)
	break;
      
      if (c == BLOCK_PATH || c == BLOCK_STEEL || c == BLOCK_BRICK)
	source->map[row][col++] = c;
      else if (c != '\n')
	break;
    }
  if (row != MAP_ROW || col != 0)
    die("bad map");
}

void copy_request(request_t source, request_t *destination)
{
  int i, j;
  for (i = 0; i < MAP_ROW; i++)
    for (j = 0; j < MAP_COL; j++)
      destination->map[i][j] = source.map[i][j];
  for (i = 0; i < MAX_CLIENTS; i++)
    destination->clients[i] = source.clients[i];
}

int set_client(request_t *source, int client)
{
  int i;
  for (i = 0; i < MAX_CLIENTS; i++)
    if (source->clients[i].id == -1)
      {	
	source->clients[i].id = client;
	source->clients[i].posx = posx++;
	source->clients[i].posy = 1;
	source->clients[i].alive = 1;
	source->clients[i].direction = LEFT;
	source->clients[i].next_direction = NOTHING;
	return i;
      }
  return -1;
}


void reset_client(client_t *client)
{
  client->id = -1;
  client->posx = -1;
  client->posy = -1;
  client->alive= 0;
  client->direction = NOTHING;
  client->next_direction = NOTHING;
}

int main(int argc, char **args)
{
  int socket,
    max,
    i,
    j,
    k,
    t,
    len,
    client,
    pos,
    ret,
    reading,
    time;
  fd_set set;
  request_t server_request;
  request_t request;
  struct timeval tv;
  struct sockaddr_in addr;
  int bombs[MAP_ROW][MAP_COL];
  for (i = 0; i < MAP_ROW; i++)
    for (j = 0; j < MAP_COL; j++)
      bombs[i][j] = 0;
  
  socket = create_server("0.0.0.0", 4242);
  printf("Server is running ...\n");
  fflush(stdout);
  init_request(&server_request, "map.txt");
  while (1)
    {
      FD_ZERO(&set);
      FD_SET(socket, &set);
      max = socket;
      tv.tv_sec = 0;
      tv.tv_usec = 10;
      for (i = 0; i < MAX_CLIENTS; i++)
	{
	  if (server_request.clients[i].id != -1)
	    {
	      FD_SET(server_request.clients[i].id , &set);
	      if (server_request.clients[i].id > max)	
		max = server_request.clients[i].id;
	    }
	}
      
      if ((ret = select(max + 1, &set, NULL, NULL, &tv)) < 0)
	die("select()");
      
      if (ret == 0)
	{
	  // timeout
	  if (time > 1000)
	    {
	      time = 0;
	      for (i = 0; i < MAP_ROW; i++)
		{
		  for (j = 0; j < MAP_COL; j++)
		    {
		      if (bombs[i][j] > 0)
			bombs[i][j]--;
		      
		      if (bombs[i][j] == 0)
			{
			  if (server_request.map[i][j] == BLOCK_FIRE)
			    {			      
			      server_request.map[i][j] = BLOCK_PATH;
			      bombs[i][j] = 0;
			    }
			  else if (server_request.map[i][j] == BLOCK_BOMB_1)
			    {
			      for (k = -2; k < 3; k++)
				{
				  if (i + k > 0 && i + k < MAP_ROW)
				    {
				      if (server_request.map[i + k][j] != BLOCK_STEEL)
					{
					  bombs[i + k][j] = 4;
					  server_request.map[i + k][j] = BLOCK_FIRE;
					}
				    }
				  if (j + k > 0 && j + k < MAP_ROW)
				    {
				      if (server_request.map[i][j + k] != BLOCK_STEEL)
					{
					  bombs[i][j + k] = 4; 
					  server_request.map[i][j + k] = BLOCK_FIRE;
					}
				    }
				  for (t = 0; t < MAX_CLIENTS; t++)
				    {
				      client_t aux = server_request.clients[t];
				      
				      if (aux.id > 0 && aux.alive
					  && ((aux.posx == (k + j) && aux.posy == i)
					      ||(aux.posx == j && aux.posy == (k + i))))
					{
					  server_request.clients[t].alive = 0;
					  server_request.clients[t].posx = -1;
					  server_request.clients[t].posy = -1;
					  request.protocol = P_GAME_OVER;
					  write(aux.id, &request, sizeof(request_t));
					}
				    }
				}
			    }
			}
		    }
		}
	      for (i = 0; i < MAX_CLIENTS; i++)
		{
		  if (server_request.clients[i].id > 0 && server_request.clients[i].alive)
		    {      
		      int x = server_request.clients[i].posx;
		      int y = server_request.clients[i].posy;
		      if (server_request.clients[i].pose_bomb)
			{
			  server_request.map[y][x] = BLOCK_BOMB_1;
			  bombs[y][x] = 20;
			}
		      switch(server_request.clients[i].next_direction)
			{
			case LEFT:
			  if (server_request.clients[i].posx > 0
			      &&  server_request.map[y][x - 1] == BLOCK_PATH)
			    server_request.clients[i].posx--;
			  printf("left\n"); 
			  break;
			case RIGHT:
			  if (server_request.clients[i].posx + 1 < MAP_COL
			      && server_request.map[y][x + 1] == BLOCK_PATH)
			      server_request.clients[i].posx++;
			  printf("right\n");
			  break;
			case DOWN:
			  if (server_request.clients[i].posy + 1 < MAP_ROW 
			      &&  server_request.map[y + 1][x] == BLOCK_PATH)
			    server_request.clients[i].posy++;
			  printf("down\n");
			  break;
			case UP:
			  if (server_request.clients[i].posy > 0
			      &&  server_request.map[y - 1][x] == BLOCK_PATH)
			    server_request.clients[i].posy--;
			  printf("up\n");
			  break;
			}
		      fflush(stdout);
		    }
		  server_request.clients[i].pose_bomb = 0;
		  server_request.clients[i].next_direction = NOTHING;
		}
	      for (i = 0; i < MAX_CLIENTS; i++)
		{
		  if (server_request.clients[i].id > 0)
		    {
		      request.protocol = P_UPDATE;
		      copy_request(server_request, &request);
		      write(server_request.clients[i].id,
			    &request, sizeof(request_t));
		    }
		}
	    }
	  time++;
	  fflush(stdout);
	}
      else
	{
	  if (FD_ISSET(socket, &set))
	    {
	      len = sizeof(addr);
	      client = accept(socket, (struct sockaddr *) &addr, &len);
	      printf("New client is connect\n");
	      fflush(stdout);
	      pos = set_client(&server_request, client);
	      request.protocol = P_CONNECT;
	      request.id = client;
	      if (pos >= 0)
		request.response = 1;
	      else
		request.response = 0;
	      copy_request(server_request, &request);
	      write(client, &request, sizeof(request_t));
	    }

	  for (i = 0; i < MAX_CLIENTS; i++)
	    {
	      ret = server_request.clients[i].id;

	      if (ret > 0 && FD_ISSET(ret, &set))
		{
		  reading = read(ret, &request, sizeof(request_t));
		  if (reading == 0)
		    {
		      printf("client %d is disconnect\n",  ret);
		      reset_client(&(server_request.clients[i]));
		      close(ret);
		      fflush(stdout);
		    }
		  else
		    {
		      if (request.protocol == P_MOVE)
			{
			  server_request.clients[i].next_direction =
			    request.direction;
			  printf("Client want move\n");
			}
		      else if (request.protocol == P_POSE_BOMB)
			{
			  server_request.clients[i].pose_bomb = 1;
			  printf("Client want pose bomb\n");
			}
		      fflush(stdout);
		    }
		}
	    }
	}
    }
  
  return (0);
}