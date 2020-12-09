#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "myv4l2.h"
#include "formatconverter.h"
#include "x264encoder.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef unsigned char BYTE;
typedef int LONG;
typedef unsigned int DWORD;
typedef unsigned short WORD;

#pragma pack(1)

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

class MainWindow : public QMainWindow, MyV4l2::CaptureCallback,
                    x264Encoder::Callback
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    virtual void callback(RawVideoData *data);
    virtual void onEncodedCallback(unsigned char *data, int len);

private slots:
    void on_mBtnPreview_clicked();

    void on_mBtnSnapshot_clicked();

    void on_mBtnExit_clicked();

    int saveFrameAsBmpFile(unsigned char *pData, int width, int height, std::string strPath);

    void on_mBtnEncode_clicked();

    void on_mBtnRecord_clicked();

private:
    Ui::MainWindow *ui;

    MyV4l2 m_v4l2;
    FormatConverter m_convert;

    FILE *m_out_file;
    x264Encoder m_encoder;
};
#endif // MAINWINDOW_H
