/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:29:34 by llefranc          #+#    #+#             */
/*   Updated: 2023/04/28 14:49:57 by llefranc         ###   ########.fr       */
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
void print_packet(enum e_packtype type, const uint8_t *buf, int packet_len)
{
	const struct icmphdr *hdr;
	char *step_str = type == E_PACK_SEND ? "SENT" : "RECEIVED";

	/* Skipping IP header for received packets */
	if (type == E_PACK_RECV) {
		buf += sizeof(struct iphdr);
		packet_len -= sizeof(struct iphdr);
	}
	hdr = (const struct icmphdr *)(buf);

	printf("----------------------------------------------\n");
	printf("Packet state: %s\n", step_str);
	printf("ICMP type: %d\n", hdr->type);
	printf("ICMP code: %d\n", hdr->code);
	printf("ICMP checksum: %d\n", hdr->checksum);
	printf("ICMP id: %d\n", hdr->un.echo.id);
	printf("ICMP sequence: %d\n", hdr->un.echo.sequence);
	printf("ICMP packet size: %d (bytes)\n\n", packet_len);

	printf("Packet content (ICMP header + body):\n");
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
	return (1.0 - (float)(p->nb_recv_ok) / (float)p->nb_send) * 100.0;
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
 * Print number of bytes and rtt of a received packet.
 */
int print_recv_info(const struct sockinfo *s_info, const uint8_t *buf,
		    int packet_len)
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

	printf("%ld bytes from %s: ", packet_len - sizeof(*p_iph),
	       s_info->str_sin_addr);
	printf("icmp_seq=%d ttl=%d ", p_icmph->un.echo.sequence, p_iph->ttl);
	printf("time=%ld us\n", now.tv_usec - p_body->tv_usec);
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
	       p_info->nb_recv_ok);

	if (p_info->nb_recv_err)
		printf("+%d errors, ", p_info->nb_recv_err);

	printf("%d%% packet loss, time %dms\n",
	       (int)calc_perc_packet_loss(p_info), ms);

	if (p_info->nb_recv_ok)
		printf("rtt min/avg/max/stddev = xxx/xxx/xxx/xxx ms\n");
}