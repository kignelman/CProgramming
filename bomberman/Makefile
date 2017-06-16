CC	=	gcc
CFLAGS  =   -Wall -Werror
NAME1	=	server
NAME2	=	client
SRC1	=	server_main.c	\
		socket_util.c \
		server.c

SRC2    = 	client_main.c	\
		socket_util.c \
		client.c

OBJ1	=	$(SRC1:%.c=%.o)

OBJ2	=	$(SRC2:%.c=%.o)

RM	=	rm -f

$(NAME1):	$(OBJ1)
		$(CC) $(OBJ1) -o $(NAME1)

$(NAME2):	$(OBJ2)
		$(CC) $(OBJ2) -o $(NAME2) -lSDL2 -lSDL2main -lSDL2_image

all:		$(NAME1) $(NAME2)

clean:
	$(RM) $(OBJ1)
	$(RM) $(OBJ2)

fclean:		clean
	$(RM) $(NAME1)
	$(RM) $(NAME2)

re:	fclean all