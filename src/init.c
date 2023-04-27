/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:29:06 by llefranc          #+#    #+#             */
/*   Updated: 2023/04/27 19:28:24 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

/**
 * Check if ping is running with root rights and if destination address is
 * present.
 * @ac: number of arguments provided to ft_ping.
 *
 * Return: 0 on success, -1 on error.
 */
int ping_check(int ac)
{
	if (getuid() != 0) {
		printf("ft_ping: usage error: need to be run as root\n");
		return -1;
	}
	if (ac == 1) {
		printf("ft_ping: usage error: Destination address required\n\n");
		return -1;
	}
	return 0;
}

/**
 * Init the internet remote socket address and its string representation based
 * on host provided as argument (hosts ex: 127.0.0.1 or google.com).
 */
static int init_sock_addr(struct sockinfo *s_info)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_RAW,
		.ai_protocol = IPPROTO_ICMP
	};
	struct addrinfo *tmp;
	int ret;

	if ((ret = getaddrinfo(s_info->host, NULL, &hints, &tmp)) != 0) {
		printf("getaddrinfo: %s\n", gai_strerror(ret));
		return -1;
	}
	s_info->remote_addr = *(struct sockaddr_in *)tmp->ai_addr;
	freeaddrinfo(tmp);

	if (inet_ntop(AF_INET, &s_info->remote_addr.sin_addr,
	    s_info->str_sin_addr, sizeof(s_info->str_sin_addr)) == NULL) {
		printf("inet_ntop: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

/**
 * Create a RAW socket with ICMP protocol and set the TTL value for ip layer.
 */
static int create_socket(uint8_t ttl)
{
	int sock_fd;

	if ((sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		printf("socket: %s\n", strerror(errno));
		return -1;
	}
	if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) {
		printf("setsockopt: %s\n", strerror(errno));
		close(sock_fd);
		return -1;
	}
	return sock_fd;
}

/**
 * Does several initialization:
 *	- Initialize sockinfo/packinfo structures.
 * 	- Find internet remote socket address based on host (ex: google.com).
 *	- Create raw socket with ICMP protocol.
 *	- Set the ttl for ip layer.
 * @sock_fd: Will be initialized with raw socket file descriptor.
 * @s_info: Will be init with socket remote address info.
 * @p_info: Init first_time_send member to now.
 * @host: The host to ping.
 * @ttl: Ttl value for ip layer.
 *
 * Return: 0 on success, -1 if the initialization failed. On failure, the file
 *         descriptor is automatically closed.
 */
int ping_init(int *sock_fd, struct sockinfo *s_info, struct packinfo *p_info,
	      char *host, int ttl)
{
	if (gettimeofday(&p_info->time_first_send, NULL) == -1) {
		printf("gettimeofday: %s\n", strerror(errno));
		return -1;
	}
	s_info->host = host;
	if (init_sock_addr(s_info) == -1)
		return -1;
	if ((*sock_fd = create_socket(ttl)) == -1)
		return -1;
	return 0;
}
