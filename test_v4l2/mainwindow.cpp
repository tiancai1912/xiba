#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <fcntl.h>
#include <QDebug>

#define CAPTURE_WIDTH 160
#define CAPTURE_HEIGHT 120

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mTexEdit->setText("Hello, Welcome to TextEdit!\n");
    initWidgets();
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::saveFrameAsBmpFile(unsigned char *pData, int width, int height, std::string strPath)
{
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
//    targetinfoheader.biSizeImage = CAPTURE_WIDTH*CAPTURE_HEIGHT*3;
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

void MainWindow::initWidgets()
{
    ui->mSliderBrightness->setRange(0, 100);
    ui->mSpinBoxBrightness->setRange(0, 100);

    ui->mSliderSaturation->setRange(0, 100);
    ui->mSpinBoxSaturation->setRange(0, 100);

    ui->mSliderContrast->setRange(0, 100);
    ui->mSpinBoxContrast->setRange(0, 100);

    ui->mSliderHue->setRange(0, 100);
    ui->mSpinBoxHue->setRange(0, 100);


    //slot
    connect(ui->mSpinBoxBrightness, SIGNAL(valueChanged(int)), ui->mSliderBrightness, SLOT(setValue(int)));
    connect(ui->mSliderBrightness, SIGNAL(valueChanged(int)), ui->mSpinBoxBrightness, SLOT(setValue(int)));
    connect(ui->mSpinBoxBrightness, SIGNAL(valueChanged(int)), this, SLOT(setBrightness(int)));
    connect(ui->mSliderBrightness, SIGNAL(valueChanged(int)), this, SLOT(setBrightness(int)));

    connect(ui->mSliderSaturation, SIGNAL(valueChanged(int)), ui->mSpinBoxSaturation, SLOT(setValue(int)));
    connect(ui->mSpinBoxSaturation, SIGNAL(valueChanged(int)), ui->mSliderSaturation, SLOT(setValue(int)));
    connect(ui->mSliderSaturation, SIGNAL(valueChanged(int)), this, SLOT(setSaturation(int)));
    connect(ui->mSpinBoxSaturation, SIGNAL(valueChanged(int)), this, SLOT(setSaturation(int)));

    connect(ui->mSliderContrast, SIGNAL(valueChanged(int)), ui->mSpinBoxContrast, SLOT(setValue(int)));
    connect(ui->mSpinBoxContrast, SIGNAL(valueChanged(int)), ui->mSliderContrast, SLOT(setValue(int)));
    connect(ui->mSliderContrast, SIGNAL(valueChanged(int)), this, SLOT(setContrast(int)));
    connect(ui->mSpinBoxContrast, SIGNAL(valueChanged(int)), this, SLOT(setContrast(int)));

    connect(ui->mSliderHue, SIGNAL(valueChanged(int)), ui->mSpinBoxHue, SLOT(setValue(int)));
    connect(ui->mSpinBoxHue, SIGNAL(valueChanged(int)), ui->mSliderHue, SLOT(setValue(int)));
    connect(ui->mSliderHue, SIGNAL(valueChanged(int)), this, SLOT(setHue(int)));
    connect(ui->mSpinBoxHue, SIGNAL(valueChanged(int)), this, SLOT(setHue(int)));
}

int MainWindow::setBrightness(int value)
{
    return m_v4l2.setBrightness(value);
}

int MainWindow::setSaturation(int value)
{
    return m_v4l2.setSaturation(value);
}

int MainWindow::setContrast(int value)
{
    return m_v4l2.setContrast(value);
}

int MainWindow::setHue(int value)
{
    return m_v4l2.setHue(value);
}

void MainWindow::on_mBtnSetText_clicked()
{
    ui->mTexEdit->setText("Hello, I Can Insert Plain Text!\n");
}

void MainWindow::on_mBtnInsertText_clicked()
{
    ui->mTexEdit->insertPlainText("Hello, I Can Set Plain Text!\n");
}

void MainWindow::on_mBtnOpenDevice_clicked()
{
    m_v4l2.openDevice("/dev/video0");
    m_v4l2.setCaptureCallback(this);
//    m_fd = m_v4l2.v4l2_open("/dev/video0", O_RDWR | O_NONBLOCK);
//    if (m_fd < 0) {
//        qDebug() << "open device failed!\n";
//    }

//    struct v4l2_capability cap;
//    int ret = m_v4l2.v4l2_querycap(m_fd, &cap);
//    if (ret < 0) {
//        qDebug() << "query cap failed!\n";
//    }

//    qDebug() << "card: " << cap.card <<endl;
//    qDebug() << "driver: " << cap.driver <<endl;
//    qDebug() << "version: " << cap.version <<endl;
//    qDebug() << "device caps: " << cap.device_caps <<endl;
//    qDebug() << "support video capture: " << (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) << endl;
//    qDebug() << "support video mmap: " << (cap.capabilities & V4L2_CAP_STREAMING) << endl;
}

void MainWindow::on_mBtnCloseDevice_clicked()
{
    m_v4l2.closeDevice();
//    int ret = m_v4l2.v4l2_close(m_fd);
//    if (ret < 0) {
//        qDebug() << "close device failed!\n";
//    }
}

int MainWindow::yuvtorgb0(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
    unsigned int in, out;
    int y0, u, y1, v;
    unsigned int pixel24;
    unsigned char *pixel = (unsigned char *)&pixel24;
    unsigned int size = width*height*2;

    for (in = 0, out = 0; in < size; in += 4, out += 6)
    {
        y0 = yuv[in+0];
        u  = yuv[in+1];
        y1 = yuv[in+2];
        v  = yuv[in+3];

        m_sign3 = true;
        pixel24 = yuvtorgb(y0, u, v);
        rgb[out+0] = pixel[0];    //for QT
        rgb[out+1] = pixel[1];
        rgb[out+2] = pixel[2];
        //rgb[out+0] = pixel[2];  //for iplimage
        //rgb[out+1] = pixel[1];
        //rgb[out+2] = pixel[0];

        //sign3 = true;
        pixel24 = yuvtorgb(y1, u, v);
        rgb[out+3] = pixel[0];
        rgb[out+4] = pixel[1];
        rgb[out+5] = pixel[2];
        //rgb[out+3] = pixel[2];
        //rgb[out+4] = pixel[1];
        //rgb[out+5] = pixel[0];
    }
    return 0;
}

int MainWindow::yuvtorgb(int y, int u, int v)
{
   unsigned int pixel24 = 0;
   unsigned char *pixel = (unsigned char *)&pixel24;
   int r, g, b;
   static long int ruv, guv, buv;

   if (m_sign3)
   {
       m_sign3 = false;
       ruv = 1159*(v-128);
       guv = 380*(u-128) + 813*(v-128);
       buv = 2018*(u-128);
   }

   r = (1164*(y-16) + ruv) / 1000;
   g = (1164*(y-16) - guv) / 1000;
   b = (1164*(y-16) + buv) / 1000;

   if (r > 255) r = 255;
   if (g > 255) g = 255;
   if (b > 255) b = 255;
   if (r < 0) r = 0;
   if (g < 0) g = 0;
   if (b < 0) b = 0;

   pixel[0] = r;
   pixel[1] = g;
   pixel[2] = b;

   return pixel24;
}

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
        ui->mLabelShowImage->setPixmap(pixmap);

        delete dst_buf[0];
        dst_buf[0] = nullptr;
    }
}

