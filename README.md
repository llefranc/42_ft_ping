# ft_ping (@42Paris)

![Alt text](https://github.com/llefranc/42_ft_ping/blob/main/ft_ping_example.png)

## About

ft_ping is a reimplementation in C of the ping utility based on the ping implementation from inetutils-2.0.
ft_ping is using raw sockets to send ICMP echo messages, and is displaying the ICMP echo messages responses received.
Each second, a new ICMP echo packet will be send.

When receiving an ICMP packet, ft_ping is checking if this ICMP packet was correctly addressed to its process by using the ID field of ICMP echo messages, and if it's an error packet or not.

ft_ping will ping the targeted host until Ctrl+C is pressed. It will then display statistics about the received ICMP echo responses.

ft_ping can accept a simple IPv4 address or hostname as a parameter. It supports both numerical IP addresses and hostnames.

ft_ping supports the following options :
- `-h`: provides help information about the usage and command-line options of ft_ping.
- `-q`: enables quiet output, ft_ping will only display the end statistics.
- `-v`: enables verbose output and allows viewing the results in case of problems or errors related to the packets.

Here is the [subject][2].

### Building and running the project

css
Copy code
ft_ping [-v] [-h] <destination>
-v: Enables verbose output, providing detailed information about the packets and their responses in case of errors.
-h: Displays help information about the usage and options of ft_ping.
<destination>: Specifies the destination IP address or hostname to ping.
Examples
Ping an IP address:

Copy code
ft_ping 192.168.0.1
Ping a hostname:

Copy code
ft_ping example.com
Ping with verbose output:

Copy code
ft_ping -v example.com
Display help information:

Copy code
ft_ping -h
Additional Notes
This implementation of ft_ping is focused on IPv4 addresses and does not support IPv6.
The program should be run with appropriate permissions, as sending ICMP Echo Request packets may require administrative privileges.
Error messages and feedback will be displayed using the printf functions, providing clear information about any issues encountered.

## Sources

- [RFC of ICMP protocol][2]
- [RFC of IP protocol][3]

[1]: https://github.com/llefranc/42_ft_ping/blob/main/ft_ping.en.subject.pdf
[2]: https://www.rfc-editor.org/rfc/rfc792
[3]: https://www.rfc-editor.org/rfc/rfc791