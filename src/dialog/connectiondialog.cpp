#include "connectiondialog.h"
#include "ui_connectiondialog.h"
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

void ConnectionDialog::hideAvatarList()
{
    if (!ui->avatarList->isVisible()) return;
    ui->avatarList->hide();
    ui->avatarList->clear();
}

void ConnectionDialog::showAvatarList()
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

ConnectionDialog::ConnectionDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

    ui->nameLineEdit->setText(Config.UserName);
    ui->nameLineEdit->setMaxLength(64);

    ui->passwordLineEdit->setText("");
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    ui->hostComboBox->addItems(Config.HistoryIPs);
    ui->hostComboBox->lineEdit()->setText(Config.HostAddress);

    ui->connectButton->setFocus();

    ui->avatarPixmap->setPixmap(G_ROOM_SKIN.getGeneralPixmap(Config.UserAvatar,
                                                             QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE, false));

    hideAvatarList();

    ui->reconnectionCheckBox->setChecked(Config.value("EnableReconnection", false).toBool());

    setFixedHeight(height());
    setFixedWidth(ShrinkWidth);

    manager = new QNetworkAccessManager(this);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)),
        this, SLOT(finishedSlot(QNetworkReply *)));
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

void ConnectionDialog::on_connectButton_clicked()
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

    // verify username and password here
    QNetworkRequest request;

    QString url = QString("https://id.ob-studio.cn/assets/data/user_login.php");
    request.setUrl(url);
    QString cont;
    cont = QString("username=%1&password_hash=%2").arg(username).arg(password_hash);
    QByteArray contArray = cont.toLatin1();
    char *contChar = contArray.data();
    m_Reply = manager->post(request, contChar);

    Config.UserName = username;
    Config.HostAddress = ui->hostComboBox->lineEdit()->text();
}

void ConnectionDialog::on_changeAvatarButton_clicked()
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

void ConnectionDialog::on_avatarList_itemDoubleClicked(QListWidgetItem *item)
{
    QString general_name = item->data(Qt::UserRole).toString();
    QPixmap avatar(G_ROOM_SKIN.getGeneralPixmap(general_name, QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE, false));
    ui->avatarPixmap->setPixmap(avatar);
    Config.UserAvatar = general_name;
    Config.setValue("UserAvatar", general_name);
    hideAvatarList();

    setFixedWidth(ShrinkWidth);
}

void ConnectionDialog::on_clearHistoryButton_clicked()
{
    ui->hostComboBox->clear();
    ui->hostComboBox->lineEdit()->clear();

    Config.HistoryIPs.clear();
    Config.remove("HistoryIPs");
}

void ConnectionDialog::on_detectLANButton_clicked()
{
    UdpDetectorDialog *detector_dialog = new UdpDetectorDialog(this);
    connect(detector_dialog, SIGNAL(address_chosen(QString)),
            ui->hostComboBox->lineEdit(), SLOT(setText(QString)));

    detector_dialog->exec();
}

void ConnectionDialog::finishedSlot(QNetworkReply *)
{
    m_Reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    m_Reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    bool flag = false;

    if (m_Reply->error() == QNetworkReply::NoError) {
        QByteArray contentArray = m_Reply->readAll();
        QString content = QString::fromUtf8(contentArray);
        qDebug() << content;
        if (content != QString("0")) {
            QMessageBox::warning(this, tr("Warning"), QString(tr("Username or password is incorrect!\nValidate code: %1")).arg(content));
            flag = true;
        }
    } else {
        qDebug() << m_Reply->errorString();
        QMessageBox::warning(this, tr("Warning"), tr("Fatal error occurred during request!"));
        flag = true;
    }

    m_Reply->deleteLater();

    if (flag) return;

    Config.setValue("UserName", Config.UserName);
    Config.setValue("HostAddress", Config.HostAddress);
    Config.setValue("EnableReconnection", ui->reconnectionCheckBox->isChecked());

    accept();
}

// -----------------------------------

UdpDetectorDialog::UdpDetectorDialog(QDialog *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Detect available server's addresses at LAN"));
    detect_button = new QPushButton(tr("Refresh"));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(detect_button);

    list = new QListWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(list);
    layout->addLayout(hlayout);

    setLayout(layout);

    detector = NULL;
    connect(detect_button, SIGNAL(clicked()), this, SLOT(startDetection()));
    connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(chooseAddress(QListWidgetItem *)));

    detect_button->click();
}

void UdpDetectorDialog::startDetection()
{
    list->clear();
    detect_button->setEnabled(false);

    detector = new UdpDetector;
    connect(detector, SIGNAL(detected(QString, QString)), this, SLOT(addServerAddress(QString, QString)));
    QTimer::singleShot(2000, this, SLOT(stopDetection()));

    detector->detect();
}

void UdpDetectorDialog::stopDetection()
{
    detect_button->setEnabled(true);
    detector->stop();
    delete detector;
    detector = NULL;
}

void UdpDetectorDialog::addServerAddress(const QString &server_name, const QString &address_)
{
    QString address = address_;
    if (address.startsWith("::ffff:"))
        address.remove(0, 7);
    QString label = QString("%1 [%2]").arg(server_name).arg(address);
    QListWidgetItem *item = new QListWidgetItem(label);
    item->setData(Qt::UserRole, address);

    list->addItem(item);
}

void UdpDetectorDialog::chooseAddress(QListWidgetItem *item)
{
    accept();

    QString address = item->data(Qt::UserRole).toString();
    emit address_chosen(address);
}

