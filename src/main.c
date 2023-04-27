/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 12:54:16 by lucaslefran       #+#    #+#             */
/*   Updated: 2023/04/27 20:06:19 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

// #include <linux/ip.h>
// #include <linux/icmp.h>



// struct sockaddr_in {
// 	sa_family_t    sin_family; /* address family: AF_INET */
// 	in_port_t      sin_port;   /* port in network byte order */
// 	struct in_addr sin_addr;   /* internet address */
// };

// struct in_addr {
// 	uint32_t       s_addr;     /* address in network byte order */
// };

// struct icmphdr {
//   __u8		type;
//   __u8		code;
//   __sum16	checksum;
//   union {
// 	struct {
// 		__be16	id;
// 		__be16	sequence;
// 	} echo;
// 	__be32	gateway;
// 	struct {
// 		__be16	__unused;
// 		__be16	mtu;
// 	} frag;
// 	__u8	reserved[4];
//   } un;
// };

// --- india.fr ping statistics ---
// 168 packets transmitted, 0 received, +44 errors, 100% packet loss, time 169609ms


// struct icmphdr {
//   __u8		type;
//   __u8		code;
//   __sum16	checksum;
//   union {
// 	struct {
// 		__be16	id;
// 		__be16	sequence;
// 	} echo;
// 	__be32	gateway;
// 	struct {
// 		__be16	__unused;
// 		__be16	mtu;
// 	} frag;
// 	__u8	reserved[4];
//   } un;
// };

// #define ICMP_ECHO		8	/* Echo Request			*/
// #define ICMP_ECHOREPLY		0	/* Echo Reply			*/


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
			if (send_icmp_ping(sock_fd, &s_info, &p_info) == -1)
				goto fatal_close_sock;
			alarm(1);
		}
		if (recv_icmp_ping(sock_fd, &s_info, &p_info) == -1)
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