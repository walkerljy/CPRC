#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream>
#include <thread>

using namespace std;

// 添加动态库的lib
#pragma comment(lib, "ws2_32.lib")

SOCKET m_Socket;
SOCKADDR_IN m_RemoteAddress; // 远程地址
int m_RemoteAddressLen;

// socket环境
WSADATA wsaData;
const char *ip = "127.0.0.1";
int port = 1000;

// 接收和发送
char recvBuf[1024] = {0};
char sendBuf[1024] = "Nice to meet you!";
char *curPic = new char[60000];

// int ProcessBuff(){
//     if(recvBuf[1024]!={0})
// }


int UDPCLientInit()
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

    // 远端地址
    m_RemoteAddress.sin_family = AF_INET;
    m_RemoteAddress.sin_port = htons(port);
    m_RemoteAddressLen = sizeof(m_RemoteAddress);
    inet_pton(AF_INET, ip, &m_RemoteAddress.sin_addr);
    return 0;
}

void UDPConnectLoop(){
    while (1)
    {
        int sendLen = sendto(m_Socket, sendBuf, strlen(sendBuf), 0, (sockaddr *)&m_RemoteAddress, m_RemoteAddressLen);
        if (sendLen > 0)
        {
            std::printf("发送到远程端连接, 其ip: %s, port: %d\n", inet_ntoa(m_RemoteAddress.sin_addr), ntohs(m_RemoteAddress.sin_port));
            cout << "发送到远程端的信息： " << sendBuf << endl;
        }
        Sleep(5000);
    }
}

void UDPClientLoop(){
    while (1)
    {
        int recvLen = recvfrom(m_Socket, recvBuf, 1024, 0, NULL, NULL);
        if (recvLen > 0)
        {
            //std::printf("接收到一个连接, 其ip: %s, port: %d\n", inet_ntoa(m_RemoteAddress.sin_addr), ntohs(m_RemoteAddress.sin_port));
            // cout << "接收到一个信息： " << ((int)recvBuf[0])-32 << endl;
        }
        recvBuf[1024] = {0};
    }
}

void UDPClose(){
    closesocket(m_Socket);
    WSACleanup();
    return;
}
int main(){
    UDPCLientInit();
    thread (UDPConnectLoop).detach();
    thread (UDPClientLoop).detach();
    while (true)
    {
        true;
    }
    UDPClose();
    return 0;
}