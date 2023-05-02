/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:21:28 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/02 19:02:20 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PING_H
#define PING_H

#include <netinet/ip_icmp.h>

#define IP_TTL_VALUE 64
#define ICMP_BODY_SIZE 56

/**
 * Contain the information relative to the ICMP packets.
 * @nb_send: The number of ICMP echo request packets sent.
 * @nb_ok: The number of ICMP echo response packets received.
 * @nb_err: The number of ICMP packets other than response packet received.
 * @time_first_send: The timestamp of the first ICMP packet sent.
 * @time_last_send:The timestamp of the last ICMP packet sent.
 */
struct packinfo {
	int nb_send;
	int nb_ok;
	int nb_err;
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

/**
 * Indicate if a packet was sent or received.
 * @E_PACK_SEND: The packet was sent.
 * @E_PACK_RECV: The packet was received.
 */
enum e_packtype {
	E_PACK_SEND,
	E_PACK_RECV,
};

int ping_check(int ac);
int ping_init_sock(int *sock_fd, struct sockinfo *s_info, char *host, int ttl);

void print_start_info(const struct sockinfo *s_info);
int print_recv_info(const struct sockinfo *s_info, const uint8_t *buf,
		    int packet_len, int ttl);
void print_packet_content(enum e_packtype type, uint8_t *buf, int packet_len);
void print_end_info(const struct sockinfo *s_info,
		    const struct packinfo *p_info);

int icmp_send_ping(int sock_fd, const struct sockinfo *s_info,
		   struct packinfo *p_info);
int icmp_recv_ping(int sock_fd, const struct sockinfo *s_info,
		   struct packinfo *p_info);

#endif /* PING_H */