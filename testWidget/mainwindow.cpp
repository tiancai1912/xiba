#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#define CAPTURE_WIDTH 160
#define CAPTURE_HEIGHT 120

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QString("T-King"));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_mBtnPreview_clicked()
{
    m_v4l2.openDevice("/dev/video0");
    m_v4l2.setCaptureCallback(this);
    m_v4l2.startCaptureLoop();
}

void MainWindow::on_mBtnSnapshot_clicked()
{
    RawVideoData *data = m_v4l2.getFrame();
    if (data->data != nullptr) {
        //save bmp
        unsigned char *src_buf[4];
        int src_linesize[4] = {240, 0, 0, 0};

        unsigned char *dst_buf[4];
        int dst_linesize[4] = {480, 0, 0, 0};

        src_buf[0] = (unsigned char *)(data->data);
        dst_buf[0] = (unsigned char *)malloc(CAPTURE_WIDTH * CAPTURE_HEIGHT * 3);
        m_convert.convert(src_buf, src_linesize, dst_buf, dst_linesize);
        saveFrameAsBmpFile((unsigned char *)(dst_buf[0]), CAPTURE_WIDTH, CAPTURE_HEIGHT, "./test.bmp");

        //show snapshot
        QImage image(dst_buf[0], CAPTURE_WIDTH, CAPTURE_HEIGHT, QImage::Format::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image);
        ui->mLabelSnapshot->setPixmap(pixmap);

        delete dst_buf[0];
        dst_buf[0] = nullptr;
    }
}

void MainWindow::on_mBtnExit_clicked()
{
    m_v4l2.stopCaptureLoop();
    m_v4l2.closeDevice();

    m_encoder.unInit();

    if (m_out_file != nullptr) {
        fclose(m_out_file);
        m_out_file = nullptr;
    }
}
int g_count = 0;
void MainWindow::callback(RawVideoData *data)
{
    if (data->data != nullptr) {
        unsigned char *src_buf[4];
        int src_linesize[4] = {240, 0, 0, 0};

        unsigned char *dst_buf[4];
        int dst_linesize[4] = {480, 0, 0, 0};

        src_buf[0] = (unsigned char *)(data->data);
        dst_buf[0] = (unsigned char *)malloc(CAPTURE_WIDTH * CAPTURE_HEIGHT * 3);
        m_convert.convert(src_buf, src_linesize, dst_buf, dst_linesize);

        QImage image(dst_buf[0], CAPTURE_WIDTH, CAPTURE_HEIGHT, QImage::Format::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image);
        ui->mLabelPreview->setPixmap(pixmap);

        x264Encoder::Out *out = m_encoder.encodeOneFrame(data->data, data->length);

        if (m_out_file != nullptr) {
            int ret = fwrite(out->data, 1, out->len, m_out_file);
            if (ret != out->len) {
                qDebug() << "write out file failed!" << "ret value: " << ret << endl;
            }
        }

        delete dst_buf[0];
        dst_buf[0] = nullptr;
    }
}

void MainWindow::onEncodedCallback(unsigned char *data, int len)
{

}

int MainWindow::saveFrameAsBmpFile(unsigned char *pData, int width, int height, std::string strPath)
{
    //分别为rgb数据，要保存的bmp文件名，图片长宽
    int size = width*height*3*sizeof(char); // 每个像素点3个字节
    // 位图第一部分，文件信息
    BMPFILEHEADER_T bfh;
    bfh.bfType = (WORD)0x4d42;  //bm
    bfh.bfSize = size  // data size
            + sizeof( BMPFILEHEADER_T ) // first section size
            + sizeof( BMPINFOHEADER_T ) // second section size
            ;

    printf("word size: %d, dword size: %d\n", sizeof(WORD), sizeof(DWORD));
    printf("the head size: %d and info size: %d\n", sizeof(BMPFILEHEADER_T), sizeof(BMPINFOHEADER_T));
    bfh.bfReserved1 = 0; // reserved
    bfh.bfReserved2 = 0; // reserved
    bfh.bfOffBits = 54;//真正的数据的位置

    // 位图第二部分，数据信息
    BMPINFOHEADER_T bih;
    bih.biSize = sizeof(BMPINFOHEADER_T);
    bih.biWidth = width;
    bih.biHeight = -height;//BMP图片从最后一个点开始扫描，显示时图片是倒着的，所以用-height，这样图片就正了
    bih.biPlanes = 1;//为1，不用改
    bih.biBitCount = 24;
    bih.biCompression = 0;//不压缩
    bih.biSizeImage = size + 54;
    bih.biXPelsPerMeter = 0 ;//像素每米
    bih.biYPelsPerMeter = 0 ;
    bih.biClrUsed = 0;//已用过的颜色，24位的为0
    bih.biClrImportant = 0;//每个像素都重要

    FILE *fp = fopen(strPath.c_str(), "wb");
    //           FILE * fp = fopen( bmp_file,"wb" );
    if( !fp ) return -1;

    //           fwrite( &bfh, 8, 1,  fp );//由于linux上4字节对齐，而信息头大小为54字节，第一部分14字节，第二部分40字节，所以会将第一部分补齐为16自己，直接用sizeof，打开图片时就会遇到premature end-of-file encountered错误
    //           fwrite(&bfh.bfReserved2, sizeof(bfh.bfReserved2), 1, fp);
    //           printf("the size11 : %d\n", sizeof(bfh.bfReserved2));

    //           fwrite(&bfh.bfOffBits, 54, 1, fp);
    //           printf("the size22 : %d\n", sizeof(bfh.bfOffBits));

    //           fwrite( &bih, sizeof(BMPINFOHEADER_T),1,fp );
    //           printf("the size33 : %d\n", sizeof(BMPINFOHEADER_T));

    fwrite(&bfh, sizeof(BMPFILEHEADER_T), 1, fp);
    printf("the size11: %d\n", sizeof(BMPFILEHEADER_T));
    fwrite(&bih, sizeof(BMPINFOHEADER_T), 1, fp);
    printf("the size22: %d\n", sizeof(BMPINFOHEADER_T));

    printf("the pdata length: %d\n", strlen((const char *)pData));

    fwrite(pData,size,1,fp);
    fclose( fp );

    return 0;
}

void MainWindow::on_mBtnEncode_clicked()
{
    x264Encoder::EncodingFormat format;
    format.width = CAPTURE_WIDTH;
    format.height = CAPTURE_HEIGHT;
    format.format = X264_CSP_YUYV;
    format.bitDepth = 8;
//    format.path = "test.yuyv";

    //open encoder
    m_encoder.init(format);
    m_encoder.setCallback(this);
}

void MainWindow::on_mBtnRecord_clicked()
{
    m_out_file = fopen("./test.h264", "wb");
    if (m_out_file == nullptr) {
        qDebug() << "open out file test.h264 failed!" << endl;
    }
}
