#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "myv4l2.h"
#include "formatconverter.h"
#include <string.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//void savebmp(uchar * pdata, char * bmp_file, int width, int height )
//{      //分别为rgb数据，要保存的bmp文件名，图片长宽
//       int size = width*height*3*sizeof(char); // 每个像素点3个字节
//       // 位图第一部分，文件信息
//       BITMAPFILEHEADER bfh;
//       bfh.bfType = (WORD)0x4d42;  //bm
//       bfh.bfSize = size  // data size
//              + sizeof( BITMAPFILEHEADER ) // first section size
//              + sizeof( BITMAPINFOHEADER ) // second section size
//              ;
//       bfh.bfReserved1 = 0; // reserved
//       bfh.bfReserved2 = 0; // reserved
//       bfh.bfOffBits = sizeof( BITMAPFILEHEADER )+ sizeof( BITMAPINFOHEADER );//真正的数据的位置

//       // 位图第二部分，数据信息
//       BITMAPINFOHEADER bih;
//       bih.biSize = sizeof(BITMAPINFOHEADER);
//       bih.biWidth = width;
//       bih.biHeight = -height;//BMP图片从最后一个点开始扫描，显示时图片是倒着的，所以用-height，这样图片就正了
//       bih.biPlanes = 1;//为1，不用改
//       bih.biBitCount = 24;
//       bih.biCompression = 0;//不压缩
//       bih.biSizeImage = size;
//       bih.biXPelsPerMeter = 2835 ;//像素每米
//       bih.biYPelsPerMeter = 2835 ;
//       bih.biClrUsed = 0;//已用过的颜色，24位的为0
//       bih.biClrImportant = 0;//每个像素都重要
//       FILE * fp = fopen( bmp_file,"wb" );
//       if( !fp ) return;

//       fwrite( &bfh, 8, 1,  fp );//由于linux上4字节对齐，而信息头大小为54字节，第一部分14字节，第二部分40字节，所以会将第一部分补齐为16自己，直接用sizeof，打开图片时就会遇到premature end-of-file encountered错误
//       fwrite(&bfh.bfReserved2, sizeof(bfh.bfReserved2), 1, fp);
//       fwrite(&bfh.bfOffBits, sizeof(bfh.bfOffBits), 1, fp);
//       fwrite( &bih, sizeof(BITMAPINFOHEADER),1,fp );
//       fwrite(pdata,size,1,fp);
//       fclose( fp );
//}

typedef unsigned char BYTE;
//typedef unsigned short WORD;
//typedef unsigned int DWORD;
//typedef unsigned long LONG;

typedef int LONG;
typedef unsigned int DWORD;
typedef unsigned short WORD;

#pragma pack(1)

//typedef struct tagBITMAPFILEHEADER {
//    WORD        bfType;
//    DWORD       bfSize;
//    WORD        bfReserved1;
//    WORD        bfReserved2;
//    DWORD       bfOffBits;
//} BITMAPFILEHEADER;

//typedef struct tagBITMAPINFOHEADER {
//    DWORD       biSize;
//    LONG        biWidth;
//    LONG        biHeight;
//    WORD        biPlanes;
//    WORD        biBitCount;
//    DWORD       biCompression;
//    DWORD       biSizeImage;
//    LONG        biXPersPerMeter;
//    LONG        biYPersPerMeter;
//    DWORD       biClrUsed;
//    DWORD       biClrImportant;
//} BITMAPINFOHEADER;

//typedef struct tagRGBQUAD {
//    BYTE        rgbBlue;
//    BYTE        rgbGreen;
//    BYTE        rgbRed;
//    BYTE        rgbReserved;
//} RGBQUAD;


typedef struct {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BMPFILEHEADER_T;

typedef struct{
        DWORD      biSize; //4
        LONG       biWidth; //4
        LONG       biHeight;    //4
        WORD       biPlanes;    //2
        WORD       biBitCount;  //2
        DWORD      biCompression;   //4
        DWORD      biSizeImage; //4
        LONG       biXPelsPerMeter; //4
        LONG       biYPelsPerMeter; //4
        DWORD      biClrUsed;   //4
        DWORD      biClrImportant;  //4
} BMPINFOHEADER_T;

#pragma pack()



//void SaveImage(unsigned char *pData, int width, int height, std::string strPath)
//{
//    BITMAPFILEHEADER targetfileheader;
//    BITMAPINFOHEADER targetinfoheader;
//    memset(&targetfileheader, 0, sizeof(BITMAPFILEHEADER));
//    memset(&targetinfoheader, 0, sizeof(BITMAPINFOHEADER));

//    targetfileheader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
//    targetfileheader.bfSize = width * height + sizeof(RGBQUAD) * 256 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
//    targetfileheader.bfReserved1 = 0;
//    targetfileheader.bfReserved2 = 0;
//    targetfileheader.bfType = 0x4D42;

//    targetinfoheader.biBitCount = 8;
//    targetinfoheader.biSize = sizeof(BITMAPINFOHEADER);
//    targetinfoheader.biHeight = height;
//    targetinfoheader.biWidth = width;
//    targetinfoheader.biPlanes = 1;
//    targetinfoheader.biCompression = 0;
//    targetinfoheader.biSizeImage = 0;
//    targetinfoheader.biXPersPerMeter = 0;
//    targetinfoheader.biYPersPerMeter = 0;
//    targetinfoheader.biClrImportant = 0;
//    targetinfoheader.biClrUsed = 0;

//    RGBQUAD rgbquad[256];
//    for (int i = 0; i < 256; i++) {
//        rgbquad[i].rgbBlue = i;
//        rgbquad[i].rgbGreen = i;
//        rgbquad[i].rgbRed = i;
//        rgbquad[i].rgbReserved = 0;
//    }

//    FILE *fp = fopen(strPath.c_str(), "wb");
//    fwrite(&targetfileheader, sizeof(BITMAPFILEHEADER), 1, fp);
//    fwrite(&targetinfoheader, sizeof(BITMAPINFOHEADER), 1, fp);
//    fwrite(&rgbquad, sizeof(RGBQUAD), 256, fp);

//    for (int i = 0; i < height; i++) {
//        fwrite(pData + (height - 1 - i) * width, width, 1, fp);
//    }

//    fclose(fp);
//}

class MainWindow : public QMainWindow, MyV4l2::CaptureCallback
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int openV4l2Device();
    int closeV4l2Device();

    int startCapture();
    void stopCapture();

    int saveFrameAsBmpFile(unsigned char *pData, int width, int height, std::string strPath);

    int showDevices();

    void initWidgets();

    int yuvtorgb0(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
    int yuvtorgb(int y, int u, int v);

    virtual void callback(RawVideoData *data);

private slots:

    void on_mBtnSetText_clicked();

    void on_mBtnInsertText_clicked();

    void on_mBtnOpenDevice_clicked();

    void on_mBtnStartCapture_clicked();

    void on_mBtnCloseDevice_clicked();

    void on_mBtnSaveBmp_clicked();

    void on_mBtnStopCapture_clicked();

    void on_mBtnShowDevices_clicked();

    void on_mSliderBrightness_valueChanged(int value);

    int setBrightness(int value);
    int setSaturation(int value);
    int setContrast(int value);
    int setHue(int value);

private:
    Ui::MainWindow *ui;

    MyV4l2 m_v4l2;
//    int m_fd;
//    MyV4l2::v4l2_buf *m_bufs;

//    MyV4l2::v4l2_buf_unit *m_unit;
    FormatConverter m_convert;

    MyThread *m_thread;

    bool m_sign3;
};
#endif // MAINWINDOW_H
