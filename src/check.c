/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/02 19:28:43 by llefranc          #+#    #+#             */
/*   Updated: 2023/05/11 21:15:41 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ping.h"

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

/* Each character correspond to an option supported by ft_ping */
static const char supported_opts[] = "hqv";

/**
 * Check if ping is running with root rights.
 * @ac: number of arguments provided to ft_ping.
 *
 * Return: 0 on success, -1 on error.
 */
int check_rights()
{
	if (getuid() != 0) {
		printf("ft_ping: usage error: need to be run as root\n");
		return -1;
	}
	return 0;
}

/**
 * Parse an option argument (argument that starts with '-' character) and set
 * the appropriates flags in struct options.
 */
static int parse_opt_arg(char *arg, struct options *opts)
{
	char *match = NULL;
	size_t len = strlen(arg);

	/* i = 1 because of '-' character before any option */
	for (size_t i = 1; i < len; ++i) {
		if ((match = strchr(supported_opts, arg[i])) != NULL) {
			switch (*match) {
			case 'h':
				opts->help = 1;
				break;
			case 'q':
				opts->quiet = 1;
				break;
			case 'v':
				opts->verb = 1;
				break;
			default:
				printf("ping: unknown option\n");
			}
		} else {
			printf("ft_ping: invalid option -- '%c'\n", arg[i]);
			printf("Try 'ft_ping -h' for more information.\n");
			return -1;
		}
	}
	return 0;
}

/**
 * Check each argument, init host paramater and ft_ping options. If there is
 * an unkown option or help option in the arguments, display help.
 * @ac: Number of arguments.
 * @av: Arguments of ft_ping.
 * @host: Will point to the last non-option argument.
 * @opts: Flags will be init based on options argument.
 *
 * Return: 0 on success, -1 if there is no argument or an unknow option, and 1
 *         if help option was met.
 */
int check_args(int ac, char **av, char **host, struct options *opts)
{
	int nb_host = 0;

	for (int i = 1; i < ac; ++i) {
		if (av[i][0] == '-' && strlen (av[i]) > 1) {
			if (parse_opt_arg(av[i], opts) == -1)
				return -1;
		} else {
			*host = av[i];
			++nb_host;
		}
		if (opts->help) {
			print_help();
			return 1;
		}
	}
	if (!nb_host) {
		printf("ft_ping: missing host operand\n");
		printf("Try 'ft_ping -h' for more information.\n");
		return -1;
	} else if (nb_host > 1) {
		printf("ft_ping: only one host is needed\n");
		printf("Try 'ft_ping -h' for more information.\n");
		return -1;
	}
	return 0;
}