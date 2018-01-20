TouhouKeireikaku Alpha
=====================================

TouhouKeireikaku is an open source game, which derives from Touhoukill by Iwtmusou and is originally
based on QSanguosha and Touhou Project. It is never to be used for any commercial purposes.<br>
The whole project is written in C++, along with QT for GUI framework, and Lua for AI scripts.<br>
by Kouchya

GitHub: https://github.com/obstudio/TouhouKeireikaku

Email: kouchyakun@outlook.com

*************************************

Current version: 0.1.7 alpha

*************************************

For players:

To make the program function properly, the following dll files may be needed (apart from the files
that are already included in the root directory):<br>
　　bin/icudt54.dll<br>
　　bin/icuin54.dll<br>
　　bin/icuuc54.dll<br>
　　bin/Qt5Declarative.dll<br>
　　bin/Qt5Qml.dll<br>
　　bin/Qt5Script.dll<br>
　　bin/Qt5Sql.dll<br>
　　bin/libgcc_s_dw2-1.dl<br>
　　bin/libstdc++-6.dll<br>
　　bin/libwinpthread-1.dll<br>
　　bin/Qt5XmlPatterns.dll<br>
　　plugins/imageformats<br>

*************************************

For developers:

To build this project, a QT Library (5.5.1 and 5.6.1 tested, 4.x does not work) and a QT Creator (3.5.1 tested) are both needed.
Swig may be necessary as well in order to generate swig/sanguosha_wrap.cxx.
Make sure that swig/sanguosha_wrap.cxx exists, and then open the .pro file in QT Creator and build it.

*************************************

Acknowledgements:

The persons listed below are appreciated who have made considerable and significant contributions to this project:<br>
	Mogara - The author of QSanguosha.<br>
	Iwtmusou - The author of TouhouKill.<br>
	jjyyxx - This program would never be completed but for his technical support.<br>

The persons listed below are appreciated who have helped me a lot with my coding problems in Tieba:<br>
	独孤安河<br>
	youko1316<br>
	myetyet<br>
	Yajin°<br>
	xtfnfhvzzv<br>
	czb0598<br>
	doublebit<br>

What is more, we shall express my gratitude and apology to the illustrators of the raw images of the generals and cards, whose names are
presented in illustrators.xlsx. Since we do not have a single illustrator in our studio, we have to pick the pictures from Pixiv and some
other websites without being authorized. We are sorry for arbitrarily using those illustrators' works in this program, and we promise that
this program is never to be used for commercial purposes.



	