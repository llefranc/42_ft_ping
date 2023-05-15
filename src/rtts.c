/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rtt_list.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/11 18:59:06 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/11 19:40:33 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

/**
 * Calculate the round-trip time value for a received ICMP packet.
 */
static int calc_packet_rtt(struct icmphdr *icmph, struct rtt_node *new_rtt)
{
	struct timeval *t_send;
	struct timeval t_recv;

	t_send = ((struct timeval *)skip_icmphdr(icmph));
	if (gettimeofday(&t_recv, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	timersub(&t_recv, t_send, &new_rtt->val);
	return 0;
}

/**
 * Calculate the round-trip time value for a received packet and add it at
 * the end of the linked list containing all the rtt values from previous
 * received ICMP packet.
 */
struct rtt_node * rtts_save_new(struct packinfo *pi, struct icmphdr *icmph)
{
	struct rtt_node *elem = pi->rtt_list;
	struct rtt_node *new_rtt = NULL;

	if ((new_rtt = malloc(sizeof(*new_rtt))) == NULL)
		return NULL;
	if (calc_packet_rtt(icmph, new_rtt) == -1)
		return NULL;
	new_rtt->next = NULL;
	if (elem != NULL) {
		while (elem->next)
			elem = elem->next;
		elem->next = new_rtt;
	} else {
		pi->rtt_list = new_rtt;
	}
	pi->rtt_last = new_rtt;
	return new_rtt;
}

/**
 * Free all nodes of the linked list containing all the round-trip time valus.
 */
void rtts_clean(struct packinfo *pi)
{
	struct rtt_node *elem = pi->rtt_list;
	struct rtt_node *tmp;

	while (elem) {
		tmp = elem;
		elem = elem->next;
		free(tmp);
	}
}

/**
 * Calculate the standard deviation based on average rtt.
 */
void calc_stddev(struct packinfo *pi, long nb_elem)
{
	struct rtt_node *elem = pi->rtt_list;
	struct timeval *avg = &pi->avg;
	long sec_dev = 0;
	long usec_dev = 0;
	long total_sec_dev = 0;
	long total_usec_dev = 0;

	while (elem) {
		sec_dev = elem->val.tv_sec - avg->tv_sec;
		sec_dev *= sec_dev;
		total_sec_dev += sec_dev;
		usec_dev = elem->val.tv_usec - avg->tv_usec;
		usec_dev *= usec_dev;
		total_usec_dev += usec_dev;
		elem = elem->next;
	}
	if (nb_elem - 1 > 0) {
		total_sec_dev /= nb_elem - 1;
		total_usec_dev /= nb_elem - 1;
		pi->stddev.tv_sec = (long)sqrt(total_sec_dev);
		pi->stddev.tv_usec = (long)sqrt(total_usec_dev);
	} else {
		pi->stddev.tv_sec = 0;
		pi->stddev.tv_usec = 0;
	}
}

/**
 * Fill the struct packinfo with several statistics calculated with round-trip
 * times from all received ICMP packets:
 *    - Minimum round-trip time value.
 *    - Maximum round-trip time value.
 *    - Average round-trip time value.
 *    - Standard deviation round-trip time value.
 */
void rtts_calc_stats(struct packinfo *pi)
{
	struct rtt_node *elem = pi->rtt_list;
	long nb_elem = 0;
	long total_sec = 0;
	long total_usec = 0;

	pi->min = &elem->val;
	pi->max = &elem->val;
	while (elem) {
		if (timercmp(pi->min, &elem->val, >))
			pi->min = &elem->val;
		else if (timercmp(pi->max, &elem->val, <))
			pi->max = &elem->val;

		total_sec += elem->val.tv_sec;
		total_usec += elem->val.tv_usec;
		if (total_usec > 100000) {
			total_usec -= 100000;
			++total_sec;
		}
		++nb_elem;
		elem = elem->next;
	}
	pi->avg.tv_sec = total_sec / nb_elem;
	pi->avg.tv_usec = total_usec / nb_elem;
	calc_stddev(pi, nb_elem);
}