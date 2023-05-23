# ft_ping (@42Paris)

> This project was code for Linux. Here is the [subject][1].
>

![Alt text](https://github.com/llefranc/42_ft_ping/blob/main/ft_ping_example.png)

## About

ft_ping is a reimplementation in C of the ping utility based on the ping implementation from inetutils-2.0.

ft_ping uses a raw socket to send each second an ICMP Echo Request packet with a timestamp in its body.
It will also display a line of information for each received ICMP packet.

It accepts a simple IPv4 address or hostname as a parameter, and supports both numerical IP addresses and hostnames.

> The program should be run with appropriate permissions, as sending ICMP Echo Request packets may require administrative privileges.

When receiving an ICMP packet, ft_ping will check:
- If the packet was correctly addressed to this process by checking the PID stored in the ID field of ICMP Echo Request.
- If it's an error packet or not.

ft_ping will ping the targeted host until `Ctrl+C` is pressed. It will then display statistics about the received ICMP Echo Response packets.

ft_ping supports also the following options :
- `-h`: provides help information about the usage and command-line options of ft_ping.
- `-q`: enables quiet output, ft_ping will only display the end statistics.
- `-v`: enables verbose output and allows viewing the results in case of problems or errors related to the packets.

*Example of error packet (packet filtered) with verbose output*

![Alt text](https://github.com/llefranc/42_ft_ping/blob/main/ft_ping_example2.png)

## Building and running the project

1. Download/Clone this repo

        git clone https://github.com/llefranc/42_ft_ping

2. `cd` into the root directory and run `make`

        cd 42_ft_ping
        make

3. Run `ft_ping` with appropriate permissions

		sudo ./ft_ping 192.168.0.1
		sudo ./ft_ping localhost
		sudo ./ft_ping google.com -v

## Sources

- [RFC of ICMP protocol][2]
- [RFC of IP protocol][3]

[1]: https://github.com/llefranc/42_ft_ping/blob/main/ft_ping.en.subject.pdf
[2]: https://www.rfc-editor.org/rfc/rfc792
[3]: https://www.rfc-editor.org/rfc/rfc791