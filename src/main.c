/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 12:54:16 by lucaslefran       #+#    #+#             */
/*   Updated: 2023/05/02 17:11:15 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

_Bool pingloop = 1;
_Bool send_packet = 1;

void handler(int signum)
{
	if (signum == SIGINT)
		pingloop = 0;
	else if (signum == SIGALRM)
		send_packet = 1;
}

int main(int ac, char **av)
{
	char *host;
	int sock_fd;
	struct sockinfo s_info = {};
	struct packinfo p_info = {};

	if (ping_check(ac) == -1)
		return 1;
	host = av[ac - 1];
	if (ping_init(&sock_fd, &s_info, &p_info, host, IP_TTL_VALUE) == -1)
		goto fatal;

	signal(SIGINT, &handler);
	signal(SIGALRM, &handler);

	print_start_info(&s_info);
	while (pingloop) {
		if (send_packet) {
			send_packet = 0;
			if (icmp_send_ping(sock_fd, &s_info, &p_info) == -1)
				goto fatal_close_sock;
			alarm(1);
		}
		if (icmp_recv_ping(sock_fd, &s_info, &p_info) == -1)
			goto fatal_close_sock;
	}
	print_end_info(&s_info, &p_info);

	close(sock_fd);
	return 0;

fatal_close_sock:
	close(sock_fd);
fatal:
	printf("ft_ping: fatal error\n");
	return 1;
}