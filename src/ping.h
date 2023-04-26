/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:21:28 by llefranc          #+#    #+#             */
/*   Updated: 2023/04/26 18:07:51 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PING_H
#define PING_H

#define IP_TTL_VALUE 64
#define PING_BODY_SIZE 56

#include <netinet/ip.h>

extern _Bool recv_sigint;
extern _Bool recv_sigalrm;

struct pinginfo {
	char *host;
	struct sockaddr_in remote_addr;
	char str_sin_addr[sizeof("xxx.xxx.xxx.xxx")];
	int nb_send;
	int nb_recv_ok;
	int nb_recv_err;
	struct timeval time_start;
	struct timeval time_last_send;
};

int ping_check(int ac);
int ping_init(int *sock_fd, struct pinginfo *pi, char *host, int ttl);

void print_start_info(const struct pinginfo *pi);
int print_packet_info(const struct pinginfo *pi, int packet_len,
		      const uint8_t *buf);
void print_end_info(const struct pinginfo *pi);

#endif /* PING_H */