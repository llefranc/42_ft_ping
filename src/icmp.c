/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:30:15 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/02 17:59:56 by llefranc         ###   ########.fr       */
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

/**
 * Calculate the ICMP checksum.
 */
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
		*((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
		sum += oddbyte;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (unsigned short) ~sum;
}

/**
 * Fill ICMP echo request packet header, and add a timestamp to the packet body.
 */
static int fill_icmp_echo_packet(struct packinfo *p_info, uint8_t *buf,
				 int packet_len)
{
	static int seq = 1;
	struct icmphdr *hdr = (struct icmphdr *)buf;
	struct timeval *timestamp = (struct timeval *)(buf + sizeof(*hdr));

	if (gettimeofday(&p_info->time_last_send, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	memcpy(timestamp, &p_info->time_last_send,
	       sizeof(p_info->time_last_send));

	hdr->type = ICMP_ECHO;
	hdr->un.echo.id = getpid();
	hdr->un.echo.sequence = seq++;
	hdr->checksum = checksum((unsigned short *)buf, packet_len);

	return 0;
}

int icmp_send_ping(int sock_fd, const struct sockinfo *s_info,
		   struct packinfo *p_info)
{
	ssize_t nb_bytes;
	uint8_t buf[sizeof(struct icmphdr) + ICMP_BODY_SIZE] = {};

	if (fill_icmp_echo_packet(p_info, buf, sizeof(buf)) == -1)
		return -1;

	nb_bytes = sendto(sock_fd, buf, sizeof(buf), 0,
			  (const struct sockaddr *)&s_info->remote_addr,
			  sizeof(s_info->remote_addr));
	if (nb_bytes == -1)
		goto err;

	// print_packet_content(E_PACK_SEND, buf, nb_bytes);
	p_info->nb_send++;
	return 0;

err:
	if (errno == EACCES) {
		printf("ft_ping: socket access error. Are you trying "
		       "to ping broadcast ?\n");
	} else {
		printf("sendto err: %s\n", strerror(errno));
	}
	return -1;
}

/**
 * Return true if the ICMP packet received is addressed to this process by
 * checking the ID field, which contain process PID. Return false if the packet
 * is an ICMP ECHO request from this process (case we're pinging localhost).
 */
_Bool is_addressed_to_us(uint8_t *buf)
{
	struct icmphdr *hdr_sent;
	struct icmphdr *hdr_rep = (struct icmphdr *)buf;

	/* To discard our own ICMP echo request when pinging localhost */
	if (hdr_rep->type == ICMP_ECHO)
		return 0;

	/* If error, jumping to ICMP sent packet header stored in body */
	if (hdr_rep->type != ICMP_ECHOREPLY)
		buf += sizeof(struct icmphdr) + sizeof(struct iphdr);
	hdr_sent = (struct icmphdr *)buf;

	return hdr_sent->un.echo.id == getpid();
}

/*
* Size of (IP header + ICMP header) * 2 because in case of error,
* ip header + icmp echo request header of sent packet will also be
* in body (cf RFC792).
*
* 0                   1                   2                   3
* 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
* |     Type      |     Code      |          Checksum             |
* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
* |                             unused                            |
* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
* |      Internet Header + 64 bits of Original Data Datagram      |
* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
int icmp_recv_ping(int sock_fd, const struct sockinfo *s_info,
		   struct packinfo *p_info)
{
	struct msghdr msg = {};
	struct iovec iov[1] = {};
	struct icmphdr *hdr;
	uint8_t buf[(sizeof(struct iphdr) + sizeof(struct icmphdr)) * 2
		    + ICMP_BODY_SIZE + 1] = {};
	ssize_t nb_bytes;
	int ttl;

	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	iov[0].iov_base = buf;
	iov[0].iov_len = sizeof(buf);

	nb_bytes = recvmsg(sock_fd, &msg, MSG_DONTWAIT);
	if (errno != EAGAIN && errno != EWOULDBLOCK && nb_bytes == -1) {
		printf("recvmsg err: %s\n", strerror(errno));
		return -1;
	} else if (nb_bytes == -1) {
		return 0;
	}
	/* Skipping IP header */
	hdr = (struct icmphdr *)(buf + sizeof(struct iphdr));
	nb_bytes -= sizeof(struct iphdr);

	if (!is_addressed_to_us((uint8_t *)hdr))
		return 0;

	ttl = ((struct iphdr *)(buf))->ttl;
	if (print_recv_info(s_info, (uint8_t *)hdr, nb_bytes, ttl) == -1)
		return -1;
	// print_packet_content(E_PACK_RECV, (uint8_t *)hdr, nb_bytes);
	hdr->type == ICMP_ECHOREPLY ? p_info->nb_ok++ : p_info->nb_err++;

	return 1;
}