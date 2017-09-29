function speak(to,type)
	if not sgs.GetConfig("AIChat", false) then return end
	if to:getState() ~= "robot" then return end

	local i =math.random(1,#sgs.ai_chat[type])
	to:speak(sgs.ai_chat[type][i])
end

function speakTrigger(card,from,to,event)
	--[[if (event=="death") and from:hasSkill("ganglie") then
		speak(from,"ganglie_death")
	end]]--

	if (event == "death") and (from:hasSkills("jiejie|shixi")) then
		speak(from, "yukari_death")
	end

	if not card then return end

	--[[if card:isKindOf("Indulgence") and (to:getHandcardNum()>to:getHp()) then
		speak(to,"indulgence")
	elseif card:isKindOf("LeijiCard") then
		speak(from,"leiji_jink")
	elseif card:isKindOf("QuhuCard") then
		speak(from,"quhu")
	elseif card:isKindOf("Slash") and from:hasSkill("wusheng") and to:hasSkill("yizhong") then
		speak(from,"wusheng_yizhong")
	elseif card:isKindOf("Slash") and to:hasSkill("yiji") and (to:getHp()<=1) then
		speak(to,"guojia_weak")
	elseif card:isKindOf("SavageAssault") and (to:hasSkill("kongcheng") or to:hasSkill("huoji")) then
		speak(to,"daxiang")
	elseif card:isKindOf("FireAttack") and to:hasSkill("luanji") then
		speak(to,"yuanshao_fire")
	end]]--

	if card:isKindOf("DaosheCard") then
		speak(from, "daoshe")
	elseif card:isKindOf("YuansheCard") then
		speak(from, "yuanshe")
	elseif card:isKindOf("ChiwaCard") then
		speak(from, "chiwa")
	end
end

sgs.ai_chat_func[sgs.SlashEffected].blindness=function(self, player, data)
	local effect= data:toSlashEffect()
	local chat ={"你们总想搞个大新闻，再把我批判一番",
				"你们……你们不要“听风就是雨”",
				"如果我身份上有了偏差，你是要负责任的"}
	if not effect.from then return end

	--[[if self:hasCrossbowEffect(effect.from) then
		table.insert(chat, "快闪，药家鑫来了。")
		table.insert(chat, "果然是连弩降智商呀。")
		table.insert(chat, "杀死我也没牌拿，真2")
	end

	if effect.from:getMark("drank") > 0 then
		table.insert(chat, "喝醉了吧，乱砍人？")
	end]]--

	if effect.from:isLord() then
		table.insert(chat, "你们主公还是要提高自己的姿势水平")
		table.insert(chat, "主公杀人，也要按照基本法，去进行的")
	end

	local index =1+ (os.time() % #chat)

	if os.time() % 10 <= 3 and not effect.to:isLord() then
		effect.to:speak(chat[index])
	end
end

sgs.ai_chat_func[sgs.Death].stupid_lord=function(self, player, data)
	local damage=data:toDeath().damage
	local chat ={"你们主公都too simple, sometimes naive!",
				"我就什么都不说，这是坠吼的！",
				}
	if damage and damage.from and damage.from:isLord() and self.role=="loyalist" and damage.to:objectName() == player:objectName() then
		local index =1+ (os.time() % #chat)
		damage.to:speak(chat[index])
	end
end

sgs.ai_chat_func[sgs.Dying].fuck_renegade=function(self, player, data)
	local dying = data:toDying()
	local chat ={"小内，我看你闷声发大财",
				"我是队友啊，你们怎么能不资瓷队友呢",
				"小内不资瓷我们，全托管吧",
				}
	if (self.role=="rebel" or self.role=="loyalist") and sgs.current_mode_players["renegade"]>0 and dying.who:objectName() == player:objectName() then
		local index =1+ (os.time() % #chat)
		player:speak(chat[index])
	end
end

sgs.ai_chat_func[sgs.EventPhaseStart].beset=function(self, player, data)
	local chat ={
		"你一个主公的命运啊，当然也要考虑到反贼的行程",
		"坦克快停下，不要急着从那个主公的身上压过去",
		"大家已经研究撅腚了，一人一下弄死",
		"主公你给我们搞的这局东西啊……Excited!",
	}
	if player:getPhase()== sgs.Player_Start and self.role=="rebel" and sgs.current_mode_players["renegade"]==0
			and sgs.current_mode_players["loyalist"]==0  and sgs.current_mode_players["rebel"]>=2 and os.time() % 10 < 4 then
		local index =1+ (os.time() % #chat)
		player:speak(chat[index])
	end
end




function SmartAI:speak(type, isFemale)
	if not sgs.GetConfig("AIChat", false) then return end
	if self.player:getState() ~= "robot" then return end

	if sgs.ai_chat[type] then
		local i =math.random(1,#sgs.ai_chat[type])
		if isFemale then type = type .. "_female" end
		self.player:speak(sgs.ai_chat[type][i])
	else
		self.player:speak(type)
	end
end

sgs.ai_chat={}

sgs.ai_chat.hostile_female=
{
"很惭愧，就拆了一点微小的手牌，谢谢大家",
"你的牌将-1s" ,
"我的牌，比你不知道高到哪里去了",
}

sgs.ai_chat.hostile={
"很惭愧，就拆了一点微小的手牌，谢谢大家",
"你的牌将-1s" ,
"我的牌，比你不知道高到哪里去了",
}

sgs.ai_chat.respond_hostile={
"你们竟……我以为偏激",
"别搞我，还是另请高明吧",
"针对也得按照地方的法律啊",
}

sgs.ai_chat.friendly=
{ "你已经被钦定了" }

sgs.ai_chat.respond_friendly=
{ "呱！" }

sgs.ai_chat.duel_female=
{
"我跟你讲，你这样子是不行的！"
}

sgs.ai_chat.duel=
{
"我跟你讲，你这样子是不行的！"
}

sgs.ai_chat.lucky=
{
"这个完全无中生有的东西我再替他说一遍",
"你们快跟我谈笑风生"
}

sgs.ai_chat.collateral_female=
{
"这个Apply for Weapon，那你要写一个报告"
}

sgs.ai_chat.collateral=
{
"这个Apply for Weapon，那你要写一个报告"
}

sgs.ai_chat.yukari_death=
{
"我告诉你们，我还是too young"
}

sgs.ai_chat.daoshe=
{
"我们东方记者跑得比西方记者还快"
}

sgs.ai_chat.yuanshe=
{
"我们新闻界非常熟悉西方的这一套理论"
}

sgs.ai_chat.chiwa=
{
"蛤神在此，尔等安敢不膜？"
}
