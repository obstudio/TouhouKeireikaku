#ifndef _USER_INFO_DIALOG_H
#define _USER_INFO_DIALOG_H

#include <QDialog>
#include <QString>

namespace Ui
{
class UserInfoDialog;
}

class UserInfoDialog : public QDialog
{
    Q_OBJECT
public:
    UserInfoDialog(QWidget *parent = 0);
    ~UserInfoDialog();

private:
    Ui::UserInfoDialog *ui;
    QString post(QString url, QString cont);
    int downloadBP();

private slots:
	void execWithBP();
};

#endif