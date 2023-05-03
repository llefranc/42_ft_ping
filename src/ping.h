/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:21:28 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/03 17:17:37 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PING_H
#define PING_H

#include <netinet/ip_icmp.h>

#define IP_TTL_VALUE 64
#define ICMP_BODY_SIZE 56

/**
 * The different available options of ft_ping.
 * @help: Help option (display help).
 * @quiet: Quiet option (no output when receiving a packet).
 * @verb: Verbose option (display content of each received packet).
 */
struct options {
	_Bool help;
	_Bool quiet;
	_Bool verb;
};

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
	char str_sin_addr[INET_ADDRSTRLEN];
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

int check_rights();
int check_args(int ac, char **av, char **host, struct options *opts);

int init_sock(int *sock_fd, struct sockinfo *si, char *host, int ttl);

void print_help();
void print_start_info(const struct sockinfo *si);
int print_recv_info(const struct sockinfo *si, const uint8_t *buf,
		    int packet_len, int ttl);
void print_packet_content(enum e_packtype type, uint8_t *buf, int packet_len);
void print_end_info(const struct sockinfo *si, const struct packinfo *pi);

int icmp_send_ping(int sock_fd, const struct sockinfo *si, struct packinfo *pi,
		   const struct options *opts);
int icmp_recv_ping(int sock_fd, const struct sockinfo *si, struct packinfo *pi,
		   const struct options *opts);

#endif /* PING_H */