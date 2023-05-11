/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:30:15 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/11 16:35:02 by llefranc         ###   ########.fr       */
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

/*
 * Size of (IP header + ICMP header) * 2 because in case of an ICMP error
 * packet, its body will also contain ip header + icmp echo request header
 * of sent packet.
 */
#define RECV_PACK_SIZE ((IP_HDR_SIZE + ICMP_HDR_SIZE) * 2 + ICMP_BODY_SIZE + 1)

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
static int fill_icmp_echo_packet(uint8_t *buf, int packet_len)
{
	static int seq = 1;
	struct icmphdr *hdr = (struct icmphdr *)buf;
	struct timeval *timestamp = skip_icmphdr(buf);

	if (gettimeofday(timestamp, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	hdr->type = ICMP_ECHO;
	hdr->un.echo.id = getpid();
	hdr->un.echo.sequence = seq++;
	hdr->checksum = checksum((unsigned short *)buf, packet_len);
	return 0;
}

/**
 * Send an ICMP echo request packet.
 * @sock_fd: RAW socket file descriptor.
 * @si: Contain the information relative to the remote socket to ping.
 * @pi: Contain the information relative to the ICMP packets.
 * @opts: The different available options for ft_ping.
 *
 * Return: 0 if packet was successfully sent, -1 in case of error.
 */
int icmp_send_ping(int sock_fd, const struct sockinfo *si, struct packinfo *pi,
		   const struct options *opts)
{
	ssize_t nb_bytes;
	uint8_t buf[sizeof(struct icmphdr) + ICMP_BODY_SIZE] = {};

	if (fill_icmp_echo_packet(buf, sizeof(buf)) == -1)
		return -1;

	nb_bytes = sendto(sock_fd, buf, sizeof(buf), 0,
			  (const struct sockaddr *)&si->remote_addr,
			  sizeof(si->remote_addr));
	if (nb_bytes == -1)
		goto err;

	if (!opts->quiet && opts->verb)
		print_icmp_packet(E_PACK_SEND, buf, nb_bytes);
	pi->nb_send++;
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
static _Bool is_addressed_to_us(uint8_t *buf)
{
	struct icmphdr *hdr_sent;
	struct icmphdr *hdr_rep = (struct icmphdr *)buf;

	/* To discard our own ICMP echo request when pinging localhost */
	if (hdr_rep->type == ICMP_ECHO)
		return 0;

	/* If error, jumping to ICMP sent packet header stored in body */
	if (hdr_rep->type != ICMP_ECHOREPLY)
		buf += ICMP_HDR_SIZE + IP_HDR_SIZE;
	hdr_sent = (struct icmphdr *)buf;

	return hdr_sent->un.echo.id == getpid();
}

/**
 * Read from the RAW non-blocking socket. If there is some data available, print
 * a line describing the received packet.
 * @sock_fd: RAW socket file descriptor.
 * @pi: Contain the information relative to the ICMP packets.
 * @opts: The different available options for ft_ping.
 *
 * Return: 1 if a packet was received, 0 if there was no data to read, -1 in
 *         case of error.
 */
int icmp_recv_ping(int sock_fd, struct packinfo *pi, const struct options *opts)
{
	uint8_t buf[RECV_PACK_SIZE] = {};
	ssize_t nb_bytes;
	struct icmphdr *icmph;
	struct iovec iov[1] = {
		[0] = { .iov_base = buf, .iov_len = sizeof(buf)}
	};
	struct msghdr msg = { .msg_iov = iov, .msg_iovlen = 1 };

	nb_bytes = recvmsg(sock_fd, &msg, MSG_DONTWAIT);
	if (errno != EAGAIN && errno != EWOULDBLOCK && nb_bytes == -1) {
		printf("recvmsg err: %s\n", strerror(errno));
		return -1;
	} else if (nb_bytes == -1) {
		return 0;
	}
	icmph = skip_iphdr(buf);
	if (!is_addressed_to_us((uint8_t *)icmph))
		return 0;

	if (icmph->type == ICMP_ECHOREPLY)
		pi->nb_ok++;
	if (!opts->quiet) {
		if (print_recv_info(buf, nb_bytes) == -1)
			return -1;
		if (opts->verb) {
			print_icmp_packet(E_PACK_RECV, (uint8_t *)icmph,
			                  nb_bytes - IP_HDR_SIZE);
		}
	}
	return 1;
}