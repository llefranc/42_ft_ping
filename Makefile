# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lucaslefrancq <lucaslefrancq@student.42    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/04/24 12:53:39 by lucaslefran       #+#    #+#              #
#    Updated: 2023/04/24 13:09:31 by lucaslefran      ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	ft_ping

SRCS		=	main.c

# HDRS		=

PATH_SRCS	=	src/

OBJS		=	$(SRCS:.c=.o)

CC		=	gcc

FLAGS		=	-g -fsanitize=address -Wall -Werror -Wextra

all		:	$(NAME)

# $(NAME)		:	$(addprefix $(PATH_SRCS), $(OBJS)) $(HDRS)
$(NAME)		:	$(addprefix $(PATH_SRCS), $(OBJS))
				$(CC) -o $(NAME) $(addprefix $(PATH_SRCS), $(OBJS)) $(FLAGS)
				@echo "ping is ready";

clean		:
				rm -rf $(addprefix $(PATH_SRCS), $(OBJS))

fclean		:	clean
				rm -rf $(NAME)

re		:	fclean all

.PHONY		:	all clean fclean re

%.o		:	%.c
				@$(CC) $(FLAGS) -o $@ -c $<