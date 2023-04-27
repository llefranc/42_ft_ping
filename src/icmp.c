/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:30:15 by llefranc          #+#    #+#             */
/*   Updated: 2023/04/27 20:07:33 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

static unsigned short checksum(unsigned short *ptr, int nbytes) {
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

int send_icmp_ping(int sock_fd, const struct sockinfo *s_info,
		   struct packinfo *p_info)
{
	ssize_t nb_bytes;
	static int seq = 1;
	struct icmphdr *hdr;
	uint8_t buf[sizeof(struct icmphdr) + ICMP_BODY_SIZE] = {};

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
	debug_print_packet("send", buf, nb_bytes);

	p_info->nb_send++;
	return 0;
}

// implementer en plus le recv icmp type
static inline pid_t get_packet_pid(uint8_t *buf)
{
	struct icmphdr *hdr = (struct icmphdr *)(buf + sizeof(struct iphdr));

	return hdr->un.echo.id;
}


int recv_icmp_ping(int sock_fd, const struct sockinfo *s_info,
		   struct packinfo *p_info)
{
	struct msghdr msg = {};
	struct iovec iov[1] = {};
	ssize_t nb_bytes;
	uint8_t buf[sizeof(struct iphdr) + sizeof(struct icmphdr)
		    + ICMP_BODY_SIZE] = {};

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