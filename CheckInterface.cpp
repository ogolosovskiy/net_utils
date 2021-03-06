
#include <cassert>
#include <string>
#include <cstdint>

#include<iostream>

#include <winsock2.h>
#include <ws2tcpip.h>


int find_interface(std::string target_fqdn, uint16_t target_port, sockaddr_storage& local_addr) 
{
	int error = 0;
	int test_sock = -1;
	addrinfo *boot_addr = nullptr;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	do {
		error = getaddrinfo(target_fqdn.c_str(), std::to_string(target_port).c_str(), &hints, &boot_addr);
		if (error) {
			std::cerr << "syscall getaddrinfo fails: " << gai_strerror(error) << std::endl;
			break;
		}

		test_sock = socket(boot_addr->ai_family, SOCK_DGRAM, IPPROTO_UDP);
		if (test_sock < 0) {
			std::cerr << "syscall socket fails: " << gai_strerror(errno) << std::endl;
			break;
		}

		error = connect(test_sock, boot_addr->ai_addr, boot_addr->ai_addrlen);
		if (error) {
			std::cerr << "syscall getaddrinfo fails: " << gai_strerror(errno) << std::endl;
			break;
		}

		socklen_t len;
		if (boot_addr->ai_family == AF_INET) {
			len = sizeof(sockaddr_in);
		}
		else if (boot_addr->ai_family == AF_INET6) {
			len = sizeof(sockaddr_in6);
		}
		error = getsockname(test_sock, (struct sockaddr *) &local_addr, &len);
		if (error) {
			std::cerr << "syscall getsockname fails: " << gai_strerror(errno) << std::endl;
			break;
		}
	} while (false);

	freeaddrinfo(boot_addr);
	closesocket(test_sock);

	return error;
}


std::string sockaddr_storage_to_host_name(const sockaddr_storage& sa)
{
	if (sa.ss_family == AF_INET6) {
		char buffer[INET6_ADDRSTRLEN];
		int error = getnameinfo((struct sockaddr const*)&sa, sizeof(sockaddr_in6), buffer, sizeof(buffer), 0, 0, NI_NUMERICHOST);
		if (error == 0)
			return buffer;
	}

	if (sa.ss_family == AF_INET) {
		char buffer[INET_ADDRSTRLEN];
		int error = getnameinfo((struct sockaddr const*)&sa, sizeof(sockaddr_in), buffer, sizeof(buffer), 0, 0, NI_NUMERICHOST);
		if (error == 0)
			return buffer;
	}

	return "";
}

int main()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cerr <<  "WSAStartup failed: " << iResult;
		return 1;
	}

	sockaddr_storage local_addr;
	find_interface("google.com", 80, local_addr);

	std::cout << "local interface: " << sockaddr_storage_to_host_name(local_addr) << std::endl;

    return 0;
}

