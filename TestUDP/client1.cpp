#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream>

using namespace std;

// 添加动态库的lib
#pragma comment(lib, "ws2_32.lib")
/*
void inet_pton(int af, const char *src, void *dst)
{
    struct sockaddr_in ip4;
    struct sockaddr_in6 ip6;
    char ipaddr[64];
    sprintf(ipaddr, "%s:2020", src);
    int addr_size = -1;
    if (af == AF_INET)
    {
        addr_size = sizeof(SOCKADDR_IN);
        ;
        WSAStringToAddress((LPSTR)ipaddr, af, NULL, (LPSOCKADDR)&ip4, &addr_size);
        memcpy(dst, &(ip4.sin_addr), 4);
    }
    else if (af == AF_INET6)
    {
        addr_size = sizeof(SOCKADDR_IN6);
        WSAStringToAddress((LPSTR)ipaddr, af, NULL, (LPSOCKADDR)&ip6, &addr_size);
        memcpy(dst, &(ip6.sin6_addr), 16);
    }
    // printf("ipaddr len=%d\nAF_INET6=%d,AF_INET=%d,af=%d\n",addr_size,AF_INET6,AF_INET,af);
}
*/
int main()
{
    SOCKET m_Socket;
    SOCKADDR_IN m_RemoteAddress; // 远程地址
    int m_RemoteAddressLen;

    // socket环境
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup error:" << GetLastError() << endl;
        return false;
    }

    // socket对象
    m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_Socket == INVALID_SOCKET)
    {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
        return false;
    }

    // 远端地址
    const char *ip = "127.0.0.1";
    int port = 1000;
    m_RemoteAddress.sin_family = AF_INET;
    m_RemoteAddress.sin_port = htons(port);
    m_RemoteAddressLen = sizeof(m_RemoteAddress);
    inet_pton(AF_INET, ip, &m_RemoteAddress.sin_addr);

    // 接收和发送
    char recvBuf[1024] = {0};
    char sendBuf[1024] = "Nice to meet you!";

    while (1)
    {
        int sendLen = sendto(m_Socket, sendBuf, strlen(sendBuf), 0, (sockaddr *)&m_RemoteAddress, m_RemoteAddressLen);
        if (sendLen > 0)
        {
            std::printf("发送到远程端连接, 其ip: %s, port: %d\n", inet_ntoa(m_RemoteAddress.sin_addr), ntohs(m_RemoteAddress.sin_port));
            cout << "发送到远程端的信息： " << sendBuf << endl;
        }

        int recvLen = recvfrom(m_Socket, recvBuf, 1024, 0, NULL, NULL);
        if (recvLen > 0)
        {
            std::printf("接收到一个连接, 其ip: %s, port: %d\n", inet_ntoa(m_RemoteAddress.sin_addr), ntohs(m_RemoteAddress.sin_port));
            cout << "接收到一个信息： " << recvBuf << endl;
        }
    }

    closesocket(m_Socket);
    WSACleanup();
    return true;
}