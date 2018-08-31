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
#include <QNetworkConfigurationManager>
#include <QHostInfo>

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
        QStringList skins;
        skins << "compactSkin.image.json" << "compactSkin.layout.json" << "compactSkinAlt.layout.json"
            << "defaultSkin.animation.json" << "defaultSkin.audio.json" << "defaultSkin.image.json"
            << "defaultSkin.layout.json" << "defaultSkinAlt.layout.json"
            << "skinList.json" << "skinListAlt.json";

        QHostInfo host = QHostInfo::fromName("www.baidu.com");

        if (host.error() != QHostInfo::NoError) {
            QMessageBox::warning(NULL, "TouhouKeireikaku Warning", "Offline: " + host.errorString());
            for (int i = 0; i < skins.length(); i++) {
                if (!QFile::exists(user_dir + "/mascot/skins/" + skins.at(i))) {
                    QMessageBox::warning(NULL, "TouhouKeireikaku Warning", "Lacking skin json files.");
                    return 0;
                }
                QFileInfo fileInfo;
                fileInfo.setFile(user_dir + "/mascot/skins/" + skins.at(i));
                if (fileInfo.size() == 0) {
                    QMessageBox::warning(NULL, "TouhouKeireikaku Warning", "Lacking skin json files.");
                    return 0;
                }
            }
        } else {
            DownloadManager manager;
            manager.append("download.json");
            manager.append("hash.json");
            
            for (int i = 0; i < skins.length(); i++) {
                manager.append("skins/" + skins.at(i));
            }

            QString download_dir = user_dir + "/mascot/download.json";
            QFile file;
            file.setFileName(download_dir);
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
            if (QFile::exists(pro_dir + "/mascot/download.json")) {
                QFile pro_dl(pro_dir + "/mascot/download.json");
                pro_dl.open(QIODevice::ReadOnly | QIODevice::Text);
                QString val2 = pro_dl.readAll();
                pro_dl.close();

                QJsonDocument doc2 = QJsonDocument::fromJson(val2.toUtf8());
                QJsonObject dl_obj2 = doc2.object();
                QJsonObject images2 = dl_obj2.value(QString("images")).toObject();

                QJsonArray generals = images["generals"].toArray();
                QJsonArray generals2 = images2["generals"].toArray();
                for (int i = 0; i < generals.size(); i++) {
                    QJsonValue item = generals.at(i);
                    //QString version = item.section(":", -1, -1);
                    //QString version2 = generals2.at(i).toString().section(":", -1, -1);
                    if (!generals2.contains(item)) {
                        QString name = item.toString().section(":", 0, -2);
                        general_card_path = QString("diorite/begonia/salmon/") + name;
                        general_avatar_path = QString("diorite/begonia/bream/") + name;
                        general_fullskin_path = QString("diorite/dandelion/bonito/urotopine/") + name;
                        manager.append(general_card_path);
                        manager.append(general_avatar_path);
                        manager.append(general_fullskin_path);
                    }
                }

                QJsonArray cards = images["cards"].toArray();
                QJsonArray cards2 = images2["cards"].toArray();
                for (int i = 0; i < cards.size(); i++) {
                    QJsonValue item = cards.at(i);
                    //QString version = cards.at(i).toString().section(":", -1, -1);
                    //QString version2 = cards2.at(i).toString().section(":", -1, -1);
                    if (!cards2.contains(item)) {
                        QString name = item.toString().section(":", 0, -2);
                        big_card_path = QString("diorite/sage/") + name;
                        card_path = QString("diorite/papyrus/") + name;
                        manager.append(big_card_path);
                        manager.append(card_path);
                    }
                }

                QJsonArray equips = images["equips"].toArray();
                QJsonArray equips2 = images2["equips"].toArray();
                for (int i = 0; i < equips.size(); i++) {
                    QJsonValue trio_val = equips.at(i);
                    //QJsonObject trio2 = equips2.at(i).toObject();
                    //QString version = trio["version"].toString();
                    //QString version2 = trio2["version"].toString();
                    if (!equips2.contains(trio_val)) {
                        QJsonObject trio = trio_val.toObject();
                        big_card_path = QString("diorite/sage/") + trio["big"].toString();
                        card_path = QString("diorite/papyrus/") + trio["other"].toString();
                        equip_path = QString("diorite/babysbreath/") + trio["other"].toString();
                        small_equip_path = QString("diorite/dandelion/gourami/") + trio["other"].toString();
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
                    general_card_path = QString("diorite/begonia/salmon/") + name;
                    general_avatar_path = QString("diorite/begonia/bream/") + name;
                    general_fullskin_path = QString("diorite/dandelion/bonito/urotopine/") + name;
                    manager.append(general_card_path);
                    manager.append(general_avatar_path);
                    manager.append(general_fullskin_path);
                }

                QJsonArray cards = images["cards"].toArray();
                for (int i = 0; i < cards.size(); i++) {
                    QString name = cards.at(i).toString().section(":", 0, -2);
                    big_card_path = QString("diorite/sage/") + name;
                    card_path = QString("diorite/papyrus/") + name;
                    manager.append(big_card_path);
                    manager.append(card_path);
                }

                QJsonArray equips = images["equips"].toArray();
                for (int i = 0; i < equips.size(); i++) {
                    QJsonObject trio = equips.at(i).toObject();
                    big_card_path = QString("diorite/sage/") + trio["big"].toString();
                    card_path = QString("diorite/papyrus/") + trio["other"].toString();
                    equip_path = QString("diorite/babysbreath/") + trio["other"].toString();
                    small_equip_path = QString("diorite/dandelion/gourami/") + trio["other"].toString();
                    manager.append(big_card_path);
                    manager.append(card_path);
                    manager.append(equip_path);
                    manager.append(small_equip_path);
                }
            }

            manager.append("download.json", pro_dir);
            //QMessageBox::warning(NULL, "TouhouKeireikaku Warning", "Pro download failed!");

            QObject::connect(&manager, SIGNAL(finished()), qApp, SLOT(quit()));
            qApp->exec();

            if (!QFile::exists(user_dir + "/mascot/download.json") || !QFile::exists(user_dir + "/mascot/hash.json")) {
                QMessageBox::warning(NULL, "TouhouKeireikaku Warning", "Files download failed!");
                return 0;
            } else {
                for (int i = 0; i < skins.length(); i++) {
                    if (!QFile::exists(user_dir + "/mascot/skins/" + skins.at(i))) {
                        QMessageBox::warning(NULL, "TouhouKeireikaku Warning", "Files download failed!");
                        return 0;
                    }
                }
            }
        }
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

