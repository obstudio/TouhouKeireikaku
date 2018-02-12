#include <QApplication>

#include <QCoreApplication>
#include <QTranslator>
#include <QDir>
#include <cstring>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QMessageBox>

#include "mainwindow.h"
#include "settings.h"
#include "banpair.h"
#include "server.h"
#include "audio.h"
#include "downloadmanager.h"

#ifdef USE_BREAKPAD
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4091)
#endif
#include "breakpad/client/windows/handler/exception_handler.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif


using namespace google_breakpad;

static bool callback(const wchar_t *dump_path, const wchar_t *id,
                     void *, EXCEPTION_POINTERS *,
                     MDRawAssertionInfo *,
                     bool succeeded)
{
    if (succeeded)
        qWarning("Dump file created in %s, dump guid is %s\n", dump_path, id);
    else
        qWarning("Dump failed\n");
    return succeeded;
}

int main(int argc, char *argv[])
{
    ExceptionHandler eh(L"./dmp", NULL, callback, NULL,
                        ExceptionHandler::HANDLER_ALL);
#else
int main(int argc, char *argv[])
{
#endif
    if (argc > 1 && strcmp(argv[1], "-server") == 0) {
        new QCoreApplication(argc, argv);
    } else {
        new QApplication(argc, argv);
        QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");

        QString user_dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

        DownloadManager manager;
        manager.append("download.json");
        manager.append("hash.json");

        QFile file;
        file.setFileName(user_dir + "/assets/download.json");
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString val = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
        QJsonObject dl_obj = doc.object();
        QJsonObject images = dl_obj.value(QString("images")).toObject();

        QString general_card_path, general_avatar_path, general_fullskin_path;
        QString big_card_path, card_path;
        QString equip_path, small_equip_path;
        
        //Q_ASSERT(QFile::exists(user_dir + "/assets/download.json"));
        //Q_ASSERT(QFile::exists(user_dir + "/assets/hash.json"));

        QString pro_dir("C:/ProgramData/TouhouKeireikaku");
        if (QFile::exists(pro_dir + "/assets/download.json")) {
            QFile pro_dl(pro_dir + "/assets/download.json");
            pro_dl.open(QIODevice::ReadOnly | QIODevice::Text);
            QString val2 = pro_dl.readAll();
            pro_dl.close();

            QJsonDocument doc2 = QJsonDocument::fromJson(val2.toUtf8());
            QJsonObject dl_obj2 = doc2.object();
            QJsonObject images2 = dl_obj2.value(QString("images")).toObject();

            QJsonArray generals = images["generals"].toArray();
            QJsonArray generals2 = images2["generals"].toArray();
            for (int i = 0; i < generals.size(); i++) {
                QString version = generals.at(i).toString().section(":", -1, -1);
                QString version2 = generals2.at(i).toString().section(":", -1, -1);
                if (version != version2) {
                    QString name = generals.at(i).toString().section(":", 0, -2);
                    general_card_path = QString("image/generals/card/") + name + QString(".jpg");
                    general_avatar_path = QString("image/generals/avatar/") + name + QString(".png");
                    general_fullskin_path = QString("image/fullskin/generals/full/") + name + QString(".png");
                    manager.append(general_card_path);
                    manager.append(general_avatar_path);
                    manager.append(general_fullskin_path);
                }
            }

            QJsonArray cards = images["cards"].toArray();
            QJsonArray cards2 = images2["cards"].toArray();
            for (int i = 0; i < cards.size(); i++) {
                QString version = cards.at(i).toString().section(":", -1, -1);
                QString version2 = cards2.at(i).toString().section(":", -1, -1);
                if (version != version2) {
                    QString name = cards.at(i).toString().section(":", 0, -2);
                    big_card_path = QString("image/big-card/") + name + QString(".png");
                    card_path = QString("image/card/") + name + QString(".png");
                    manager.append(big_card_path);
                    manager.append(card_path);
                }
            }

            QJsonArray equips = images["equips"].toArray();
            QJsonArray equips2 = images2["equips"].toArray();
            for (int i = 0; i < equips.size(); i++) {
                QJsonObject trio = equips.at(i).toObject();
                QJsonObject trio2 = equips2.at(i).toObject();
                QString version = trio["version"].toString();
                QString version2 = trio2["version"].toString();
                if (version != version2) {
                    big_card_path = QString("image/big-card/") + trio["big"].toString() + QString(".png");
                    card_path = QString("image/card/") + trio["other"].toString() + QString(".png");
                    equip_path = QString("image/equips/") + trio["other"].toString() + QString(".png");
                    small_equip_path = QString("image/fullskin/small-equips/") + trio["other"].toString() + QString(".png");
                    manager.append(big_card_path);
                    manager.append(card_path);
                    manager.append(equip_path);
                    manager.append(small_equip_path);
                }
            }
        } else {
            QJsonArray generals = images["generals"].toArray();
            for (int i = 0; i < generals.size(); i++) {
                QString name = generals.at(i).toString().section(":", 0, -2);
                general_card_path = QString("image/generals/card/") + name + QString(".jpg");
                general_avatar_path = QString("image/generals/avatar/") + name + QString(".png");
                general_fullskin_path = QString("image/fullskin/generals/full/") + name + QString(".png");
                manager.append(general_card_path);
                manager.append(general_avatar_path);
                manager.append(general_fullskin_path);
            }

            QJsonArray cards = images["cards"].toArray();
            for (int i = 0; i < cards.size(); i++) {
                QString name = cards.at(i).toString().section(":", 0, -2);
                big_card_path = QString("image/big-card/") + name + QString(".png");
                card_path = QString("image/card/") + name + QString(".png");
                manager.append(big_card_path);
                manager.append(card_path);
            }

            QJsonArray equips = images["equips"].toArray();
            for (int i = 0; i < equips.size(); i++) {
                QJsonObject trio = equips.at(i).toObject();
                big_card_path = QString("image/big-card/") + trio["big"].toString() + QString(".png");
                card_path = QString("image/card/") + trio["other"].toString() + QString(".png");
                equip_path = QString("image/equips/") + trio["other"].toString() + QString(".png");
                small_equip_path = QString("image/fullskin/small-equips/") + trio["other"].toString() + QString(".png");
                manager.append(big_card_path);
                manager.append(card_path);
                manager.append(equip_path);
                manager.append(small_equip_path);
            }
        }

        manager.append("download.json", pro_dir);

        QObject::connect(&manager, SIGNAL(finished()), qApp, SLOT(quit()));
        qApp->exec();
    }

#ifdef Q_OS_MAC
#ifdef QT_NO_DEBUG
    QDir::setCurrent(qApp->applicationDirPath());
#endif
#endif

#ifdef Q_OS_LINUX
    QDir dir(QString("lua"));
    if (dir.exists() && (dir.exists(QString("config.lua")))) {
        // things look good and use current dir
    } else {
        QDir::setCurrent(qApp->applicationFilePath().replace("games", "share"));
    }
#endif

    // initialize random seed for later use
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    QTranslator qt_translator, translator;
    qt_translator.load("qt_zh_CN.qm");
    translator.load("sanguosha.qm");

    qApp->installTranslator(&qt_translator);
    qApp->installTranslator(&translator);

    Sanguosha = new Engine;
    Config.init();
    //qApp->setFont(Config.AppFont);
    BanPair::loadBanPairs();

    if (qApp->arguments().contains("-server")) {
		//printf("Server will start in a few seconds...\n");
        Server *server = new Server(qApp);
        printf("Server is starting on port %u\n", Config.ServerPort);

        if (server->listen())
            printf("Starting successfully\n");
        else
            printf("Starting failed!\n");

        return qApp->exec();
    } else {
		qApp->setFont(Config.AppFont);
	}

    QFile file("sanguosha.qss");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
    }

#ifdef AUDIO_SUPPORT
    Audio::init();
#endif

    MainWindow *main_window = new MainWindow;

    Sanguosha->setParent(main_window);
    main_window->show();

    foreach (QString arg, qApp->arguments()) {
        if (arg.startsWith("-connect:")) {
            arg.remove("-connect:");
            Config.HostAddress = arg;
            Config.setValue("HostAddress", arg);

            main_window->startConnection();
            break;
        }
    }

    return qApp->exec();
}

