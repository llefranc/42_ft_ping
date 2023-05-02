/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:29:34 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/02 17:30:21 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>

/**
 * Print the content of a sent or received ICMP packet.
 */
void print_packet(enum e_packtype type, uint8_t *buf, int packet_len)
{
	char *step_str = type == E_PACK_SEND ? "SENT" : "RECEIVED";
	struct icmphdr *hdr = (struct icmphdr *)(buf);
	struct timeval *tv;

	printf("----------------------------------------------\n");
	printf("Packet state: %s\n\n", step_str);

	printf("ICMP header\n");
	printf("  type: %d\n", hdr->type);
	printf("  code: %d\n", hdr->code);
	printf("  checksum: %d\n", hdr->checksum);

	if (hdr->type == ICMP_ECHO || hdr->type == ICMP_ECHOREPLY) {
		tv = (struct timeval *)(buf + sizeof(*hdr));
		printf("  id: %d\n", hdr->un.echo.id);
		printf("  sequence: %d\n", hdr->un.echo.sequence);
		printf("ICMP body:\n");
		printf("  timestamp: %lds, %ldms\n\n", tv->tv_sec, tv->tv_usec);
	}
	printf("Packet content (ICMP header + body, %d bytes):\n\n", packet_len);
	for (int i = 0; i < packet_len; ++i) {
		printf("%02x ", buf[i]);
	}
	printf("\n----------------------------------------------\n");
}

/**
 * Calculate the percentage of lost packets.
 */
static inline float calc_perc_packet_loss(const struct packinfo *p)
{
	return (1.0 - (float)(p->nb_ok) / (float)p->nb_send) * 100.0;
}

/**
 * Calculate the number of milliseconds elapsed between two times.
 */
static inline int calc_ms_elapsed(const struct timeval *start,
				  const struct timeval *end)
{
	return (end->tv_sec - start->tv_sec) * 1000
	       + (end->tv_usec - start->tv_usec) / 1000;
}

/**
 * Print the information at start of ft_ping.
 */
void print_start_info(const struct sockinfo *pi)
{
	printf("PING %s (%s) %d(%zu) bytes of data.\n", pi->host,
	       pi->str_sin_addr, ICMP_BODY_SIZE,
	       ICMP_BODY_SIZE + sizeof(struct iphdr) + sizeof(struct icmphdr));
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
 * Print information of a received ICMP packet :
 *    - If it's an echo reply, print number of bytes received, remote host
 *      address, sequence number, ttl value and rtt.
 *    - If it's an error packet, print remote host address, sequence number
 *      and the appropriate error message.
 */
int print_recv_info(const struct sockinfo *s_info, const uint8_t *buf,
		    int packet_len, int ttl)
{
	struct timeval now;
	struct icmphdr *p_hdr = (struct icmphdr *)(buf);
	struct timeval *p_body = (struct timeval *)(buf + + sizeof(*p_hdr));
	int type = p_hdr->type;
	int code = p_hdr->code;

	if (gettimeofday(&now, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	if (type == ICMP_ECHOREPLY) {
		printf("%d bytes from %s: ", packet_len, s_info->str_sin_addr);
		printf("icmp_seq=%d ttl=%d ", p_hdr->un.echo.sequence, ttl);
		printf("time=%ld us\n", now.tv_usec - p_body->tv_usec);
	} else {
		/* If error, jump to ICMP sent packet header stored in body */
		p_hdr = (struct icmphdr *)(buf + sizeof(*p_hdr)
		        + sizeof(struct iphdr));
		printf("From %s: ", s_info->str_sin_addr);
		printf("icmp_seq=%d ", p_hdr->un.echo.sequence);
		print_icmp_err(type, code);
	}
	return 0;
}

/**
 * Print the different statistics for all send packets at the end of ft_ping
 * command.
 */
void print_end_info(const struct sockinfo *s_info,
		    const struct packinfo *p_info)
{
	int ms = calc_ms_elapsed(&p_info->time_first_send,
				 &p_info->time_last_send);

	printf("\n--- %s ping statistics ---\n", s_info->host);
	printf("%d packets transmitted, %d received, ", p_info->nb_send,
	       p_info->nb_ok);

	if (p_info->nb_err)
		printf("+%d errors, ", p_info->nb_err);

	printf("%d%% packet loss, time %dms\n",
	       (int)calc_perc_packet_loss(p_info), ms);

	if (p_info->nb_ok)
		printf("rtt min/avg/max/stddev = xxx/xxx/xxx/xxx ms\n");
}