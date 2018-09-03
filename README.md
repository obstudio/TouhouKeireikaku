# TouhouKeireikaku 东方启灵阁

## 简介

东方启灵阁是一款以三国杀规则为原型的、结合东方Project主题的卡牌游戏，其前身为lwtmusou制作的[东方杀（touhoukill）](https://github.com/lwtmusou/touhoukill)，以及Mogara开发的著名的开源三国杀项目——太阳神三国杀。东方启灵阁源代码主要使用C++结合Qt库编写，AI部分使用Lua语言设计，音频处理部分使用FMOD。

## 项目信息及联系方式

- 开发团队：obstudio
- 源码：https://github.com/obstudio/TouhouKeireikaku
- 技能&创意：Kouchya, githubshn, shigma, jjyyxx
- 插画设计：见illustrators.xlsx文档
- 代码开发：Kouchya, jjyyxx
- 官服维护：jjyyxx, NN708, Kouchya
- 官方开发群：583625902
- 联系邮箱：kouchyakun@outlook.com

## 更新日志

当前版本: 0.2.4 Alpha

- 重做了etc文件夹下的座次身份优选表
- 对部分角色嘲讽及防御力重新进行了评估
- 修复了黑谷山女蛛丝AI代码导致其不能出牌的bug
- 重新设置了AI对主公选将的评估
- 转移了官服、登录验证、上传积分等数据库地址
- 修改了以下角色的称号：
  - 帕秋莉·诺蕾姬
  - 爱丽丝·玛格特罗伊德
- 增强了芙兰握碎的威慑
- 大幅提高了嘲讽值在AI评估角色防御力时的影响
- 大幅提高了帕秋莉集齐火水木金土标记未开大时的嘲讽
- 增强了诹访子祟蛙的威慑
- 增强了早苗开海的威慑
- 修复了AI衣玖偶尔磁探自己的bug
- 将游戏背景设定改为主公角色对应的场所
  - 各角色场所在Package翻译文件中列出
  - 同时删去了backdrop文件夹下多余的文件
- 换掉了以下角色作为主公时的游戏bgm：
  - 帕秋莉·诺蕾姬
  - 东风谷早苗
- 修正了河城荷取粼殇的描述错误
- 修正了AI琪露诺冰瀑拿队友铁轮时导致游戏崩溃的bug
- 增强了AI帕秋莉保护标记为贤石做准备的意识
- 现在AI早苗能在必要的时候开海令队友弃牌了
- 删去了几个弱主公角色
- 修复了被瘴气角色回合结束后瘴气效果消失的bug
- 现在被瘴气角色的标记区会显示一个“瘴”字样标记
- 现在AI选将时会考虑部分角色的强度

## 玩家须知

运行游戏程序时可能会出现“无法定位输入点于XXX动态链接库”的报错，或出现图片加载不出的情况。此时请确保以下文件或文件夹在你的游戏目录下（它们都可以在Qt5.6安装目录下的5.6/mingw49_32目录下找到）：

- icudt54.dll（位于bin目录）
- icuin54.dll（位于bin目录）
- icuuc54.dll（位于bin目录）
- Qt5Declarative.dll（位于bin目录）
- Qt5Qml.dll（位于bin目录）
- Qt5Script.dll（位于bin目录）
- Qt5Sql.dll（位于bin目录）
- libgcc_s_dw2-1.dl（位于bin目录）
- libstdc++-6.dll（位于bin目录）
- libwinpthread-1.dll（位于bin目录）
- Qt5XmlPatterns.dll（位于bin目录）
- imageformats（位于plugins目录）
- platforms（位于plugins目录）

**运行时请务必保持网络连接，否则将无法下载运行游戏必要的文件。**

目前游戏官方文档尚不完善，对游戏内容有任何疑问或bug反馈请联系开发者Kouchya（联系方式见“项目信息及联系方式”段落）。

## 开发者须知

本项目遵循GPLv3开源协议，任何以本项目为基础开发的项目若违反协议，产生后果自负。

本项目使用Qt 5.6.1提供的功能库和编译工具（QtCreator 4.0.1）。编译本项目需要安装swig来生成游戏目录下的swig/sanguosha_wrap.cxx。

目前游戏开发官方文档尚不完善，开发过程中如有疑问请联系开发者Kouchya（联系方式见“项目信息及联系方式”段落）。