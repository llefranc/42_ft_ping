/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:29:34 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/11 16:36:46 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>


/**
 * Print ft_ping help.
 */
void print_help()
{
	printf("Usage\n"
	       "  ping [options] <destination>\n\n"
	       "Options:\n"
	       "  <destination>      Dns name or ip address\n"
	       "  -h                 Show help\n"
	       "  -q                 Quiet output\n"
	       "  -v                 Verbose output\n");
}

/**
 * Print the information at start of ft_ping.
 */
void print_start_info(const struct sockinfo *si)
{
	printf("PING %s (%s) %d(%zu) bytes of data.\n", si->host,
	       si->str_sin_addr, ICMP_BODY_SIZE,
	       IP_HDR_SIZE + ICMP_HDR_SIZE + ICMP_BODY_SIZE);
}

/**
 * Print the error message associated to an ICMP error packet.
 */
static void print_icmp_err(int type, int code) {
	switch (type) {
	case ICMP_DEST_UNREACH:
		switch(code) {
		case ICMP_NET_UNREACH:
			printf("Destination Net Unreachable\n");
			break;
		case ICMP_HOST_UNREACH:
			printf("Destination Host Unreachable\n");
			break;
		case ICMP_PROT_UNREACH:
			printf("Destination Protocol Unreachable\n");
			break;
		case ICMP_PORT_UNREACH:
			printf("Destination Port Unreachable\n");
			break;
		case ICMP_FRAG_NEEDED:
			printf("Frag needed\n");
			break;
		case ICMP_SR_FAILED:
			printf("Source Route Failed\n");
			break;
		case ICMP_NET_UNKNOWN:
			printf("Destination Net Unknown\n");
			break;
		case ICMP_HOST_UNKNOWN:
			printf("Destination Host Unknown\n");
			break;
		case ICMP_HOST_ISOLATED:
			printf("Source Host Isolated\n");
			break;
		case ICMP_NET_ANO:
			printf("Destination Net Prohibited\n");
			break;
		case ICMP_HOST_ANO:
			printf("Destination Host Prohibited\n");
			break;
		case ICMP_NET_UNR_TOS:
			printf("Destination Net Unreachable for Type of Service\n");
			break;
		case ICMP_HOST_UNR_TOS:
			printf("Destination Host Unreachable for Type of Service\n");
			break;
		case ICMP_PKT_FILTERED:
			printf("Packet filtered\n");
			break;
		case ICMP_PREC_VIOLATION:
			printf("Precedence Violation\n");
			break;
		case ICMP_PREC_CUTOFF:
			printf("Precedence Cutoff\n");
			break;
		default:
			printf("Dest Unreachable, Bad Code: %d\n", code);
			break;
		}
		break;
	case ICMP_SOURCE_QUENCH:
		printf("Source Quench\n");
		break;
	case ICMP_REDIRECT:
		switch(code) {
		case ICMP_REDIR_NET:
			printf("Redirect Network");
			break;
		case ICMP_REDIR_HOST:
			printf("Redirect Host");
			break;
		case ICMP_REDIR_NETTOS:
			printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIR_HOSTTOS:
			printf("Redirect Type of Service and Host");
			break;
		default:
			printf("Redirect, Bad Code: %d", code);
			break;
		}
		break;
	}
}

/**
 * Calculate and print the RTT of a received ICMP echo reply packet.
 * If RTT > 1ms, format is x.xxms. If RTT < 1ms, format is 0.xxxms.
 */
