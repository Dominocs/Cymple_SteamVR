#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <WinSock2.h>
#include <iostream>
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
class CSocket
{
public:
	static inline int selectAndRecv(SOCKET socketfd, char* pcData, uint32_t uiMaxLen)
	{
		fd_set rfd{};
		struct timeval timeout = { 0, 0 };
		FD_ZERO(&rfd);           //总是这样先清空一个描述符集
		FD_SET(socketfd, &rfd);
		/* 读取POSE */
		int SelectRcv = ::select((int)socketfd + 1, &rfd, 0, 0, &timeout); //检查该套接字是否可读)
		if (0 < SelectRcv)
		{
			SelectRcv = ::recv(socketfd, (char*)pcData, uiMaxLen, 0);
		}
		return SelectRcv;
	}
	static inline int sendto(SOCKET socketfd, char* pData, uint32_t uiLen, sockaddr_in* serverAddr)
	{
		static const int nServAddLen = sizeof(sockaddr_in);
		return ::sendto(socketfd, pData, uiLen, 0, (SOCKADDR *)serverAddr, nServAddLen);
	}
	static inline int bind(SOCKET socketfd, sockaddr_in* serverAddr) {
		int ret = ::bind(socketfd, (SOCKADDR*)serverAddr, sizeof(sockaddr_in));
		if (0 != ret) {
			DriverLog("Failed to bind socket, error code:%d.\n", WSAGetLastError());
		}
		return ret;
	}
};

#endif

