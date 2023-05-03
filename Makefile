# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/04/24 12:53:39 by lucaslefran       #+#    #+#              #
#    Updated: 2023/05/02 20:41:35 by llefranc         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	ft_ping

SRCS		=	main.c init.c print.c icmp.c check.c

HDRS		=	ping.h

PATH_SRCS	=	src/

OBJS		=	$(SRCS:.c=.o)

CC		=	gcc

FLAGS		=	-g -fsanitize=address -Wall -Werror -Wextra

all		:	$(NAME)

$(NAME)		:	$(addprefix $(PATH_SRCS), $(OBJS))
				$(CC) -o $(NAME) $(addprefix $(PATH_SRCS), $(OBJS)) $(FLAGS)
				@echo "ping is ready";

clean		:
				rm -rf $(addprefix $(PATH_SRCS), $(OBJS))

fclean		:	clean
				rm -rf $(NAME)

re		:	fclean all

.PHONY		:	all clean fclean re

%.o		:	%.c $(addprefix $(PATH_SRCS), $(HDRS))
				$(CC) $(FLAGS) -o $@ -c $<