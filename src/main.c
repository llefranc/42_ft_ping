/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 12:54:16 by lucaslefran       #+#    #+#             */
/*   Updated: 2023/04/25 11:46:22 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include <linux/icmp.h>

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


// #define ICMP_ECHO		8	/* Echo Request			*/
// #define ICMP_ECHOREPLY		0	/* Echo Reply			*/

#define BUFF_SIZE 20

static _Bool recv_sigint = 0;
static _Bool recv_sigalrm = 0;

struct pinginfo {
	int nb_send;
	int nb_recv;
};

void handler(int signum)
{
	if (signum == SIGINT)
		recv_sigint = 1;
	else if (signum == SIGALRM)
		recv_sigalrm = 1;
}

static inline struct addrinfo * init_addr(char *host)
{
	struct addrinfo hints = {
		.ai_family = AF_INET, // renommer en AF_INET ?
		.ai_socktype = SOCK_RAW,
		.ai_protocol = IPPROTO_ICMP,
	};

// struct addrinfo {
// 	int              ai_flags;
// 	int              ai_family;
// 	int              ai_socktype;
// 	int              ai_protocol;
// 	socklen_t        ai_addrlen;
// 	struct sockaddr *ai_addr; // ip address
// 	char            *ai_canonname;
// 	struct addrinfo *ai_next;
// };

	// https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
	// struct in_addr addr;
	struct addrinfo *res;
	int ret;


	// if ((ret = inet_pton(AF_INET, host, &addr)) == -1) {
	// 	printf("inet_pton err %s\n", strerror(errno));
	// 	return NULL;
	// }
	// if (!ret)
	// 	printf("couldn't parse ip addr\n");
	// else
	// 	printf("couldn't parse ip addr\n");

	if ((ret = getaddrinfo(host, NULL, &hints, &res)) != 0) {
		printf("getaddrinfo err %s\n", gai_strerror(ret));
		return NULL;
	}
	return res;
}

void send_icmp_echo_req(struct pinginfo *pi)
{
	static int seq = 0;

	printf("Icmp echo request %d sent\n", seq++);
	pi->nb_send++;
	alarm(1);
}

void recv_icmp_echo_rep(struct pinginfo *pi)
{
	// mettre ici le num sec
	pi->nb_recv++;
	printf("Icmp echo response %d received\n", pi->nb_recv);
	sleep(2);
}

// void getnameinf(void)
// {
	// char hbuf[BUFF_SIZE + 1];
	// char sbuf[BUFF_SIZE + 1];
	// getnameinfo
// }

static inline float calc_perc_transmit(const struct pinginfo *pi)
{
	return (1.0 - (float)(pi->nb_recv) / (float)pi->nb_send) * 100.0;
}

static inline void print_end_info(char *host, const struct pinginfo *pi)
{
	printf("--- %s ping statistics ---\n", host);
	printf("%d packets transmitted, %d packets received, %.1f%% packet "
	       "loss\n", pi->nb_send, pi->nb_recv, calc_perc_transmit(pi));
	if (pi->nb_recv)
		printf("round-trip min/avg/max/stddev = xxx/xxx/xxx/xxx ms\n");
}

static int check(int ac)
{
	if (getuid() != 0) {
		printf("err program should be launched as root\n");
		return -1;
	}
	if (ac == 1) {
		printf("Usage error need 1 arg\n");
		return -1;
	}
	return 0;
}

int main(int ac, char **av)
{
	int sfd;
	struct addrinfo *res;
	struct pinginfo pi = {};


	if (check(ac) == -1)
		return 1;
	if ((res = init_addr(av[ac - 1])) == NULL)
		return 1;

	// Replace here with IPPROTO_ICMP
	if ((sfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		printf("Err socket\n");
		return 1;
	} else {
		printf("raw socket created\n");
	}

	uint8_t ttl = 64;
	if (setsockopt(sfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) {
		printf("setsockopt err: %s\n", strerror(errno));
		return 1;
	}


	signal(SIGINT, &handler);
	signal(SIGALRM, &handler);

	// ajouter ici le setsockopt

	send_icmp_echo_req(&pi);
	while (!recv_sigint) {
		if (recv_sigalrm) {
			recv_sigalrm = 0;
			send_icmp_echo_req(&pi);
		}
		recv_icmp_echo_rep(&pi);
	}
	print_end_info(av[ac - 1], &pi);
	freeaddrinfo(res);
	return 0;
}