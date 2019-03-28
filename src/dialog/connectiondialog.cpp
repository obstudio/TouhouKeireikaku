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

ConnectionDialog::ConnectionDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

    ui->nameLineEdit->setText(Config.UserName);
    ui->nameLineEdit->setMaxLength(64);

    ui->hostComboBox->addItems(Config.HistoryIPs);
    ui->hostComboBox->lineEdit()->setText(Config.HostAddress);

    ui->connectButton->setFocus();

    ui->reconnectionCheckBox->setChecked(Config.value("EnableReconnection", false).toBool());

    setFixedHeight(height());
    setFixedWidth(ShrinkWidth);
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

void ConnectionDialog::on_connectButton_clicked()
{
    Config.HostAddress = ui->hostComboBox->lineEdit()->text();
    Config.setValue("HostAddress", Config.HostAddress);
    Config.setValue("EnableReconnection", ui->reconnectionCheckBox->isChecked());

    accept();
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

