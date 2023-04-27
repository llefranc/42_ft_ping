/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/24 12:54:16 by lucaslefran       #+#    #+#             */
/*   Updated: 2023/04/27 19:29:30 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

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


_Bool pingloop = 1;
_Bool send_packet = 1;

void handler(int signum)
{
	if (signum == SIGINT)
		pingloop = 0;
	else if (signum == SIGALRM)
		send_packet = 1;
}

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

// implementer en plus le recv icmp type

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

int send_icmp_echo_req(int sock_fd, const struct sockinfo *s_info,
		       struct packinfo *p_info)
{
	ssize_t nb_bytes;
	static int seq = 1;
	struct icmphdr *hdr;
	uint8_t buf[sizeof(struct icmphdr) + PING_BODY_SIZE] = {};

	if (gettimeofday(&p_info->time_last_send, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	memcpy(buf + sizeof(*hdr), &p_info->time_last_send,
	       sizeof(p_info->time_last_send));

	hdr = (struct icmphdr *)buf;
	hdr->type = ICMP_ECHO;
	hdr->un.echo.id = getpid();
	hdr->un.echo.sequence = 9;
	hdr->un.echo.sequence = seq++;
	hdr->checksum = checksum((unsigned short *)buf, sizeof(buf));

	if ((nb_bytes = sendto(sock_fd, buf, sizeof(buf), 0,
	    (const struct sockaddr *)&s_info->remote_addr,
	    sizeof(s_info->remote_addr))) == -1) {
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

	p_info->nb_send++;
	return 0;
}

pid_t get_packet_pid(uint8_t *buf)
{
	struct icmphdr *hdr = (struct icmphdr *)(buf + sizeof(struct iphdr));

	return hdr->un.echo.id;
}

int recv_icmp_echo_rep(int sock_fd, const struct sockinfo *s_info,
		       struct packinfo *p_info)
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
	if (print_recv_info(s_info, nb_bytes, buf) == -1)
		return -1;
	p_info->nb_recv_ok++;
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
			if (send_icmp_echo_req(sock_fd, &s_info, &p_info) == -1)
				goto fatal_close_sock;
			alarm(1);
		}
		if (recv_icmp_echo_rep(sock_fd, &s_info, &p_info) == -1)
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