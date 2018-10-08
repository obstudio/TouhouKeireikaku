#ifndef _REWARD_OVERVIEW_H
#define _REWARD_OVERVIEW_H

#include <QDialog>
#include <QTableWidgetItem>

class MainWindow;
namespace Ui
{
class RewardOverview;
}

class RewardOverview : public QDialog
{
    Q_OBJECT

public:
    static RewardOverview *getInstance(QWidget *main_window);

    RewardOverview(QWidget *parent = 0);
    void loadFromAll();

    ~RewardOverview();

private:
    Ui::RewardOverview *ui;

private slots:
    void on_tableWidget_itemSelectionChanged();
};

#endif

