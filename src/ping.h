/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:21:28 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/15 22:11:19 by llefranc         ###   ########.fr       */
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
 * @E_EXIT_ERR_HOST: No ICMP echo reply packet was received from target.
 * @E_EXIT_ERR_ARGS: An error occured while parsing arguments.
 */
enum e_exitcode {
	E_EXIT_OK,
	E_EXIT_ERR_HOST,
	E_EXIT_ERR_ARGS = 64
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
 * A node containing a round-trip time as a value. Can be assembled in a linked
 * list of round-trip times.
 * @val: A round-trip time calculated for an ICMP echo response packet.
 * @next: A pointer the next node.
 */
struct rtt_node {
	struct timeval val;
	struct rtt_node *next;
};

/**
 * Contain the information relative to the ICMP packets.
 * @nb_send: The number of ICMP echo request packets sent.
 * @nb_ok: The number of ICMP echo response packets received.
 * @min: A pointer to the minimum rtt value in all the ICMP responses.
 * @max: A pointer to the maximum rtt value in all the ICMP responses.
 * @ave: The average rtt value from all ICMP responses.
 * @stddev: The standard deviation from average rtt value from all ICMP
 *          responses.
 * @rtt_list: A linked list of all rtt values from all ICMP responses.
 * @rtt_last: A pointer to the rtt of the last received ICMP packet.
 */
struct packinfo {
	int nb_send;
	int nb_ok;
	struct timeval *min;
	struct timeval *max;
	struct timeval avg;
	struct timeval stddev;
	struct rtt_node *rtt_list;
	struct rtt_node *rtt_last;
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
int icmp_send_ping(int sock_fd, const struct sockinfo *si, struct packinfo *pi);
int icmp_recv_ping(int sock_fd, struct packinfo *pi,
		   const struct options *opts);

/* print.c */
void print_help();
void print_start_info(const struct sockinfo *si, const struct options *opts);
int print_recv_info(void *buf, ssize_t nb_bytes, const struct options *opts,
                    const struct packinfo *pi);
void print_end_info(const struct sockinfo *si, struct packinfo *pi);

/* rtts.c */
struct rtt_node * rtts_save_new(struct packinfo *pi, struct icmphdr *icmph);
void rtts_clean(struct packinfo *pi);
void rtts_calc_stats(struct packinfo *pi);

#endif /* PING_H */