#include <Windows.h>
#include <string>
#include <unistd.h>
using namespace std;

/*
name 截图文件的名称
desx 从源上裁剪的起始x位置
desy 从源上裁剪的起始y位置
desW 截图的宽度
desH 截图的高度
*/

HDC screenDC,memDC;
HBITMAP hBitmap,hBitmapOld;
int desx, desy, desW, desH;

int CaptureScreenInit(){
    desx=0;
    desy=0;
    desW=1920;
    desH=1080;
    //获取屏幕的句柄
    screenDC = GetDC(NULL);
    //需要注意的是对于多现实器，是从主显示器左上角为（0,0）延伸的，可以根据具体业务设置
    /*
    假如需要对屏幕进行比较多的gdi函数操作，如果每一步操作都直接对屏幕dc进行操作，
    那出现的大多数可能性都是屏幕的闪烁。
    一个很好的解决方法就是使用内存dc，将这些操作全部先在内存dc上操作，
    然后依次性在屏幕上进行操作。
    */
    memDC = CreateCompatibleDC(screenDC);
    /*
    函数功能：该函数创建与指定的设备环境相关的设备兼容的位图。
    hdc：设备环境句柄。
    nWidth：指定位图的宽度，单位为像素。
    nHeight：指定位图的高度，单位为像素。
    返回值：如果函数执行成功，那么返回值是位图的句柄；
    如果函数执行失败，那么返回值为NULL。若想获取更多错误信息，请调用GetLastError。
    */
   return 0;
}

int CaptureScreen()
{
    hBitmap = CreateCompatibleBitmap(screenDC, desW, desH);
    //该函数选择一对象到指定的设备上下文环境中，该新对象替换先前的相同类型的对象。
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
    //将指定区域的图像内存复制到位图，SRCCOPY代表复制操作枚举
    BitBlt(memDC, 0, 0, desW, desH, screenDC, 0, 0, SRCCOPY);
    return 0;
}

int SaveCapturedScreen(string outName){
    int screenW = desW;// GetDeviceCaps(screenDC, HORZRES);
    int screenH = desH;// GetDeviceCaps(screenDC, VERTRES);
        //typedef struct tagBITMAPINFOHEADER {
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
    //保存文件
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = desW;
    bi.biHeight = desH;
    /*
    位图的高度，以像素为单位。如果值为正，表示图像是从顶部到底部排列的；
    如果值为负，表示图像是从底部到顶部排列的。通常，正值是常见的。
    */
    bi.biPlanes = 1;//目标设备的平面数，通常为1。在绝大多数情况下，这个值是1。
    bi.biBitCount = 24;//24位彩色与32位区别在于不需要透明通道
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

    DWORD dwBmpSize = ((screenW * bi.biBitCount + 31) / 32) * 4 * desH;
    HANDLE hDib = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDib);
    GetDIBits(screenDC, hBitmap, 0, desH, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    FILE* file = fopen(outName.c_str(), "wb");
    if (file) {
        BITMAPFILEHEADER bf;
        /*
        它对应ASCII字符"M"和"B"，表示"BM"，
        这是Windows位图文件的标志。在二进制表示中，它实际上是字节序列0x42 0x4D。
        */
        bf.bfType = 0x4D42;
        bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
        bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, file);
        fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, file);
        fwrite(lpbitmap, dwBmpSize, 1, file);

        fclose(file);
    }
    GlobalUnlock(hDib);
    GlobalFree(hDib);
    return 0;
}

int FinishCapture(){
    //清理资源
    SelectObject(memDC, hBitmapOld);
    DeleteDC(memDC);
    ReleaseDC(NULL, screenDC);
    DeleteObject(hBitmap);
    return 0;
}

int main()
{
    CaptureScreenInit();
    CaptureScreen();
    SaveCapturedScreen("测试位图.bmp");
    usleep(1000);
    CaptureScreen();
    SaveCapturedScreen("测试位图2.bmp");
    FinishCapture();
    return 0;
}