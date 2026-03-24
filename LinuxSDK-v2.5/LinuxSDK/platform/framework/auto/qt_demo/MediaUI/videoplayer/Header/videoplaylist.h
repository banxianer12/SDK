/*!
 * @file videoplaylist.h
 * Copyright (c) 2018
 * @brief des
 * detailed des
 *
 * @date 2018
 * @author lee
 */
#ifndef VIDEOPLAYLIST_H
#define VIDEOPLAYLIST_H

#include <QWidget>
#include <QScrollBar>
#include <QDateTime>
#include <QTimer>
#include <QFileInfo>
#include <QThread>
namespace Ui {
class VideoPlayList;
}

class ScanVideoFileThread : public QThread
{
    Q_OBJECT
public:
    explicit ScanVideoFileThread(QObject *parent = 0, QString curPath = NULL);
    ~ScanVideoFileThread();
protected:
    virtual void run();
signals:
    void scanComplete(QList<QFileInfo>*);
private:
    QList<QFileInfo>* fileInfoList;
	
	QString m_curFilePath;
};


class VideoPlayList : public QWidget
{
    Q_OBJECT

public:
    static VideoPlayList *Instance(QWidget *parent = 0);//create window by this interface

    ~VideoPlayList();

signals:
    void stopMovie();

    void notifyVideoPlayerToPlay(QList<QFileInfo> playlists, int cnt);

    void notifyVideoPlayerListChanged(QList<QFileInfo> playlists);

private slots:
    void onScanComplete(QList<QFileInfo>* fileInfoList);

    void on_UsbListButton_clicked();

    void on_favoriteListButton_clicked();

    void on_refreshButton_clicked();

    void onBtnClicked();

    void on_clearButton_clicked();

public slots:

    void hidePlayList();

    void showPlayList(QString str);

    void updateStatusLabel();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::VideoPlayList *ui;
    explicit VideoPlayList(QWidget *parent = 0);
    static VideoPlayList* self;

    QList<QFileInfo> m_PlayList;

    QScrollBar *m_scrollBarV;
	
	QString m_curFilePath;

    void scanVideoFileFromUdisk();
};

#endif // VIDEOPLAYLIST_H
