/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:21:28 by llefranc          #+#    #+#             */
/*   Updated: 2023/04/27 20:07:44 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PING_H
#define PING_H

#include <netinet/ip.h>

#define IP_TTL_VALUE 64
#define ICMP_BODY_SIZE 56

/**
 * Contain the information relative to the ICMP packets.
 * @nb_send: The number of ICMP echo request packets sent.
 * @nb_recv_ok: The number of ICMP echo response packets received.
 * @nb_recv_err: The number of ICMP packets other than response packet received.
 * @time_first_send: The timestamp of the first ICMP packet sent.
 * @time_last_send:The timestamp of the last ICMP packet sent.
 */
struct packinfo {
	int nb_send;
	int nb_recv_ok;
	int nb_recv_err;
	struct timeval time_first_send;
	struct timeval time_last_send;
};

/**
 * Contain the information relative to the remote socket to ping.
 * @host: The host provided as argument to ft_ping.
 * @remote_addr: The socket remote address.
 * @str_sin_addr: A string with the resolved IPv4 address from hostname.
 */
struct sockinfo {
	char *host;
	struct sockaddr_in remote_addr;
	char str_sin_addr[sizeof("xxx.xxx.xxx.xxx")];
};

int ping_check(int ac);
int ping_init(int *sock_fd, struct sockinfo *s_info, struct packinfo *p_info,
	      char *host, int ttl);

void debug_print_packet(const char *step, const uint8_t *buf, int size);
void print_start_info(const struct sockinfo *s_info);
int print_recv_info(const struct sockinfo *s_info, int packet_len,
		    const uint8_t *buf);
void print_end_info(const struct sockinfo *s_info,
		    const struct packinfo *p_info);

int send_icmp_ping(int sock_fd, const struct sockinfo *s_info,
		   struct packinfo *p_info);
int recv_icmp_ping(int sock_fd, const struct sockinfo *s_info,
		   struct packinfo *p_info);

#endif /* PING_H */