void MainWindow::on_mBtnSaveBmp_clicked()
{
    RawVideoData *data = m_v4l2.getFrame();
    if (data->data != nullptr) {
        unsigned char *src_buf[4];
        int src_linesize[4] = {240, 0, 0, 0};

        unsigned char *dst_buf[4];
        int dst_linesize[4] = {480, 0, 0, 0};

        src_buf[0] = (unsigned char *)(data->data);
        dst_buf[0] = (unsigned char *)malloc(CAPTURE_WIDTH * CAPTURE_HEIGHT * 3);
        m_convert.convert(src_buf, src_linesize, dst_buf, dst_linesize);
        saveFrameAsBmpFile((unsigned char *)(dst_buf[0]), CAPTURE_WIDTH, CAPTURE_HEIGHT, "./test.bmp");
    }
}

void MainWindow::on_mBtnStartCapture_clicked()
{
    m_v4l2.startCaptureLoop();

//    int width = CAPTURE_WIDTH;
//    int height = CAPTURE_HEIGHT;

//    int ret = m_v4l2.v4l2_s_fmt(m_fd, &width, &height, V4L2_PIX_FMT_YUYV, V4L2_BUF_TYPE_VIDEO_CAPTURE);
//    if (ret < 0) {
//        qDebug() << "set fmt failed!\n";
//        return;
//    }

//    //init bufs
//    m_bufs = m_v4l2.v4l2_reqbufs(m_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, 3);
//    if (m_bufs == NULL) {
//        qDebug() << "req bufs failed!\n";
//        return;
//    }

//    ret = m_v4l2.v4l2_querybuf(m_fd, m_bufs);
//    if (ret < 0) {
//        qDebug() << "query buf failed!\n";
//        return;
//    }

//    ret = m_v4l2.v4l2_mmap(m_fd, m_bufs);
//    if (ret < 0) {
//        qDebug() << "mmap buf failed!\n";
//        return;
//    }

//    ret = m_v4l2.v4l2_qbuf_all(m_fd, m_bufs);
//    if (ret < 0) {
//        qDebug() << "qbuf failed!\n";
//        return;
//    }

//    ret = m_v4l2.v4l2_streamon(m_fd);
//    if (ret < 0) {
//        qDebug() << "v4l2 stream on failed!\n";
//        return;
//    }

////    int count = 0;
////    do {
////        ret = m_v4l2.v4l2_poll(m_fd);
////        if (ret < 0) {
////            qDebug() << "poll failed!\n";
//////            return;
////        }

////        count++;
////    } while (ret < 0 || count > 100);

////    ret = m_v4l2.v4l2_poll(m_fd);
////    if (ret < 0) {
////        qDebug() << "poll failed!\n";
////        return;
////    }

////    FILE *fp = fopen("test.rgb", "wb");

//    MyV4l2::v4l2_buf_unit *unit = m_v4l2.v4l2_dqbuf(m_fd, m_bufs);
//    if (unit == NULL) {
//        qDebug() << "dqbuf failed!\n";
////        return;
//    } else {
//        m_unit = unit;
////        unsigned char *src_buf[4];
////        int src_linesize[4] = {240, 0, 0, 0};
////        unsigned char *dst_buf[4];
////        int dst_linesize[4] = {480, 0, 0, 0};
////        src_buf[0] = (unsigned char *)(unit->start);
////        printf("the src length: %d\n", strlen((char *)src_buf[0]));
////        m_convert.convert(src_buf, src_linesize, dst_buf, dst_linesize);

////        fwrite(dst_buf[0], CAPTURE_WIDTH*CAPTURE_HEIGHT*3, 1, fp);
//        qDebug() << "the index: " << m_unit->index << endl;
//    }

//    ret = m_v4l2.v4l2_qbuf(m_fd, unit);
//    if (ret != 0) {
//        qDebug() << "v4l2_qbuf failed" << endl;
//    }

////    int num = 10;
////    while (num > 0) {
////        m_v4l2.v4l2_poll(m_fd);
////        num --;
////        qDebug() << "the num: " << num <<endl;

////        v4l2_buf_unit *unit1 = m_v4l2.v4l2_dqbuf(m_fd, m_bufs);
////        if (unit1 == NULL) {
////            qDebug() << "dqbuf failed!\n";
////    //        return;
////        } else {
////            m_unit = unit1;

//////            unsigned char *src_buf[4];
//////            int src_linesize[4] = {240, 0, 0, 0};
//////            unsigned char *dst_buf[4];
//////            int dst_linesize[4] = {480, 0, 0, 0};
//////            src_buf[0] = (unsigned char *)(unit1->start);
//////            printf("the src length: %d\n", strlen((char *)src_buf[0]));
//////            m_convert.convert(src_buf, src_linesize, dst_buf, dst_linesize);

//////            fwrite(dst_buf[0], CAPTURE_WIDTH*CAPTURE_HEIGHT*3, 1, fp);
////            qDebug() << "the index: " << m_unit->index << endl;
////        }

////        ret = m_v4l2.v4l2_qbuf(m_fd, unit1);
////        if (ret != 0) {
////            qDebug() << "v4l2_qbuf failed" << endl;
////        }
////    }

//////    v4l2_buf_unit *unit2 = m_v4l2.v4l2_dqbuf(m_fd, m_bufs);
//////    if (unit2 == NULL) {
//////        qDebug() << "dqbuf failed!\n";
////////        return;
//////    } else {
//////        m_unit = unit2;
//////    }


////    fclose(fp);
//    qDebug() << "capture one frame: index: " << m_unit->index << "unit length: "<< m_unit->length << endl;
}

void MainWindow::on_mBtnStopCapture_clicked()
{
    m_v4l2.stopCaptureLoop();

//    int ret = m_v4l2.v4l2_streamoff(m_fd);
//    if (ret < 0) {
//        qDebug() << "stream off failed!\n";
//        return;
//    }

//    ret = m_v4l2.v4l2_munmap(m_fd, m_bufs);
//    if (ret < 0) {
//        qDebug() << "v4l2 munmap failed!\n";
//        return;
//    }
}

void MainWindow::on_mBtnShowDevices_clicked()
{
//    int ret = m_v4l2.v4l2_enum_fmt(m_fd, V4L2_PIX_FMT_YUYV, V4L2_BUF_TYPE_VIDEO_CAPTURE);
//    if (ret < 0) {
//        qDebug() << "Not Support yuyv format!\n";
//        return;
//    }

//    qDebug() << "Support yuyv format!\n";
}

void MainWindow::on_mSliderBrightness_valueChanged(int value)
{
//    ui->mLabelBrightness->setText(QString::number(value, 10));
    ui->mSpinBoxBrightness->setValue(value);
    qDebug() << value << endl;
}