static int print_icmp_rtt(const struct timeval *t_send)
{
	long msec;
	long usec;
	struct timeval t_recv;
	struct timeval rtt = {};

	if (gettimeofday(&t_recv, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	timersub(&t_recv, t_send, &rtt);
	msec = rtt.tv_sec * 1000 + rtt.tv_usec / 1000;
	usec = rtt.tv_usec % 1000;
	if (msec > 0) {
		usec = (usec % 1000) / 10;
		printf("time=%ld.%02ld ms\n", msec, usec);
	} else {
		usec %= 1000;
		printf("time=%ld.%03ld ms\n", msec, usec);
	}
	return 0;
}

/**
 * Print information of a received packet following a ICMP echo request:
 *    - If it's an echo reply, print number of bytes received, source IP
 *      address, sequence number, ttl value and rtt.
 *    - If it's an error packet, print source IP address, sequence number
 *      and the appropriate error message.
 * @buf: Buffer fill with the received packet (IP header + ICMP).
 * @nb_bytes: Number of bytes received.
 *
 * Return: 0 on success, -1 on error.
 */
int print_recv_info(void *buf, ssize_t nb_bytes)
{
	char addr[INET_ADDRSTRLEN] = {};
	struct iphdr *iph = buf;
	struct icmphdr *icmph = skip_iphdr(iph);
	struct timeval *icmpb = skip_icmphdr(icmph);
	struct icmphdr *err_icmph;

	inet_ntop(AF_INET, &iph->saddr, addr, INET_ADDRSTRLEN);
	if (icmph->type == ICMP_ECHOREPLY) {
		printf("%ld bytes from %s: ", nb_bytes - IP_HDR_SIZE, addr);
		printf("icmp_seq=%d ttl=%d ", icmph->un.echo.sequence, iph->ttl);
		if (print_icmp_rtt(icmpb) == -1)
			return -1;
	} else {
		/* If error, jump to ICMP sent packet header stored in body */
		err_icmph = (struct icmphdr *)((uint8_t *)icmph + IP_HDR_SIZE
		            + ICMP_HDR_SIZE);
		printf("From %s: ", addr);
		printf("icmp_seq=%d ", err_icmph->un.echo.sequence);
		print_icmp_err(icmph->type, icmph->code);
	}
	return 0;
}

/**
 * Print the content of a sent or received ICMP packet.
 * @type: Indicate the type of packet (either E_PACK_SEND or E_PACK_RECV).
 * @buf: Buffer fill with the ICMP packet (header + body).
 * @nb_bytes: Length of the packet.
 */
void print_icmp_packet(enum e_packtype type, uint8_t *buf, ssize_t nb_bytes)
{
	struct icmphdr *hdr = (struct icmphdr *)buf;
	struct timeval *tv;
	char *step_str = type == E_PACK_SEND ? "SENT" : "RECEIVED";

	printf("----------------------------------------------\n");
	printf("Packet state: %s\n\n", step_str);

	printf("ICMP header\n");
	printf("  type: %d\n", hdr->type);
	printf("  code: %d\n", hdr->code);
	printf("  checksum: %d\n", hdr->checksum);

	if (hdr->type == ICMP_ECHO || hdr->type == ICMP_ECHOREPLY) {
		tv = skip_icmphdr(buf);
		printf("  id: %d\n", hdr->un.echo.id);
		printf("  sequence: %d\n", hdr->un.echo.sequence);
		printf("ICMP body:\n");
		printf("  timestamp: %lds, %ldms\n\n", tv->tv_sec, tv->tv_usec);
	}
	printf("Packet content (ICMP header + body, %ld bytes):\n\n", nb_bytes);
	for (int i = 0; i < nb_bytes; ++i) {
		printf("%02x ", buf[i]);
	}
	printf("\n----------------------------------------------\n");
}

/**
 * Calculate the percentage of lost packets.
 */
static inline float calc_packet_loss(const struct packinfo *pi)
{
	return (1.0 - (float)(pi->nb_ok) / (float)pi->nb_send) * 100.0;
}

/**
 * Print the different statistics for all send packets at the end of ft_ping
 * command.
 */
void print_end_info(const struct sockinfo *si, const struct packinfo *pi)
{
	printf("\n--- %s ping statistics ---\n", si->host);
	printf("%d packets transmitted, %d packets received, "
	       "%d%% packet loss\n", pi->nb_send, pi->nb_ok,
	       (int)calc_packet_loss(pi));
}