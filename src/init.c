/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/26 16:29:06 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/15 21:46:42 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>


/**
 * Init the internet remote socket address and its string representation based
 * on host provided as argument (hosts ex: 127.0.0.1 or google.com).
 */
static int init_sock_addr(struct sockinfo *si)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_RAW,
		.ai_protocol = IPPROTO_ICMP
	};
	struct addrinfo *tmp;

	if (getaddrinfo(si->host, NULL, &hints, &tmp) != 0) {
		printf("ft_ping: unknown host\n");
		return -1;
	}
	si->remote_addr = *(struct sockaddr_in *)tmp->ai_addr;
	freeaddrinfo(tmp);

	if (inet_ntop(AF_INET, &si->remote_addr.sin_addr, si->str_sin_addr,
	    INET_ADDRSTRLEN) == NULL) {
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
 * Do the socket initializations:
 * 	- Find internet remote socket address based on host (ex: google.com).
 *	- Create raw socket with ICMP protocol.
 *	- Set the ttl for ip layer.
 * @sock_fd: Will be init with raw socket file descriptor.
 * @si: Will be init with socket remote address infos.
 * @host: The host to ping.
 * @ttl: Ttl value for ip layer.
 *
 * Return: 0 on success, -1 if the initialization failed. On failure, the file
 *         descriptor is automatically closed.
 */
int init_sock(int *sock_fd, struct sockinfo *si, char *host, int ttl)
{
	si->host = host;
	if (init_sock_addr(si) == -1)
		return -1;
	if ((*sock_fd = create_socket(ttl)) == -1)
		return -1;
	return 0;
}
