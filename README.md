TouhouKeireikaku 东方启灵阁
=====================================

TouhouKeireikaku is an open source game, which derives from Touhoukill by lwtmusou and is originally
based on QSanguosha and Touhou Project. It is never to be used for any commercial purposes.<br>
The whole project is written in C++, along with QT for GUI framework, and Lua for AI scripts.<br>

Author: Kouchya<br>
Contributors: jjyyxx, tusikalanse<br>
Organization: obstudio<br>
GitHub: https://github.com/obstudio/TouhouKeireikaku<br>
Email: kouchyakun@outlook.com<br>

*************************************

Current version: 0.2.4 Alpha

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
　　plugins/platforms<br>

If it is your first time to run the game, make sure that your device is connected to the Internet in order to download necessary files.

*************************************

For developers:

To build this project, a QT Library (5.5.1 and 5.6.1 tested, 4.x does not work) and a QT Creator (3.5.1 tested) are both needed.
Swig may be necessary as well in order to generate swig/sanguosha_wrap.cxx.
Make sure that swig/sanguosha_wrap.cxx exists, and then open the .pro file in QT Creator and build it.

*************************************

Acknowledgements:

The persons listed below are appreciated who have made significant contributions to this project:<br>
	Mogara - The author of QSanguosha.<br>
	lwtmusou - The author of TouhouKill.<br>
