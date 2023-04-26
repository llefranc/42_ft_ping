/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 12:54:16 by lucaslefran       #+#    #+#             */
/*   Updated: 2023/04/26 15:53:04 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

// #include <linux/ip.h>
// #include <linux/icmp.h>

#include <netinet/ip_icmp.h>

#define IP_TTL_VALUE 64

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

#define PING_BODY_SIZE 56

static _Bool recv_sigint = 0;
static _Bool recv_sigalrm = 0;

struct pinginfo {
	char *host;
	struct sockaddr_in remote_addr;
	char str_sin_addr[sizeof("xxx.xxx.xxx.xxx")];
	int nb_send;
	int nb_recv_ok;
	int nb_recv_err;
};

void handler(int signum)
{
	if (signum == SIGINT)
		recv_sigint = 1;
	else if (signum == SIGALRM)
		recv_sigalrm = 1;
}

static int check_launch(int ac)
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

static inline int init_addr(struct pinginfo *pi)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_RAW,
		.ai_protocol = IPPROTO_ICMP
	};
	struct addrinfo *tmp;
	int ret;

	if ((ret = getaddrinfo(pi->host, NULL, &hints, &tmp)) != 0) {
		printf("getaddrinfo err %s\n", gai_strerror(ret));
		return -1;
	}
	pi->remote_addr = *(struct sockaddr_in *)tmp->ai_addr;
	freeaddrinfo(tmp);

	if (inet_ntop(AF_INET, &pi->remote_addr.sin_addr, pi->str_sin_addr,
	    sizeof(pi->str_sin_addr)) == NULL) {
		printf("inet_ntop err: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

static inline int init_socket(uint8_t ttl)
{
	int sock_fd;

	if ((sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		printf("socket err: %s\n", strerror(errno));
		return -1;
	}
	if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) {
		printf("setsockopt err: %s\n", strerror(errno));
		return -1;
	}
	return sock_fd;
}

// --- india.fr ping statistics ---
// 168 packets transmitted, 0 received, +44 errors, 100% packet loss, time 169609ms

static inline void print_start_info(const struct pinginfo *pi)
{
	printf("PING %s (%s) %d(%zu) bytes of data.\n", pi->host,
	       pi->str_sin_addr, PING_BODY_SIZE,
	       PING_BODY_SIZE + sizeof(struct iphdr) + sizeof(struct icmphdr));
}

static inline float calc_perc_transmit(const struct pinginfo *pi)
{
	return (1.0 - (float)(pi->nb_recv_ok) / (float)pi->nb_send) * 100.0;
}

static inline int calc_ms_elapsed(const struct timeval *start_time,
			      const struct timeval *end_time)
{
	return (end_time->tv_sec - start_time->tv_sec) * 1000
	       + (end_time->tv_usec - start_time->tv_usec) / 1000;
}

static inline void print_end_info(const struct pinginfo *pi,
				  const struct timeval *start_time,
				  const struct timeval *end_time)
{
	int ms_elapsed = calc_ms_elapsed(start_time, end_time);

	printf("\n--- %s ping statistics ---\n", pi->host);
	printf("%d packets transmitted, %d received, ", pi->nb_send,
	       pi->nb_recv_ok);

	if (pi->nb_recv_err)
		printf("+%d errors, ", pi->nb_recv_err);

	printf("%d%% packet loss, time %dms\n", (int)calc_perc_transmit(pi),
	       ms_elapsed);

	if (pi->nb_recv_ok)
		printf("rtt min/avg/max/stddev = xxx/xxx/xxx/xxx ms\n");
}

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

// implementer en plus le recv icmp type
int print_packet_info(const struct pinginfo *pi, int nb_recv_bytes, const uint8_t *buf)
{
	struct timeval now;
	struct iphdr *p_iph;
	struct icmphdr *p_icmph;
	struct timeval *p_body;

	if (gettimeofday(&now, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	p_iph = (struct iphdr *)buf;
	p_icmph = (struct icmphdr *)(buf + sizeof(*p_iph));
	p_body = (struct timeval *)(buf + sizeof(*p_iph) + sizeof(*p_icmph));

	printf("------------ recv beg -----------\n");
	for (int i = 0; i < nb_recv_bytes; ++i) {
		printf("%02x ", buf[i]);
	}
	printf("\n");
	printf("------------ recv end -----------\n");

	printf("%ld bytes from %s: icmp_seq=%d ttl=%d ",
	       nb_recv_bytes - sizeof(*p_iph), pi->str_sin_addr,
	       p_icmph->un.echo.sequence, p_iph->ttl);
	printf("time=%ld us\n", now.tv_usec - p_body->tv_usec);
	return 0;
}

unsigned short checksum(unsigned short *ptr, int nbytes) {
	unsigned long sum;
	unsigned short oddbyte;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1) {
		oddbyte = 0;
		*((unsigned char *) &oddbyte) = *(unsigned char *) ptr;
		sum += oddbyte;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (unsigned short) ~sum;
}

int send_icmp_echo_req(int sock_fd, struct pinginfo *pi,
		       struct timeval *last_send_time)
{
	ssize_t nb_bytes;
	static int seq = 1;
	struct icmphdr *hdr;
	uint8_t buf[sizeof(struct icmphdr) + PING_BODY_SIZE] = {};

	if (gettimeofday(last_send_time, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	memcpy(buf + sizeof(*hdr), last_send_time, sizeof(*last_send_time));

	hdr = (struct icmphdr *)buf;
	hdr->type = ICMP_ECHO;
	hdr->un.echo.id = getpid();
	hdr->un.echo.sequence = 9;
	hdr->un.echo.sequence = seq++;
	hdr->checksum = checksum((unsigned short *)buf, sizeof(buf));

	if ((nb_bytes = sendto(sock_fd, buf, sizeof(buf), 0,
	    (const struct sockaddr *)&pi->remote_addr,
	    sizeof(pi->remote_addr))) == -1) {
		printf("sendto err: %s\n", strerror(errno));
	} else {
		// printf("send %zu bytes\n", nb_bytes);
	}
	printf("------------ send beg -----------\n");
	for (int i = 0; i < nb_bytes; ++i) {
		printf("%02x ", buf[i]);
	}
	printf("\n");
	printf("------------ send end -----------\n");

	pi->nb_send++;
	return 0;
}

static inline pid_t get_packet_pid(uint8_t *buf)
{
	struct icmphdr *hdr = (struct icmphdr *)(buf + sizeof(struct iphdr));

	return hdr->un.echo.id;
}

int recv_icmp_echo_rep(int sock_fd, struct pinginfo *pi)
{
	struct msghdr msg = {};
	struct iovec iov[1] = {};
	ssize_t nb_bytes;
	uint8_t buf[sizeof(struct iphdr) + sizeof(struct icmphdr)
		    + PING_BODY_SIZE] = {};

	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	iov[0].iov_base = buf;
	iov[0].iov_len = sizeof(buf);

	nb_bytes = recvmsg(sock_fd, &msg, MSG_DONTWAIT);

	if (errno != EAGAIN && errno != EWOULDBLOCK && nb_bytes == -1) {
		printf("recvmsg err: %s\n", strerror(errno));
		return -1;
	} else if (nb_bytes == -1 ||
		   (nb_bytes != -1 && get_packet_pid(buf) != getpid())) {
		return 0;
	}
	// if (get)
	// rajotuer ici une fonction pour check le type de retour
	if (print_packet_info(pi, nb_bytes, buf) == -1)
		return -1;
	pi->nb_recv_ok++;
	// printf("Icmp echo response %d received\n", pi->nb_recv_ok);
	// sleep(2);
	return 0;
}

static int ping_loop(int sock_fd, struct pinginfo *pi,
		     struct timeval *last_send_time)
{
	if (send_icmp_echo_req(sock_fd, pi, last_send_time) == -1)
		return -1;
	alarm(1);

	while (!recv_sigint) {
		if (recv_sigalrm) {
			recv_sigalrm = 0;
			if (send_icmp_echo_req(sock_fd, pi, last_send_time) == -1)
				return -1;
			alarm(1);
		}
		if (recv_icmp_echo_rep(sock_fd, pi) == -1)
			return -1;
	}
	return 0;
}

// split un peu mon gros fichier degueu en plusieurs fichiers
// print
// init
// icmp
// rename send_ping
// rename recv_ping
int main(int ac, char **av)
{
	int sock_fd;
	struct pinginfo pi = {};
	struct timeval start_time = {};
	struct timeval last_send_time = {};

	if (check_launch(ac) == -1)
		return 1;
	pi.host = av[ac - 1];
	if (gettimeofday(&start_time, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		goto fatal_err;
	}
	if (init_addr(&pi) == -1)
		goto fatal_err;
	if ((sock_fd = init_socket(IP_TTL_VALUE)) == -1)
		return 1; // a revoir

	signal(SIGINT, &handler);
	signal(SIGALRM, &handler);

	print_start_info(&pi);
	if (ping_loop(sock_fd, &pi, &last_send_time) == -1)
		goto fatal_err_close_sock;
	print_end_info(&pi, &start_time, &last_send_time);

	close(sock_fd);
	return 0;

fatal_err_close_sock:
	close(sock_fd);
fatal_err:
	printf("Fatal error: ft_ping exited unexpectedly\n");
	return 1;
}