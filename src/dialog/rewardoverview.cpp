#include "rewardoverview.h"
#include "ui_rewardoverview.h"
#include "engine.h"
#include "settings.h"
#include "clientstruct.h"
#include "client.h"
#include "encryptor.h"

#include <QMessageBox>
#include <QFile>
#include <QStandardPaths>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>

static RewardOverview *Overview;

QString post(QString url, QString cont)
{
    QString replyString;
    QHostInfo host = QHostInfo::fromName("www.baidu.com");
    if (host.error() == QHostInfo::NoError) {
        QNetworkRequest request(url);
        QByteArray contArray = cont.toLatin1();
        char *contChar = contArray.data();
        QNetworkAccessManager *manager = new QNetworkAccessManager(nullptr);
        QNetworkReply *reply = manager->post(request, contChar);
        QEventLoop eventLoop;
        QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
        QByteArray replyData = reply->readAll();
        replyString = QString(replyData);
        reply->deleteLater();
        reply = nullptr;
    } else {
        replyString = "Connection timeout.";
    }
    qDebug() << replyString << endl;
    return replyString;
}

QStringList downloadRewards(QString username)
{
    QString url("https://thkrk.ob-studio.cn/download_rewards");
    QString cont = QString("username=%1&password=%2").arg(username).arg(Encryptor_PHP_Password);
    QString replyString = post(url, cont);
    if (replyString.startsWith(":::")) {
        replyString = replyString.right(replyString.length() - 3);
        if (replyString.isEmpty())
            return QStringList("None");
        return replyString.split("0");
    }
    return QStringList();
}

RewardOverview *RewardOverview::getInstance(QWidget *main_window)
{
    if (Overview == NULL)
        Overview = new RewardOverview(main_window);

    return Overview;
}

RewardOverview::RewardOverview(QWidget *parent)
    : QDialog(parent), ui(new Ui::RewardOverview)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnWidth(0, 200);
    ui->tableWidget->setColumnWidth(1, 80);

    ui->desDescriptionBox->setProperty("description", true);
}

void RewardOverview::loadFromAll()
{
    QStringList player_des = downloadRewards(Config.UserName);
    QStringList des = Sanguosha->getDesignations();
    int n = des.length();
    ui->tableWidget->setRowCount(n);
    for (int i = 0; i < n; i++) {
        QTableWidgetItem *des_item = new QTableWidgetItem(Sanguosha->translate(des.at(i)));
        ui->tableWidget->setItem(i, 0, des_item);

        QTableWidgetItem *have_item = new QTableWidgetItem(Sanguosha->translate(player_des.contains(des.at(i)) ? "Yes" : "No"));
        ui->tableWidget->setItem(i, 1, have_item);
    }

    if (n > 0) {
        ui->tableWidget->setCurrentItem(ui->tableWidget->item(0, 0));
    }
}

RewardOverview::~RewardOverview()
{
    delete ui;
}

void RewardOverview::on_tableWidget_itemSelectionChanged()
{
    int row = ui->tableWidget->currentRow();
    QString reward = Sanguosha->getDesignations().at(row);

    ui->desLabel->setText(Sanguosha->translate(reward));

    QString description = "";
    description += Sanguosha->translate(":" + reward) + "\n\n";
    description += tr("Reward Point: ") + Sanguosha->translate("~" + reward) + "\n";
    description += tr("Have got: ") + ui->tableWidget->item(row, 1)->text();
    ui->desDescriptionBox->setText(description);
}
