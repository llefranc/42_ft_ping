/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:21:28 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/11 16:30:56 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PING_H
#define PING_H

#include <netinet/ip_icmp.h>

#define IP_TTL_VALUE 64

#define IP_HDR_SIZE (sizeof(struct iphdr))
#define ICMP_HDR_SIZE (sizeof(struct icmphdr))
#define ICMP_BODY_SIZE 56

/**
 * Ft_ping exit codes.
 * @E_EXIT_OK: At least one ICMP echo reply packet was received from target.
 * @E_EXIT_NO_REPLY: No ICMP echo reply packet was received from target.
 * @E_EXIT_ERR: An error occured.
 */
enum e_exitcode {
	E_EXIT_OK,
	E_EXIT_NO_REPLY,
	E_EXIT_ERR
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

/**
 * The different available options for ft_ping.
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
 */
struct packinfo {
	int nb_send;
	int nb_ok;
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
 * Take a pointer to ip header position in a packet and return a new pointer
 * set to after the header.
 */
static inline void * skip_iphdr(void *buf)
{
	return (void *)((uint8_t *)buf + IP_HDR_SIZE);
}

/**
 * Take a pointer to icmp header position in a packet and return a new pointer
 * set to after the header.
 */
static inline void * skip_icmphdr(void *buf)
{
	return (void *)((uint8_t *)buf + ICMP_HDR_SIZE);
}

/* check.c */
int check_rights();
int check_args(int ac, char **av, char **host, struct options *opts);

/* init.c */
int init_sock(int *sock_fd, struct sockinfo *si, char *host, int ttl);

/* icmp.c*/
int icmp_send_ping(int sock_fd, const struct sockinfo *si, struct packinfo *pi,
		   const struct options *opts);
int icmp_recv_ping(int sock_fd, struct packinfo *pi, const struct options *opts);

/* print.c */
void print_help();
void print_start_info(const struct sockinfo *si);
int print_recv_info(void *buf, ssize_t nb_bytes);
void print_icmp_packet(enum e_packtype type, uint8_t *buf, ssize_t nb_bytes);
void print_end_info(const struct sockinfo *si, const struct packinfo *pi);

#endif /* PING_H */