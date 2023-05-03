/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 12:54:16 by lucaslefran       #+#    #+#             */
/*   Updated: 2023/05/03 17:07:11 by llefranc         ###   ########.fr       */
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
	int sock_fd;
	char *host = NULL;
	struct options opts = {};
	struct sockinfo si = {};
	struct packinfo pi = {};

	if (check_rights() == -1 || check_args(ac, av, &host, &opts) == -1)
		return 1;
	if (init_sock(&sock_fd, &si, host, IP_TTL_VALUE) == -1)
		return 1;

	signal(SIGINT, &handler);
	signal(SIGALRM, &handler);

	print_start_info(&si);
	while (pingloop) {
		if (send_packet) {
			send_packet = 0;
			if (icmp_send_ping(sock_fd, &si, &pi, &opts) == -1)
				goto fatal_close_sock;
			alarm(1);
		}
		if (icmp_recv_ping(sock_fd, &si, &pi, &opts) == -1)
			goto fatal_close_sock;
	}
	print_end_info(&si, &pi);

	close(sock_fd);
	return 0;

fatal_close_sock:
	close(sock_fd);
	return 1;
}