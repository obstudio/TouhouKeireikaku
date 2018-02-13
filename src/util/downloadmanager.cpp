#include "downloadmanager.h"

#include <QTextStream>
#include <QStandardPaths>
#include <QMessageBox>

#include <cstdio>

using namespace std;

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
{
}

void DownloadManager::append(const QStringList &urls, QString loc)
{
    for (const QString &urlAsString : urls)
        append(QUrl::fromEncoded(urlAsString.toLocal8Bit()), loc);

    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SIGNAL(finished()));
}

void DownloadManager::append(const QUrl &url, QString loc)
{
    /*QString filename = saveFileName(url);
    if (QFile::exists(filename)) {
        if (!filename.endsWith(".json")) {
            QString hash_dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
            hash_dir += QString("/mascot/hash.json");
            QFile file(hash_dir);
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            QString val = file.readAll();
            QJsonObject hash_obj = QJsonDocument::fromJson(val.toUtf8()).object();
            file.close();
            QStringList path_tree = filename.split('/');
            QString key("");
            for (int i = path_tree.lastIndexOf("mascot") + 1; i < path_tree.size() - 1; i++) {
                key += path_tree.at(i) + "/";
            }
            QJsonObject dir_obj = hash_obj.value(key).toObject();
            QString hash = dir_obj.value(path_tree.at(path_tree.size() - 1)).toString();
            QFile local_file(filename);
            QByteArray bArray = QCryptographicHash::hash(local_file.readAll(), QCryptographicHash::Md5);
            QString local_hash(bArray.toHex());
            local_file.close();
            if (hash == local_hash) {
                // fprintf(stderr, "File already exists.");
                // startNextDownload();
                return;
            }
        }
    }*/

    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SLOT(startNextDownload()));

    downloadQueue.enqueue(url);
    locationQueue.enqueue(loc);
    ++totalCount;
}

void DownloadManager::append(const QString &url_str, QString loc)
{
    QString url_full_str("http://thkrk.ob-studio.cn/mascot/");
    url_full_str += url_str;
    QUrl url = QUrl::fromEncoded(url_full_str.toLocal8Bit());
    QString filename = saveFileName(url);
    if (QFile::exists(filename)) {
        if (!filename.endsWith(".json")) {
            QString hash_dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
            hash_dir += QString("/mascot/hash.json");
            QFile file(hash_dir);
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            QString val = file.readAll();
            QJsonObject hash_obj = QJsonDocument::fromJson(val.toUtf8()).object();
            file.close();
            QStringList path_tree = filename.split('/');
            QString key("");
            for (int i = path_tree.lastIndexOf("mascot") + 1; i < path_tree.size() - 1; i++) {
                key += path_tree.at(i) + "/";
            }
            QJsonObject dir_obj = hash_obj.value(key).toObject();
            QString hash = dir_obj.value(path_tree.at(path_tree.size() - 1)).toString();

            QFile local_file(filename);
            if (!local_file.open(QFile::ReadOnly)) {
                QMessageBox::about(NULL, "About", "Can't open file " + filename.section("/", -1, -1) + " !!");
                return;
            }
            QCryptographicHash hash_method(QCryptographicHash::Md5);
            quint64 totalBytes = local_file.size();
            quint64 bytesWritten = 0;
            quint64 bytesToWrite = totalBytes;
            quint64 loadSize = 2097152;
            QByteArray buf;

            while (1) {
                if (bytesToWrite > 0) {
                    quint64 minReadSize = qMin(bytesToWrite, loadSize);
                    //QMessageBox::about(NULL, "About", filename.section("/", -1, -1) + ":\n" + QString::number(minReadSize));
                    buf = local_file.read(minReadSize);
                    hash_method.addData(buf);
                    bytesWritten += buf.length();
                    bytesToWrite -= buf.length();
                    //QMessageBox::about(NULL, "About", filename.section("/", -1, -1) + ":\n" + QString::number(bytesWritten) + "\n" + QString::number(bytesToWrite));
                    buf.resize(0);
                } else {
                    break;
                }

                if (bytesWritten >= totalBytes) {
                    break;
                }
            }
            local_file.close();

            //hash_method.addData(&local_file);
            QString local_hash = hash_method.result().toHex();
            //QByteArray bArray = QCryptographicHash::hash(local_file.readAll(), QCryptographicHash::Md5);
            //QString local_hash(bArray.toHex());

            //QMessageBox::about(NULL, "About", filename.section("/", -1, -1) + ":\n" + hash + "\n" + local_hash);
            
            if (hash == local_hash) {
                // fprintf(stderr, "File already exists.");
                // startNextDownload();
                return;
            }
        }
    }
    append(url, loc);
}

