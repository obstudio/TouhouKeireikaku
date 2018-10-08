#ifndef _LOGIN_DIALOG_H
#define _LOGIN_DIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QComboBox>
#include <QButtonGroup>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class UdpDetector;

namespace Ui
{
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    LoginDialog(QWidget *parent);
    ~LoginDialog();
    void hideAvatarList();
    void showAvatarList();

private:
    Ui::LoginDialog *ui;
    QNetworkAccessManager *manager;
    QNetworkReply *m_Reply;

private slots:
    void on_avatarList_itemDoubleClicked(QListWidgetItem *item);
    void on_changeAvatarButton_clicked();
    void on_connectButton_clicked();
    void finishedSlot(QNetworkReply *);
};

#endif

