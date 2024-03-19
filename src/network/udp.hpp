#ifndef UDP_HPP
#define UDP_HPP
#include <WinSock2.h>
#include "../driverlog.h"
#pragma comment (lib, "ws2_32.lib")  //���� ws2_32.dll
class UdpSocketClass
{
public:
	static inline SOCKET createUdp() {
		/* ����winsock�� */
		WSADATA  wsaData;
		if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			DriverLog("Failed to init Network DDL!\n");
			return INVALID_SOCKET;
		}
		SOCKET socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (INVALID_SOCKET == socketfd)
		{
			//����socketʧ��
			DriverLog("Failed to create socket, error code:%d.\n", WSAGetLastError());
			WSACleanup();
			return socketfd;
		}
		//����Ϊ������
		u_long imode = 1;
		if (SOCKET_ERROR == ioctlsocket(socketfd, FIONBIO, &imode))
		{
			closesocket(socketfd);
			socketfd = INVALID_SOCKET;
			DriverLog("Failed to set socket FIONBIO.\n");
			WSACleanup();
		}
		return socketfd;
	}
};
#endif