QString DownloadManager::saveFileName(const QUrl &url, QString loc)
{
    QString path = url.path();

    //QString user_dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

    /* if (path.startsWith("/mascot")) {
        path = path.right(path.length() - 7);
    }
    QString file_name = QFileInfo(path).fileName();
    if (file_name.isEmpty())
        file_name = "download";

    QString basename = user_dir + "/" + file_name; */

    QString basename = loc + path;
    QString real_dir = basename.section("/", 0, -2);
    QDir dir(real_dir);
    if (!dir.exists()) {
        dir.mkpath(real_dir);
    }

    /* if (QFile::exists(basename)) {
        // already exists, don't overwrite
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    } */

    return basename;
}

void DownloadManager::startNextDownload()
{
    if (downloadQueue.isEmpty()) {
        printf("%d/%d files downloaded successfully\n", downloadedCount, totalCount);
        emit finished();
        return;
    }

    QUrl url = downloadQueue.dequeue();
    QString path = url.path();

    QString loc = locationQueue.dequeue();
    QString filename = saveFileName(url, loc);
    /*if (QFile::exists(filename)) {
        if (!filename.endsWith(".json")) {
            QString hash_dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
            hash_dir += QString("/mascot/hash.json");
            QFile file(hash_dir);
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            QString val = file.readAll();
            QJsonObject hash_obj = QJsonDocument::fromJson(val.toUtf8()).object();
            file.close();
            QStringList path_tree = filename.split('/');
            QString key("");
            for (int i = path_tree.lastIndexOf("mascot") + 1; i < path_tree.size() - 1; i++) {
                key += path_tree.at(i) + "/";
            }
            QJsonObject dir_obj = hash_obj.value(key).toObject();
            QString hash = dir_obj.value(path_tree.at(path_tree.size() - 1)).toString();
            QFile local_file(filename);
            QByteArray bArray = QCryptographicHash::hash(local_file.readAll(), QCryptographicHash::Md5);
            QString local_hash(bArray.toHex());
            local_file.close();
            if (hash == local_hash) {
                // fprintf(stderr, "File already exists.");
                startNextDownload();
                return;
            }
        }
    } */
    output.setFileName(filename);
    if (!output.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Problem opening save file '%s' for download '%s': %s\n",
                qPrintable(filename), url.toEncoded().constData(),
                qPrintable(output.errorString()));

        startNextDownload();
        return;                 // skip this download
    }

    QNetworkRequest request(url);
    currentDownload = manager.get(request);
    connect(currentDownload, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(downloadProgress(qint64,qint64)));
    connect(currentDownload, SIGNAL(finished()),
            SLOT(downloadFinished()));
    connect(currentDownload, SIGNAL(readyRead()),
            SLOT(downloadReadyRead()));

    // prepare the output
    printf("Downloading %s...\n", url.toEncoded().constData());
    downloadTime.start();
}

void DownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    progressBar.setStatus(bytesReceived, bytesTotal);

    // calculate the download speed
    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    progressBar.setMessage(QString::fromLatin1("%1 %2")
                           .arg(speed, 3, 'f', 1).arg(unit));
    progressBar.update();
}

void DownloadManager::downloadFinished()
{
    progressBar.clear();
    output.close();

    if (currentDownload->error()) {
        // download failed
        fprintf(stderr, "Failed: %s\n", qPrintable(currentDownload->errorString()));
        output.remove();
    } else {
        // let's check if it was actually a redirect
        if (isHttpRedirect()) {
            reportRedirect();
            output.remove();
        } else {
            printf("Succeeded.\n");
            ++downloadedCount;
        }
    }

    currentDownload->deleteLater();
    startNextDownload();
}

void DownloadManager::downloadReadyRead()
{
    output.write(currentDownload->readAll());
}

bool DownloadManager::isHttpRedirect() const
{
    int statusCode = currentDownload->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void DownloadManager::reportRedirect()
{
    int statusCode = currentDownload->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QUrl requestUrl = currentDownload->request().url();
    QTextStream(stderr) << "Request: " << requestUrl.toDisplayString()
                        << " was redirected with code: " << statusCode
                        << '\n';

    QVariant target = currentDownload->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!target.isValid())
        return;
    QUrl redirectUrl = target.toUrl();
    if (redirectUrl.isRelative())
        redirectUrl = requestUrl.resolved(redirectUrl);
    QTextStream(stderr) << "Redirected to: " << redirectUrl.toDisplayString()
                        << '\n';
}