#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <Windows.h>
#include <string>
#include <time.h>

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
time_t timep;
char recvBuf[1024] = {0};
char sendBuf[1024] = "Nice to meet you, too!";
bool timeOutStatus = false;

/*
name 截图文件的名称
desx 从源上裁剪的起始x位置
desy 从源上裁剪的起始y位置
desW 截图的宽度
desH 截图的高度
*/

HDC screenDC, memDC;
HBITMAP hBitmap, hBitmapOld;
int desx, desy, desW, desH;
BITMAPINFOHEADER bi;
DWORD dwBmpSize;
char *lpbitmap;

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

void UDPTimeout()
{
    while (timeOutStatus == false)
    {
        int recvLen = recvfrom(m_Socket, recvBuf, 1024, 0, (sockaddr *)&m_RemoteAddress, &m_RemoteAddressLen);
        if (recvLen > 0)
        {
            time(&timep);
        }
    }
    return;
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
        time(&timep);
        int ltime = timep;
        timeOutStatus = false;
        thread TimeOut(UDPTimeout);
        TimeOut.detach();
        while (timep - ltime <= 60)
        {
            char *curPic = new char[60000];
            memcpy(curPic, lpbitmap, 58571);
            for (int i = 0; i < 60; i++)
            {
                sendBuf[0] = (char)i;
                for (int j = 0; j < 1000; j++)
                {
                    sendBuf[1 + j] = curPic[i * 1000 + j];
                }
                int sendLen = sendto(m_Socket, sendBuf, strlen(sendBuf), 0, (sockaddr *)&m_RemoteAddress, m_RemoteAddressLen);
                /*
                if (sendLen > 0)
                {
                    cout << "发送到远程端的信息： " << sendBuf << endl;
                }
                */
            }
            time(&timep);
        }
        cout << "Err: timeout." << endl;
        timeOutStatus = true;
    }
}

void UDPCleanUp()
{
    closesocket(m_Socket);
    WSACleanup();
}

//------------------------------------up:UDP down:Pic---------------------------------------------

int CaptureScreenInit()
{
    desx = 0;
    desy = 0;
    desW = 1920;
    desH = 1080;
    // 获取屏幕的句柄
    screenDC = GetDC(NULL);
    // 需要注意的是对于多现实器，是从主显示器左上角为（0,0）延伸的，可以根据具体业务设置
    /*
    假如需要对屏幕进行比较多的gdi函数操作，如果每一步操作都直接对屏幕dc进行操作，
    那出现的大多数可能性都是屏幕的闪烁。
    一个很好的解决方法就是使用内存dc，将这些操作全部先在内存dc上操作，
    然后依次性在屏幕上进行操作。
    */

    return 0;
}

int TempReleaseCapture()
{
    // 清理资源
    SelectObject(memDC, hBitmapOld);
    return 0;
}

int CaptureScreen()
{
    memDC = CreateCompatibleDC(screenDC);
    HANDLE hDib;
    /*
    函数功能：该函数创建与指定的设备环境相关的设备兼容的位图。
    hdc：设备环境句柄。
    nWidth：指定位图的宽度，单位为像素。
    nHeight：指定位图的高度，单位为像素。
    返回值：如果函数执行成功，那么返回值是位图的句柄；
    如果函数执行失败，那么返回值为NULL。若想获取更多错误信息，请调用GetLastError。
    */
    hBitmap = CreateCompatibleBitmap(screenDC, desW, desH);
    // 该函数选择一对象到指定的设备上下文环境中，该新对象替换先前的相同类型的对象。
    hBitmapOld = (HBITMAP)SelectObject(memDC, hBitmap);
    /*
    BOOL BitBlt(
        HDC   hdcDest,   // 目标设备上下文的句柄
        int   nXDest,    // 目标矩形的左上角 x 坐标
        int   nYDest,    // 目标矩形的左上角 y 坐标
        int   nWidth,    // 目标矩形的宽度
        int   nHeight,   // 目标矩形的高度
        HDC   hdcSrc,    // 源设备上下文的句柄
        int   nXSrc,     // 源矩形的左上角 x 坐标
        int   nYSrc,     // 源矩形的左上角 y 坐标
        DWORD dwRop      // 光栅操作码，指定如何进行位图数据的传输
    );*/
    // 将指定区域的图像内存复制到位图，SRCCOPY代表复制操作枚举
    BitBlt(memDC, 0, 0, desW, desH, screenDC, 0, 0, SRCCOPY); // typedef struct tagBITMAPINFOHEADER {
    //    DWORD biSize;          // 结构的大小，通常为 40 字节
    //    LONG  biWidth;         // 位图的宽度（以像素为单位）
    //    LONG  biHeight;        // 位图的高度（以像素为单位）
    //    WORD  biPlanes;        // 目标设备的平面数，通常为 1
    //    WORD  biBitCount;      // 每个像素的位数，通常为 1、4、8、16、24 或 32
    //    DWORD biCompression;   // 图像的压缩方式，通常为 0（不压缩）或其他压缩方式的标志
    //    DWORD biSizeImage;     // 图像的大小，以字节为单位（可以是0，不常用）
    //    LONG  biXPelsPerMeter; // 水平分辨率，每米的像素数
    //    LONG  biYPelsPerMeter; // 垂直分辨率，每米的像素数
    //    DWORD biClrUsed;       // 位图使用的颜色索引数，0 表示使用所有可用颜色索引
    //    DWORD biClrImportant;  // 对图像显示有重要影响的颜色索引数，0 表示所有颜色都重要
    //} BITMAPINFOHEADER, * PBITMAPINFOHEADER;
    // 保存文件
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = desW;
    bi.biHeight = desH;
    /*
    位图的高度，以像素为单位。如果值为正，表示图像是从顶部到底部排列的；
    如果值为负，表示图像是从底部到顶部排列的。通常，正值是常见的。
    */
    bi.biPlanes = 1;    // 目标设备的平面数，通常为1。在绝大多数情况下，这个值是1。
    bi.biBitCount = 24; // 24位彩色与32位区别在于不需要透明通道
    bi.biCompression = BI_RGB;
    /*
    常见的压缩方式包括BI_RGB（无压缩）和BI_RLE8（8位运行长度编码）。
    BI_RLE8使用这种模式保存的图片有问题
    */
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwBmpSize = ((desW * bi.biBitCount + 31) / 32) * 4 * desH;
    hDib = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char *)GlobalLock(hDib);
    GetDIBits(screenDC, hBitmap, 0, desH, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS); // screenDC
    TempReleaseCapture();
    GlobalUnlock(hDib);
    GlobalFree(hDib);
    return 0;
}

int FinishCapture()
{
    // 清理资源
    SelectObject(memDC, hBitmapOld);
    DeleteDC(memDC);
    ReleaseDC(NULL, screenDC);
    DeleteObject(hBitmap);
    return 0;
}

int PrintCharedPicture()
{
    // char *buffer = (char *)&lpbitmap;
    // printf("%s\n", lpbitmap);
    // printf("%d\n", strlen(lpbitmap));
    return 0;
}

void CaptureScreenLoop(int interval)
{
    CaptureScreenInit();
    while (true)
    {
        Sleep(interval);
        CaptureScreen();
    }
    return;
}

int main()
{
    UDPServerInit();
    thread UDPServer(UDPSendLoop);
    thread Capture(CaptureScreenLoop, 10);
    Capture.detach();
    UDPServer.detach();
    while (true)
    {
        true;
    }
    UDPCleanUp();
    FinishCapture();
    return 0;
}