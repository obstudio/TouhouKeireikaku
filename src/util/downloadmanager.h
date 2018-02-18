#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QtNetwork>
#include <QtCore>
#include <QStandardPaths>

#include "textprogressbar.h"

class DownloadManager: public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);

    void append(const QUrl &url, QString loc = QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    void append(const QStringList &urls, QString loc = QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    void append(const QString &url_str, QString loc = QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    static QString saveFileName(const QUrl &url, QString loc = QStandardPaths::writableLocation(QStandardPaths::DataLocation));

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
    QQueue<QString> locationQueue;
    QNetworkReply *currentDownload = nullptr;
    QFile output;
    QTime downloadTime;
    TextProgressBar progressBar;
    QString location;

    int downloadedCount = 0;
    int totalCount = 0;
};

#endif