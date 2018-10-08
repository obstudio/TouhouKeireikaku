#include "userinfodialog.h"
#include "ui_userinfodialog.h"
#include "settings.h"
#include "engine.h"
#include "encryptor.h"

#include <QString>
#include <QByteArray>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QNetworkRequest>
#include <QMessageBox>

UserInfoDialog::UserInfoDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::UserInfoDialog)
{
	ui->setupUi(this);

    ui->nameDisplayLabel->setText(Config.UserName);
}

UserInfoDialog::~UserInfoDialog()
{
    delete ui;
}

QString UserInfoDialog::post(QString url, QString cont) {
    QString replyString;
    QHostInfo host = QHostInfo::fromName("www.baidu.com");
    if (host.error() == QHostInfo::NoError) {
        QNetworkRequest request(url);
        QByteArray contArray = cont.toLatin1();
        char *contChar = contArray.data();
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkReply *reply = manager->post(request, contChar);
        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
        QByteArray replyData = reply->readAll();
        replyString = QString(replyData);
        reply->deleteLater();
        reply = nullptr;
    } else {
        replyString = "Connection timeout.";
    }
    //QMessageBox::warning(this, "TouhouKeireikaku Warning", replyString);
    return replyString;
}

int UserInfoDialog::downloadBP() {
    QString url("https://thkrk.ob-studio.cn/download_bp");
    QString cont = QString("username=%1&password=%2").arg(Config.UserName).arg(Encryptor_PHP_Password);
    QString replyString = post(url, cont);
    bool bp_valid;
    
    int bp = replyString.toInt(&bp_valid);
    return bp_valid ? bp : -1;
}

void UserInfoDialog::execWithBP() {
	int bp = downloadBP();
	ui->bpDisplayLabel->setText(bp >= 0 ? QString::number(bp) : tr("Fetch BP failed!"));
	exec();
}