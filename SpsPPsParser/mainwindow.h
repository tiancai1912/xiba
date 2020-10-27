#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QMediaPlayer>
#include <QMediaPlaylist>

#include "videoparser.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void initTable();
    void initWidget();
    void setTableItem(int index, char *type, QColor color);

    void setTreeItem();

    void setSpsTreeItem(sps_t *sps);
    QTreeWidgetItem * addItem(QTreeWidgetItem *src, QString dst);

    void updateTreeView(int index);

private slots:
    void on_actionOpen_triggered();

    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dropEvent(QDropEvent *event);

    void on_mTable_itemSelectionChanged();

    void on_mBtnPlayFile_clicked();

    static void run(void *arg);
    void runHandle();

private:
    Ui::MainWindow *ui;

    VideoParser *mParser;

    QMediaPlayer *mPlayer;
    QMediaPlaylist *mPlayList;
    QVideoWidget* mVideoWidget;
};

#endif // MAINWINDOW_H
