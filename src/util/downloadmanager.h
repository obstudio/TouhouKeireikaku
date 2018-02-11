#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QtNetwork>
#include <QtCore>

#include "textprogressbar.h"

class DownloadManager: public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);

    void append(const QUrl &url);
    void append(const QStringList &urls);
    void append(const QString &url_str);
    static QString saveFileName(const QUrl &url);

signals:
    void finished();

private slots:
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();

private:
    bool isHttpRedirect() const;
    void reportRedirect();

    QNetworkAccessManager manager;
    QQueue<QUrl> downloadQueue;
    QNetworkReply *currentDownload = nullptr;
    QFile output;
    QTime downloadTime;
    TextProgressBar progressBar;

    int downloadedCount = 0;
    int totalCount = 0;
};

#endif