#include "logindialog.h"
#include "ui_logindialog.h"
#include "settings.h"
#include "engine.h"
#include "detector.h"
#include "SkinBank.h"

#include <QMessageBox>
#include <QTimer>
#include <QRadioButton>
#include <QBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

static const int ShrinkWidth = 285;
static const int ExpandWidth = 826;

void LoginDialog::hideAvatarList()
{
    if (!ui->avatarList->isVisible()) return;
    ui->avatarList->hide();
    ui->avatarList->clear();
}

void LoginDialog::showAvatarList()
{
    if (ui->avatarList->isVisible()) return;
    ui->avatarList->clear();
    QList<const General *> generals = Sanguosha->findChildren<const General *>();
    foreach (const General *general, generals) {
        if (general->objectName() == "anjiang")
            continue;
        QIcon icon(G_ROOM_SKIN.getGeneralPixmap(general->objectName(), QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE, false));
        QString text = Sanguosha->translate(general->objectName());
        QListWidgetItem *item = new QListWidgetItem(icon, text, ui->avatarList);
        item->setData(Qt::UserRole, general->objectName());
    }
    ui->avatarList->show();
}

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    ui->nameLineEdit->setText(Config.UserName);
    ui->nameLineEdit->setMaxLength(64);

    ui->passwordLineEdit->setText("");
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    ui->connectButton->setFocus();

    ui->avatarPixmap->setPixmap(G_ROOM_SKIN.getGeneralPixmap(Config.UserAvatar,
                                                             QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE, false));

    hideAvatarList();

    setFixedHeight(height());
    setFixedWidth(ShrinkWidth);

    manager = new QNetworkAccessManager(this);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)),
        this, SLOT(finishedSlot(QNetworkReply *)));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_connectButton_clicked()
{
    QString username = ui->nameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray pwarray = password.toLatin1();
    char *pwchar = pwarray.data();
    hash.addData(pwchar);
    QString password_hash = hash.result().toHex();

    if (username.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("The user name can not be empty!"));
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Password cannot be empty!"));
        return;
    }

    QStringList hex_digits = {
        "0", "1", "2", "3",
        "4", "5", "6", "7",
        "8", "9", "a", "b",
        "c", "d", "e", "f"
    };
    qsrand(QTime(0, 0, 0).msecsTo(QTime::currentTime()));
    QString client_token = "";
    for (int i = 0; i < 32; i++) {
        client_token += hex_digits.at(qrand() % 16);
    }

    // verify username, password and client token here
    QNetworkRequest request;

    QString url = QString("https://id.ob-studio.cn/api/auth");
    request.setUrl(url);
    QString cont;
    cont = QString("username=%1&password_hash=%2&client_token=%3").arg(username).arg(password_hash).arg(client_token);
    QByteArray contArray = cont.toLatin1();
    char *contChar = contArray.data();
    m_Reply = manager->post(request, contChar);

    Config.UserName = username;
}

void LoginDialog::on_changeAvatarButton_clicked()
{
    if (ui->avatarList->isVisible()) {
        QListWidgetItem *selected = ui->avatarList->currentItem();
        if (selected)
            on_avatarList_itemDoubleClicked(selected);
        else {
            hideAvatarList();
            setFixedWidth(ShrinkWidth);
        }
    } else {
        showAvatarList();
        setFixedWidth(ExpandWidth);
    }
}

void LoginDialog::on_avatarList_itemDoubleClicked(QListWidgetItem *item)
{
    QString general_name = item->data(Qt::UserRole).toString();
    QPixmap avatar(G_ROOM_SKIN.getGeneralPixmap(general_name, QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE, false));
    ui->avatarPixmap->setPixmap(avatar);
    Config.UserAvatar = general_name;
    Config.setValue("UserAvatar", general_name);
    hideAvatarList();

    setFixedWidth(ShrinkWidth);
}

void LoginDialog::finishedSlot(QNetworkReply *)
{
    if (m_Reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, tr("Warning"), QString(tr("Fatal error occurred during request!\n%1")).arg(m_Reply->errorString()));
        return;
    }

    QVariant statusCode = m_Reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!statusCode.isValid()) {
        QMessageBox::warning(this, tr("Warning"), QString(tr("Invalid status code!")));
        return;
    }
    int status = statusCode.toInt();
    if (status != 200) {
        QString reason = m_Reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::warning(this, tr("Warning"), QString(tr("Authentication failed!\nError code: %1")).arg(status));
        return;
    }

    m_Reply->deleteLater();

    Config.setValue("UserName", Config.UserName);

    accept();
}
