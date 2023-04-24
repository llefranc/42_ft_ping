/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lucaslefrancq <lucaslefrancq@student.42    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 12:54:16 by lucaslefran       #+#    #+#             */
/*   Updated: 2023/04/24 21:15:32 by lucaslefran      ###   ########.fr       */
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


// #include <icmp.h> >>>> utiliser ces headers pour les define
// #include <ip.h>

#define BUFF_SIZE 20

static _Bool recv_sigint = 0;
static _Bool recv_sigalrm = 0;

struct pinginfo {
	int nb_send;
	int nb_recv;
};

struct icmppack {
	uint8_t type;
	uint8_t code;
	uint8_t checksum;
	char *data;
	uint8_t data_len;
};

void handler(int signum)
{
	if (signum == SIGINT)
		recv_sigint = 1;
	else if (signum == SIGALRM)
		recv_sigalrm = 1;
}

static inline struct addrinfo * init_addr(char *input)
{
	struct addrinfo hints = {
		.ai_family = PF_INET, // renommer en AF_INET ?
		.ai_socktype = SOCK_RAW,
	};
	// https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
	struct addr_in addr;
	struct addrinfo *res;
	int ret;


	if ((ret = inet_pton(PF_INET, input, &addr)) == -1) {
		printf("inet_pton err %s\n", strerror(errno));
		return NULL;
	}
	if (!ret)
		printf("couldn't parse ip addr\n");
	else
		printf("couldn't parse ip addr\n");

	if ((ret = getaddrinfo(input, "http", &hints, &res)) != 0) {
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
	printf("Icmp echo response %d received\n", 1);
	pi->nb_recv++;
}

// void getnameinf(void)
// {
	// char hbuf[BUFF_SIZE + 1];
	// char sbuf[BUFF_SIZE + 1];
	// getnameinfo
// }

static inline float calc_perc_transmit(const struct pinginfo *pi)
{
	return 100.0 - (float)(pi->nb_recv) / (float)pi->nb_send;
}

static inline void print_end_info(void)
{
	printf("--- %s ping statistics ---\n", av[ac - 1]);
	printf("%d packets transmitted, %d packets received, %.1f%% packet "
	       "loss\n", pi.nb_send, pi.nb_recv, calc_perc_transmit(&pi));
	if (pi.nb_recv)
		printf("round-trip min/avg/max/stddev = xxx/xxx/xxx/xxx ms");
}

static int check(int ac, char **av)
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
	int raw_sock;
	struct addrinfo *res;
	struct pinginfo pi = {};


	if (check(ac) == -1)
		return 1;
	if ((res = init_addr(av[ac - 1])) == NULL)
		return 1;

	// Replace here with IPPROTO_ICMP
	if ((raw_sock = socket(PF_INET, SOCK_RAW, 1)) == -1) {
		printf("Err socket\n");
		return 1;
	} else {
		printf("raw socket created\n");
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
	print_end_info();
	freeaddrinfo(res);
	return 0;
}