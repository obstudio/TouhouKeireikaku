-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	developers = {
	},

	kingdoms = {
		"hakurei",
		"moriya",
		"god",
	},
	kingdom_colors = {
		god = "#96943D",
		hakurei = "#CC6311",
		moriya = "#30A048",
	},

	package_names = {
		--卡牌包
		"StandardCard",
		"StandardExCard",
		"Maneuvering",
		--"New1v1Card",

		--"touhoucard",

		--武将包
		"THStandard",
		--"Special1v1",
		
		"Standard", --此包内带pattern的定义 不能屏蔽。。。
		"Test",

	},

	surprising_generals = {
		"Rara" ,
		"Fsu0413" ,
		"lzxqqqq1" ,
		"lzxqqqq2" ,
		"funima" ,
		"jiaoshenmeanimei" ,
	},

	hulao_packages = {
		"standard",
		"wind"
	},

	xmode_packages = {
		"standard",
		"wind",
		"fire",
		"nostal_standard",
		"nostal_wind",
	},

	easy_text = {
		"诶，这个身份牌是黄色的诶。",
		"我的选将框告诉我这局要凉……",
		"我三上限，不经打，不拉仇恨。",
		"你这操作很迷啊。",
		"骚操作，骚操作.jpg",
		"盲狙一时爽，狙完火葬场。",
		"蛤？",
		"蛤蛤。",
		"你……你不要“见得风是的雨”。",
		"我是见的多了，身经百战了。",
	},

	robot_names = {
		"神社的赛钱箱",
		"魔法蘑菇",
		"洋馆的妖精女仆",
		"冻青蛙",
		"无法往生的幽灵" ,
		"白芋头麻薯",
		"居家用人偶",
		"妖怪兔",
		"寺子屋的小孩",
		"向日葵（严禁攀折践踏）",
		"棋艺超群的天狗",
		"河童重工普通职员",
		"地狱鸦",
		"火焰猫" ,
		"“UFO”",
		"吓人用妖怪伞",
		"寻宝用鼠",
		"非想天则",
		"修行不足的狸猫",
		"防腐僵尸",
		"付丧神（暂时）",
		"人间之里的普通人",
		"白泽球" ,
		"毛玉" ,
		"油库里" ,
		"罪袋" ,
	},

	roles_ban = {
	},

	kof_ban = {
	},

	hulao_ban = {
	},

	xmode_ban = {
	},

	basara_ban = {
	},

	hegemony_ban = {
	},

	pairs_ban = {
	},

	convert_pairs = {
	},

	bgm_convert_pairs = {
		--BGM： 由于开始尝试加入arrange代替原曲，曲目对应关系可能有变，以后转换列表还要重新整理。
		"reimu_old->reimu_sp",
		"marisa_old->marisa_slm",
		"suika_sp->suika",
		"kosuzu->akyuu",
		"tokiko->rinnosuke",
		"unzan->ichirin",
		"mokou_sp|mokou_ndj->mokou",
		"nue_slm->nue",
		"sanae_slm|sanae_sp|sanae_ndj->sanae",
		"yorihime->toyohime",
		"youmu_ndj|youmu_slm|youki->youmu",
		"yukari_sp|yukari_ndj->yukari",
		"ran_sp->ran",
		"yuyuko_sp->yuyuko",
		"myouren->byakuren",
		"leira->prismriver",
		"cirno_sp->cirno",
		"mamizou_sp->mamizou",
		"sakuya_sp->sakuya",
		"reisen2->tewi",
		"merry|merry_ndj|renko_ndj->renko",
		"lunar|star->sunny",
		"kogasa_slm->kogasa",
		"shanghai->alice",
		"kaguya_ndj->kaguya",
	}
}
