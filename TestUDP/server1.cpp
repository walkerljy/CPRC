#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>

using namespace std;

// 添加动态库的lib
#pragma comment(lib, "ws2_32.lib")

SOCKET m_Socket;
SOCKADDR_IN m_BindAddress;   // 绑定地址
SOCKADDR_IN m_RemoteAddress; // 远程地址
int m_RemoteAddressLen;

// socket环境
WSADATA wsaData;

const char *ip = "127.0.0.1";
int port = 1000;
int ret;
char recvBuf[1024] = {0};
char sendBuf[1024] = "Nice to meet you, too!";

bool UDPServerInit()
{
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

    // 绑定占用<ip, port>
    m_BindAddress.sin_family = AF_INET;
    m_BindAddress.sin_addr.S_un.S_addr = inet_addr(ip);
    m_BindAddress.sin_port = htons(port);
    ret = bind(m_Socket, (sockaddr *)&m_BindAddress, sizeof(SOCKADDR));
    if (ret == SOCKET_ERROR)
    {
        closesocket(m_Socket);
        m_Socket = INVALID_SOCKET;
        return false;
    }
    m_RemoteAddressLen = sizeof(m_RemoteAddress);
    return true;
}

void UDPSendLoop()
{
    std::printf("已设置绑定占用的连接, 其ip: %s, port: %d\n", inet_ntoa(m_BindAddress.sin_addr), ntohs(m_BindAddress.sin_port));

    while (1)
    {
        int recvLen = recvfrom(m_Socket, recvBuf, 1024, 0, (sockaddr *)&m_RemoteAddress, &m_RemoteAddressLen);
        if (recvLen > 0)
        {
            printf("接收到一个连接, 其ip: %s, port: %d\n", inet_ntoa(m_RemoteAddress.sin_addr), ntohs(m_RemoteAddress.sin_port));
            cout << "接收到一个信息： " << recvBuf << endl;
        }
        int sendLen = sendto(m_Socket, sendBuf, strlen(sendBuf), 0, (sockaddr *)&m_RemoteAddress, m_RemoteAddressLen);
        if (sendLen > 0)
        {
            cout << "发送到远程端的信息： " << sendBuf << endl;
        }
    }
}

void UDPCleanUp(){
    closesocket(m_Socket);
    WSACleanup();
}

int main()
{
    UDPServerInit();
    thread UDPServer(UDPSendLoop);
    UDPServer.detach();
    while(true){
        true;
    }
    return 0;
}