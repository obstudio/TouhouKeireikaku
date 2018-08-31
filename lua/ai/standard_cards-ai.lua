function SmartAI:canAttack(enemy, attacker, nature)
	attacker = attacker or self.player
	nature = nature or sgs.DamageStruct_Normal
	local damage = 1
	if nature == sgs.DamageStruct_Fire and not enemy:hasArmorEffect("SilverLion") then
		if enemy:hasArmorEffect("Vine") then damage = damage + 1 end
	end
	if #self.enemies == 1 then return true end
	if self:getDamagedEffects(enemy, attacker) or (self:needToLoseHp(enemy, attacker, false, true) and #self.enemies > 1) or not sgs.isGoodTarget(enemy, self.enemies, self) then return false end
	if self:objectiveLevel(enemy) <= 2 or self:cantbeHurt(enemy, self.player, damage) or not self:damageIsEffective(enemy, nature, attacker) then return false end
	if nature ~= sgs.DamageStruct_Normal and enemy:isChained() and not self:isGoodChainTarget(enemy, self.player, nature) then return false end
	return true
end

function hasExplicitRebel(room)
	for _, player in sgs.qlist(room:getAllPlayers()) do
		if sgs.isRolePredictable() and  sgs.evaluatePlayerRole(player) == "rebel" then return true end
		if sgs.compareRoleEvaluation(player, "rebel", "loyalist") == "rebel" then return true end
	end
	return false
end

function sgs.isGoodHp(player)
	local goodHp = player:getHp() > 1 or getCardsNum("Peach", player) >= 1 or getCardsNum("Analeptic", player) >= 1
					or hasBuquEffect(player)
					or (player:hasSkill("niepan") and player:getMark("@nirvana") > 0)
					or (player:hasSkill("fuli") and player:getMark("@laoji") > 0)
	if goodHp then
		return goodHp
	else
		for _, p in sgs.qlist(global_room:getOtherPlayers(player)) do
			if sgs.compareRoleEvaluation(p,"rebel","loyalist")==sgs.compareRoleEvaluation(player,"rebel","loyalist")
					and getCardsNum("Peach",p)>0 and not global_room:getCurrent():hasSkill("wansha") then
				return true
			end
		end
		return false
	end
end

--½ç¶¨ÂôÑª£¿
--¶«·½ÆôÁé¸óÏà¹Ø
function sgs.isGoodTarget(player, targets, self, isSlash)
	local arr = {"jieming", "yiji", "guixin", "fangzhu", "neoganglie", "nosmiji", "xuehen", "xueji", "diaoou", "xuebeng", "kaihai", "suiwa", "huanni"}
	local m_skill = false
	local attacker = global_room:getCurrent()

	if targets and type(targets)=="table" then
		if #targets == 1 then return true end
		local foundtarget = false
		for i = 1, #targets, 1 do
			if sgs.isGoodTarget(targets[i]) and not self:cantbeHurt(targets[i]) then
				foundtarget = true
				break
			end
		end
		if not foundtarget then return true end
	end

	for _, masochism in ipairs(arr) do
		if player:hasSkill(masochism) then
			if masochism == "nosmiji" and player:isWounded() then m_skill = false
			elseif masochism == "xueji" and player:isWounded() then m_skill = false
			elseif masochism == "jieming" and self and self:getJiemingChaofeng(player) > -4 then m_skill = false
			elseif masochism == "yiji" and self and not self:findFriendsByType(sgs.Friend_Draw, player) then m_skill = false
			elseif masochism == "diaoou" and self then
				if player:isNude() then
					m_skill = false
				else
					local has_ningyou = false
					for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
						if p:getMark("@ningyou") > 0 then
							has_ningyou = true
							break
						end
					end
					if not has_ningyou then
						m_skill = false
					end
				end
			elseif masochism == "xuebeng" and (player:getPile("snow"):length() >= 2 or (self and self:isVeryWeak(player))) then m_skill = false
			elseif masochism == "kaihai" and self then
				for _, p in sgs.qlist(self.room:getAlivePlayers()) do
					if not self:isFriend(player, p) and p:getHandcardNum() > p:getHp() * 1.5 then
						m_skill = true
						break
					end
				end
				for _, p in sgs.qlist(self.room:getAlivePlayers()) do
					if self:isFriend(player, p) and p:getHandcardNum() > p:getHp() and p:getHp() < p:getLostHp() and (p:getHandcardNum() < p:getLostHp() or (p:getHandcardNum() == p:getLostHp() and p:getHandcardNum() > 2)) then
						m_skill = true
						break
					end
				end

				local first_step = true
				for _, p in sgs.qlist(self.room:getAlivePlayers()) do
					if not self:isFriend(player, p) and p:getHandcardNum() > p:getHp() then
						first_step = true
						break
					end
				end
				if not first_step then
					m_skill = false
				end
				for _, p in sgs.qlist(self.room:getAlivePlayers()) do
					if self:isFriend(player, p) and p:getHandcardNum() < p:getLostHp() then
						m_skill = true
						break
					end
				end

				for _, p in sgs.qlist(self.room:getAlivePlayers()) do
					if self:isFriend(player, p) and p:getHandcardNum() > p:getHp() then
						local y = p:getHandcardNum() - p:getHp()
						for _, p2 in sgs.qlist(self.room:getOtherPlayers(p)) do
							if self:isFriend(player, p2) and p2:getHandcardNum() < p2:getLostHp() and self:isWeakerThan(p2, p) and not self:isVeryWeak(p) and p:getHandcardNum() - p:getHp() < p2:getLostHp() - p2:getHandcardNum() then
								m_skill = true
								break
							end
						end
					end
				end
				if m_skill then break end
			elseif masochism == "suiwa" and self then
				for _, p in sgs.qlist(self.room:getAlivePlayers()) do
					if p:getArmor() and p:getArmor():isKindOf("SilverLion") then
						m_skill = true
						break
					end
					if p:getArmor() and p:objectName() ~= player:objectName() and player:getArmor() == nil then
						m_skill = true
						break
					end
				end
				if m_skill then break end
			elseif masochism == "huanni" and player:isKongcheng() then m_skill = false
			elseif masochism == "feixiang" and self then
				local flag = false
				for _, p in sgs.qlist(self.room:getAlivePlayers()) do
					if self:isFriend(player, p) and (p:getHandcardNum() < p:getHp() or
							(p:getHandcardNum() < p:getHp() - 1 and p:objectName() == player:objectName())) then
						flag = true
						break
					end
				end
				if not flag then m_skill = false end
			else
				m_skill = true
				break
			end
		end
	end

	if player:hasSkill("huilei") and not player:isLord() and player:getHp() == 1 then
		if attacker and attacker:getHandcardNum() >= 4 then return false end
		return sgs.compareRoleEvaluation(player, "rebel", "loyalist") == "rebel"
	end

	if player:hasLordSkill("shichou") and player:getMark("@hate") == 0 then
		for _, p in sgs.qlist(player:getRoom():getOtherPlayers(player)) do
			if p:getMark("hate_" .. player:objectName()) > 0 and p:getMark("@hate_to") > 0 then
				return false
			end
		end
	end
	
	if player:hasSkill("miezui") then
		if attacker:getHandcardNum() < 2 then
			return false
		end
		local cards = sgs.QList2Table(attacker:getHandcards())
		local id1 = -1
		local id2 = -1
		for i = 1, #cards - 1, 1 do
			local c1 = cards[i]
			for j = i + 1, #cards, 1 do
				local c2 = cards[j]
				if c1:getSuit() == c2:getSuit() and not c1:isKindOf("Peach") and not c2:isKindOf("Peach")
						and not c1:isKindOf("ExNihilo") and not c2:isKindOf("ExNihilo")
						and not (c1:isKindOf("Analeptic") and c2:isKindOf("Analeptic")) then
					id1 = c1:getEffectiveId()
					id2 = c2:getEffectiveId()
				end
			end
		end
		if id1 < 0 or id2 < 0 then
			return false
		end
	end
	
	if isSlash and player:hasSkill("wosui") and self then
		local current = self.room:getCurrent()
		if self:isFriend(current) and not self:isFriend(player, current) and player:getHp() <= current:getHp() then
			if getKnownCard(player, self.player, "Jink", true) > 0 then return false end
			if not (player:isKongcheng() and not player:hasArmorEffect("EightDiagram") and player:getPile("wooden_ox"):isEmpty()) then
				local all_num = player:getHandcardNum() + player:getPile("wooden_ox"):length()
				local visible_num = 0
				local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), player:objectName())
				for _, c in sgs.qlist(player:getHandcards()) do
					if (c:hasFlag("visible") or c:hasFlag(flag)) and not c:isKindOf("Jink") then
						visible_num = visible_num + 1
					end
				end
				for _, id in sgs.qlist(player:getPile("wooden_ox")) do
					local c = sgs.Sanguosha:getCard(id)
					if (c:hasFlag("visible") or c:hasFlag(flag)) and not c:isKindOf("Jink") then
						visible_num = visible_num + 1
					end
				end
				if (visible_num < all_num or player:hasArmorEffect("EightDiagram")) and current:getCards("he"):length() <= 4
						and (current:getHandcardNum() <= 3 or self:isWeak(current) or self:hasKeyEquip(current)) then
					return false
				end
			end
		end
	end
	
	if isSlash and self and (self:hasCrossbowEffect() or self:getCardsNum("Crossbow") > 0) and self:getCardsNum("Slash") > player:getHp() then
		return true
	end

	if m_skill and sgs.isGoodHp(player) then
		return false
	else
		return true
	end
end

function sgs.getDefenseSlash(player, self)
--±ØÖÐµÄÇ°ÌáÏÂ£¬1ÑªµÄÓÅÏÈ¶È°¡ ÎÒ²Á
--ÊµÀýÖÐ»á³öÏÖ ÊÖ³Ö¹áÊ¯¸« Ò»°ÑÅÆ È¥¿³2Ñª¿ÕÊÖÅÆ¶ø²»È¥¿³1Ñª4ÕÅÅÆµÄ¡£¡£¡£¡£
	if not player then return 0 end
	local attacker = global_room:getCurrent()
	local defense = getCardsNum("Jink", player, attacker)

	local knownJink = getKnownCard(player, attacker, "Jink", true)

	if sgs.card_lack[player:objectName()]["Jink"] == 1 and knownJink == 0 then defense = 0 end

	defense = defense + knownJink * 1.2

	local hasEightDiagram = false

	if player:hasSkill("juwang") then
		defense = defense + 0.8
	end
	if player:hasSkill("langying") and player:getCards("e"):length()>0 then
		defense = defense + 2.2
	end
	if self and player:hasSkills("lingqi") then
		if (self:invokeTouhouJudge(player)) then
			defense = defense + 10
		end
		if player:hasSkills("qixiang") and player:getHandcardNum() < player:getMaxCards() then
			defense = defense + 10
		end
	end
	if player:hasSkill("juxian") and player:faceUp() then
		defense = defense + 10
	end

	if player:hasSkill("wushou") then
		defense = defense + (0.6 * (1 + player:getLostHp()))
	end
	if player:hasSkill("guangji") and not player:getPile("tianyi"):isEmpty() then
		defense = defense + 10
	end
	
	if player:hasSkill("wosui") and self and getKnownCard(player, self.player, "Jink", true) < player:getHandcardNum() then
		defense = defense + 1
	end
	
	if (player:hasArmorEffect("EightDiagram") or (player:hasSkill("bazhen") and not player:getArmor())
			or (self and self:hasJiahuEffect(player))) and not IgnoreArmor(attacker, player) then
		hasEightDiagram = true
	end

	if hasEightDiagram then
		defense = defense + 1.3
		if player:hasSkill("tiandu") then defense = defense + 0.6 end
		if player:hasSkill("leiji") then defense = defense + 0.4 end
		if player:hasSkill("wosui") then defense = defense + 0.6 end
		if self then
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				if p:hasSkill("qiangyun") and getKnownCard(p, self.player, "club", false) + getKnownCard(p, self.player, "spade", false) < p:getHandcardNum()
						and self:isFriend(p, player) then
					defense = defense + 0.3
				end
			end
		end
	end

	if player:hasSkill("mingzhe") and getCardsNum("Jink", player) >= 1 then
		defense = defense + 0.2
	end


	if player:hasSkill("tuntian") and player:hasSkill("zaoxian") and getCardsNum("Jink", player) >= 1 then
		defense = defense + 1.5
	end

	if player:hasSkill("aocai") and player:getPhase() == sgs.Player_NotActive then defense = defense + 0.5 end
	if player:hasSkill("wanrong") and not hasManjuanEffect(player) then defense = defense + 0.5 end




	local hujiaJink = 0
	if player:hasLordSkill("hujia") then
		local lieges = global_room:getLieges("wei", player)
		for _, liege in sgs.qlist(lieges) do
			if sgs.compareRoleEvaluation(liege,"rebel","loyalist") == sgs.compareRoleEvaluation(player,"rebel","loyalist") then
				hujiaJink = hujiaJink + getCardsNum("Jink",liege)
				if liege:hasArmorEffect("EightDiagram") then hujiaJink = hujiaJink + 0.8 end
			end
		end
		defense = defense + hujiaJink
	end

	local tianrenJink = 0 --ÀàËÆ»¤¼Ý
	if player:hasLordSkill("tianren") and player:getPhase()==sgs.Player_NotActive  then
		local lieges = global_room:getLieges("zhan", player)
		for _, liege in sgs.qlist(lieges) do
			if sgs.compareRoleEvaluation(liege,"rebel","loyalist") == sgs.compareRoleEvaluation(player,"rebel","loyalist") then
				tianrenJink = tianrenJink + getCardsNum("Jink",liege)
				if liege:hasArmorEffect("EightDiagram") then tianrenJink = tianrenJink + 0.8 end
			end
		end
		defense = defense + tianrenJink
	end

	if attacker:canSlashWithoutCrossbow() and attacker:getPhase() == sgs.Player_Play then
		local hcard = player:getHandcardNum()
		if attacker:hasSkill("liegong") and (hcard >= attacker:getHp() or hcard <= attacker:getAttackRange()) then defense = 0 end
		if attacker:hasSkill("kofliegong") and hcard >= attacker:getHp() then defense = 0 end
	end
	--¿¼ÂÇÍþÑ¹£¿£¿
	local jiangqin = global_room:findPlayerBySkillName("niaoxiang")
	local need_double_jink = attacker:hasSkills("wushuang|drwushuang")
							or (attacker:hasSkill("roulin") and player:isFemale())
							or (player:hasSkill("roulin") and attacker:isFemale())
							or (jiangqin and jiangqin:isAdjacentTo(player) and attacker:isAdjacentTo(player) and self and self:isFriend(jiangqin, attacker))
	if need_double_jink and getKnownCard(player, attacker, "Jink", true, "hes") < 2
		and getCardsNum("Jink", player) < 1.5
		and (not player:hasLordSkill("hujia") or hujiaJink < 2)
		and (not player:hasLordSkill("tianren") or player:getPhase()~=sgs.Player_NotActive  or tianrenJink < 2)
		then
		defense = 0
	end
	--´óºÈ´øÀ´µÄÎÊÌâ ÌìÈË²»¿¼ÂÇ
	--if attacker:hasSkill("dahe") and player:hasFlag("dahe")   and getKnownCard(player, attacker, "Jink", true, "hes") == 0 and getKnownNum(player) == player:getHandcardNum()
	--  and not (player:hasLordSkill("hujia") and hujiaJink >= 1) then
	--  defense = 0
	--end

	local jink = sgs.cloneCard("jink")
	if player:isCardLimited(jink, sgs.Card_MethodUse) then defense = 0 end

	defense = defense + math.min(player:getHp() * 0.45, 10)

	if attacker then
		local m = sgs.masochism_skill:split("|")
		for _, masochism in ipairs(m) do
			if player:hasSkill(masochism) and sgs.isGoodHp(player) then
				defense = defense + 1
			end
		end
		if player:hasSkill("jieming") then defense = defense + 4 end
		if player:hasSkill("yiji") then defense = defense + 4 end
		if player:hasSkill("guixin") then defense = defense + 4 end
	end

	if not sgs.isGoodTarget(player) then defense = defense + 10 end

	if player:hasSkills("nosrende|rende") and player:getHp() > 2 then defense = defense + 1 end
	if player:hasSkill("kuanggu") and player:getHp() > 1 then defense = defense + 0.2 end
	if player:hasSkill("zaiqi") and player:getHp() > 1 then defense = defense + 0.35 end
	if player:hasSkill("tianming") then defense = defense + 0.1 end

	if player:getHp() > getBestHp(player) then defense = defense + 0.8 end
	if player:getHp() <= 2 then defense = defense - 0.4 end

	local playernum = global_room:alivePlayerCount()
	if (player:getSeat()-attacker:getSeat()) % playernum >= playernum-2 and playernum>3 and player:getHandcardNum()<=2 and player:getHp()<=2 then
		defense = defense - 0.4
	end

	--if player:hasSkill("tianxiang") then defense = defense + player:getHandcardNum() * 0.5 end
	--ÕâÒ»²½±¾Éí»¹ÊÇÂÔÓÐÎÊÌâ 0ÅÆ4ÑªµÄÈË¿ÛÏÂÀ´·ÀÓùÖµ±È1ÑªÖ»ÓÐ1ÕÅÃ÷É±µÄÈË·ÀÓùÖµ»¹¸ß
	--Ñ¹¸ù²»¶®¼¯»ðÀíÂÛ  Ö»ÒªÃ»ÅÆ¾ÍÄÜ¿Û¼õÁË´ó°Ñ·ÀÓùÖµ¡£¡£¡£ 2ÑªÖÒ»òÕß¿Õ³ÇÖÒ °ï1ÑªÖ÷µ²µ¶²»Òª²»ÒªµÄ
	 --if player:getHandcardNum() == 0 and player:getPile("wooden_ox"):isEmpty()
	 if sgs.card_lack[player:objectName()]["Jink"] == 1 and knownJink == 0
	 and hujiaJink == 0  and tianrenJink==0
	 and not player:hasSkill("kongcheng") then
		if player:getHp() <= 1 then defense = defense - 2.5 end
		if player:getHp() == 2 then defense = defense - 1.5 end
		if not hasEightDiagram then defense = defense - 1 end
		if attacker:hasWeapon("GudingBlade") and player:getHandcardNum() == 0
		   and not (player:hasArmorEffect("SilverLion") and not IgnoreArmor(attacker, player)) then
			 defense = defense - 2
		end
	 end

	local has_fire_slash
	local cards = sgs.QList2Table(attacker:getHandcards())
	for i = 1, #cards, 1 do
		if (attacker:hasWeapon("Fan") and cards[i]:objectName() == "slash" and not cards[i]:isKindOf("ThunderSlash"))
		or cards[i]:isKindOf("FireSlash")
		or (attacker:hasSkill("here") or player:hasSkill("here") and cards[i]:isKindOf("Slash")) then
			has_fire_slash = true
			break
		end
	end

	if player:hasArmorEffect("Vine") and not IgnoreArmor(attacker, player) and has_fire_slash then
		defense = defense - 0.6
	end

	if isLord(player) then
		defense = defense - 0.4
		if sgs.isLordInDanger() then defense = defense - 0.7 end
	end


	if (sgs.ai_chaofeng[player:getGeneralName()] or 0) >=3 then
		defense = defense - math.max(6, (sgs.ai_chaofeng[player:getGeneralName()] or 0)) * 0.035
	end

	if player:hasFlag("ZhangqiFlag") then
		defense = defense - 2.6
		if self and self:isWeak(player) then
			defense = defense - 2
		end
		if knownJink == 0 and not hasEightDiagram then
			defense = defense - 4
		end
	end

	if player:getMark("@philosopher") > 0 and player:getMark("@fire") * player:getMark("@water") * player:getMark("@wood") * player:getMark("@gold") * player:getMark("@earth") > 0 then
		defense = defense - 10
	end

	if player:hasSkill("suiwa") and self then
		local stop = false
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if p:getArmor() and p:getArmor():isKindOf("SilverLion") then
				defense = defense + 20
				stop = true
				break
			end
		end
		if not stop then
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				if p:getArmor() and p:objectName() ~= player:objectName() and player:getArmor() == nil then
					defense = defense + 10
					stop = true
					break
				end
			end
		end
	end

	if player:hasSkill("shiling") and not player:faceUp() then
		if player:getHandcardNum() >= 2 then
			defense = defense + 10
		elseif player:getHandcardNum() == 1 then
			defense = defense + player:getHp() * 3 - 5
		end
	end

	if not player:faceUp() then defense = defense - 0.35 end

	if player:containsTrick("indulgence") and not player:containsTrick("YanxiaoCard") then defense = defense - 0.15 end
	if player:containsTrick("supply_shortage") and not player:containsTrick("YanxiaoCard") then defense = defense - 0.15 end

	if (attacker:hasSkill("roulin") and player:isFemale()) or (attacker:isFemale() and player:hasSkill("roulin")) then
		defense = defense - 2.4
	end

	if not hasEightDiagram then
		if player:hasSkill("jijiu") then defense = defense - 3 end
		if player:hasSkill("dimeng") then defense = defense - 2.5 end
		if player:hasSkill("guzheng") and knownJink == 0 then defense = defense - 2.5 end
		if player:hasSkill("qiaobian") then defense = defense - 2.4 end
		if player:hasSkill("jieyin") then defense = defense - 2.3 end
		if player:hasSkills("noslijian|lijian") then defense = defense - 2.2 end
		if player:hasSkill("nosmiji") and player:isWounded() then defense = defense - 1.5 end
		if player:hasSkill("xiliang") and knownJink == 0 then defense = defense - 2 end
		--if player:hasSkill("shouye") then defense = defense - 2 end
		if player:hasSkill("herong") then defense = defense - 1.5 end
	end


	return defense
end

sgs.ai_compare_funcs["defenseSlash"] = function(a,b)
	return sgs.getDefenseSlash(a) < sgs.getDefenseSlash(b)
end

--¶«·½É±Ïà¹Ø
--¡¾¶³½á¡¿¡¾ËÀµû¡¿
--¡¾ÉñµÂ¡¿
function SmartAI:slashProhibit(card, enemy, from)
	local mode = self.room:getMode()
	if mode:find("_mini_36") then return self.player:hasSkill("keji") end
	card = card or sgs.cloneCard("slash", sgs.Card_NoSuit, 0)
	from = from or self.player
	if from:hasSkill("here") or enemy:hasSkill("here") then
		card= self:copyHereSlash(card)
	end

	local nature = card:isKindOf("FireSlash") and sgs.DamageStruct_Fire
					or card:isKindOf("ThunderSlash") and sgs.DamageStruct_Thunder
	if self:isEnemy(enemy,from) and from:hasSkill("shende") and self:getOverflow(from,from:getMaxCards())>0 then
		return false
	end
	for _, askill in sgs.qlist(enemy:getVisibleSkillList()) do
		local filter = sgs.ai_slash_prohibit[askill:objectName()]
		if filter and type(filter) == "function" and filter(self, from, enemy, card) then return true end
	end

	if self:isFriend(enemy, from) then
		if self:sidieEffect(from) then return false end
		if from:hasSkill("dongjie") and not from:hasFlag("dongjie") and not enemy:faceUp() then return false end
		if card:isKindOf("FireSlash") or from:hasWeapon("Fan") or from:hasSkill("zonghuo")  then
			if enemy:hasArmorEffect("Vine") and not (enemy:isChained() and self:isGoodChainTarget(enemy, nil, nil, nil, card)) then return true end
		end
		if enemy:isChained() and (card:isKindOf("NatureSlash") or from:hasSkill("zonghuo")) and self:slashIsEffective(card, enemy, from)
			and (not self:isGoodChainTarget(enemy, from, nature, nil, card)) then return true end
		if getCardsNum("Jink",enemy, from) == 0 and enemy:getHp() < 2 and self:slashIsEffective(card, enemy, from) then return true end
		if enemy:isLord() and self:isWeak(enemy) and self:slashIsEffective(card, enemy, from) then return true end
		if from:hasWeapon("GudingBlade") and enemy:isKongcheng() then return true end
	else
		if self:slashProhibitToEghitDiagram(card,from,enemy)
		or self:slashProhibitToDiaopingTarget(card,from,enemy) then
			return true
		end
		if (card:isKindOf("NatureSlash") or from:hasSkill("zonghuo")) and enemy:isChained()
			and not self:isGoodChainTarget(enemy, from, nature, nil, card) and self:slashIsEffective(card, enemy, from) then
			return true
		end
	end

	return self.room:isProhibited(from, enemy, card) or not self:slashIsEffective(card, enemy, from) -- @todo: param of slashIsEffective
end

function SmartAI:canLiuli(other, another)
	if not other:hasSkills("liuli|liuyue") then return false end
	if type(another) == "table" then
		if #another == 0 then return false end
		for _, target in ipairs(another) do
			if target:getHp() < 3 and self:canLiuli(other, target) then return true end
		end
		return false
	end

	if not self:needToLoseHp(another, self.player, true) or not self:getDamagedEffects(another, self.player, true) then return false end
	local n = other:getHandcardNum()
	if n > 0 and (other:distanceTo(another) <= other:getAttackRange()) then return true
	elseif other:getWeapon() and other:getOffensiveHorse() and (other:distanceTo(another) <= other:getAttackRange()) then return true
	elseif other:getWeapon() or other:getOffensiveHorse() then return other:distanceTo(another) <= 1
	else return false end
end

--¶«·½É±Ïà¹Ø
--self:touhouDamage
--´Ë´¦ÎÞÐ§ ai¾Í»áÄ¬ÈÏ²»Ê¹ÓÃÉ±¶Ô¸ÃÄ¿±ê
--¡¾ÐéÓ°¡¿ÉÔÎ¢ÌØÊâ ²»È»ai²»»áÉ±»ÃÃÎ
--ÎÞÐ§ºÍ·ÀÖ¹ÉËº¦Ïµ»¹ÊÇ¸Ã·Ö¿ª£¿£¿
--¿¼ÂÇ¡¾ÀúÕ½¡¿ºÍ¡¾ÉñµÂ¡¿µÄÇé¿ö£¬Ã»ÓÐÓÐÐ§Ä¿±ê£¬ÓÐÎÞÐ§Ä¿±ê£¬µ«ÊÇÊÖÅÆÒç³öÒ²Òª³öÉ±¡£
--overflowÏÞÖÆ²éÑ¯ÆäËûÄ¿±êµÄslashIsEffective Ã»ÓÐÓÐÐ§Ä¿±êÁË Ôò ´ËÎÞÐ§Ä¿±êÒ²±äÎªÓÐÐ§£¿£¿£¿
--µ«ÊÇÉñµÂÉ±¾²µç»¹ÊÇµ°ÌÛ¡£¡£¡£
function SmartAI:slashIsEffective(slash, to, from, ignore_armor)
	if not slash or not to then self.room:writeToConsole(debug.traceback()) return end
	from = from or self.player
	if from:hasSkill("here") or to:hasSkill("here") then
		slash= self:copyHereSlash(slash)
	end

	if self:touhouEffectNullify(slash,from,to) then return false end
	if self:canNizhuan(to, from) then
		return false
	end
	if to:hasSkill("zuixiang") and to:isLocked(slash) then return false end
	if to:hasSkill("yizhong") and not to:getArmor() then
		if slash:isBlack() then
			return false
		end
	end
	if to:hasSkill("yicun")  then
		if self:yicunEffective(slash, to, from) then
			return false
		end
	end
	if to:hasSkill("huiwu")  and self:isFriend(to, from) then--Ã»¿¼ÂÇËÀµû
		return false
	end
	if self:hasHeavySlashDamage(from, slash, to) and to:hasSkill("shengchun") then
		return false
	end
	
	if self:canMaoyouSelf(to) and to:getHp() == 1 and self:isEnemy(to) then
		return false
	end

	local natures = {
		Slash = sgs.DamageStruct_Normal,
		FireSlash = sgs.DamageStruct_Fire,
		ThunderSlash = sgs.DamageStruct_Thunder,
	}

	if not ignore_armor and from:objectName() == self.player:objectName() then
		if to:getArmor() and from:hasSkill("moukui") then
			if not self:isFriend(to) or (to:getArmor() and self:needToThrowArmor(to)) then
				if not (self:isEnemy(to) and self:doNotDiscard(to)) then
					local id = self:askForCardChosen(to, "hes", "moukui")
					if id == to:getArmor():getEffectiveId() then ignore_armor = true end
				end
			end
		end
	end
	
	if not ignore_armor and  to:hasArmorEffect("IronArmor")  then
		if slash:isKindOf("NatureSlash") then return false end
		if not slash:isKindOf("NatureSlash") and from:hasWeapon("Fan") and self:isFriend(from, to) then return false end
	end

	local nature = natures[slash:getClassName()]
	self.equipsToDec = sgs.getCardNumAtCertainPlace(slash, from, sgs.Player_PlaceEquip)
	local eff = self:damageIsEffective(to, nature, from)
	self.equipsToDec = 0
	if not eff then return false end

	if to:hasArmorEffect("Vine") and self:getCardId("FireSlash") and slash:isKindOf("ThunderSlash") and self:objectiveLevel(to) >= 3 then
		 return false
	end

	if IgnoreArmor(from, to) or ignore_armor then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "RenwangShield" then
			--[[if not self:isFriend(to,from) and from:hasSkill("guaili")
			and from:getHandcardNum()>1 then
				return true
			end]]
			--if from:hasSkill("louguan") then
			--  return true
			--end
			return not slash:isBlack()
		elseif armor:objectName() == "Vine" then
			local skill_name = slash:getSkillName() or ""
			local can_convert = false
			if skill_name == "guhuo" then
				can_convert = true
			else
				local skill = sgs.Sanguosha:getSkill(skill_name)
				if not skill or skill:inherits("FilterSkill") then
					can_convert = true
				end
			end
			return nature ~= sgs.DamageStruct_Normal or (can_convert and (from:hasWeapon("Fan") or (from:hasSkill("lihuo") and not self:isWeak(from))))
		end
	end

	if to:hasSkill("xuying") and to:getHandcardNum() > 0 then return true end
	if to:hasSkill("zhengti") then--ÑÏ¸ñÀ´½²Ó¦¸ÃÍùºóÅ²
		local zhengti, transfer_to = self:zhengtiParse(from,to)
		if zhengti then
			return false
		elseif transfer_to then
			if self:isFriend(from, transfer_to) then
				return false
			else
				to = transfer_to
			end
		end
	end


	-- ÊÕµ½0ÉËº¦·ÀÖ¹ÉËº¦Ê±
	local fakeDamage = sgs.DamageStruct(slash, from, to, 1, self:touhouDamageNature(slash, from, to))
	if self:touhouDamage(fakeDamage,from,to).damage <= 0 then
		local effect, willEffect = self:touhouDamageEffect(fakeDamage,from,to)
		if not effect then
			return false
		elseif not willEffect then
			return false
		end
	end
	return true
end

function SmartAI:slashIsAvailable(player, slash) -- @todo: param of slashIsAvailable
	player = player or self.player
	slash = slash or self:getCard("Slash", player)
	if not slash or not slash:isKindOf("Slash") then slash = sgs.cloneCard("slash") end
	assert(slash)
	return slash:isAvailable(player) or slash:hasFlag("jiuhao")
end


--¶«·½É±Ïà¹Ø
--¡¾¶³½á¡¿¡¾ËÀµû¡¿¡¾°µÓò¡¿
--¡¾»ÃÊÓ¡¿¡¾ÐéÓ°¡¿
--¡¾ÃÎÏÖ¡¿¡¾»¯Áú¡¿ËÍ¾õÐÑ£¿£¿
function SmartAI:isPriorFriendOfSlash(friend, card, source)
	source = source or self.player
	if source:hasSkill("canye") and not friend:isNude() and source:getHp() > friend:getHp() then return false end
	local huatuo = self.room:findPlayerBySkillName("jijiu")
	if source:hasSkill("here") or friend:hasSkill("here") then
		card= self:copyHereSlash(card)
	end
	if friend:hasSkill("xuying") then return false end
	--¿ÉÒÔÈ¡ÏûÉËº¦ ËùÒÔºÍhasheavyslashdamageÎÞ¹Ø£¿£¿
	if source:hasSkill("dongjie") and not source:hasFlag("dongjie") and not friend:faceUp() then return true end
	if friend:hasSkill("anyu") and not friend:faceUp() and card:isBlack() and sgs.card_lack[friend:objectName()]["Jink"] == 0 then return true end
	--[[if friend:hasSkill("huanshi") and self:touhouCanHuanshi(card,source,friend)>0 then
		if sgs.card_lack[friend:objectName()]["Jink"] == 0 then
			return true
		end
	end]]
	--local lingxian = self.room:findPlayerBySkillName("huanshi")
	--ÒªÇó×Ô¼ºÒÑ¾­ÌøÉí·Ý£¬ÇÒµÐÈËÊýÁ¿²»Îª0
	--ÌØ¶¨¶ÔÏóÓÅÏÈËÀµû ²»ÐÐÃ´¡£¡£¡£
	if self:sidieEffect(source) then
		local sidies = self:touhouSidieTarget(card,source)
		if #sidies ~= 0 and not table.contains(sidies, friend:objectName()) then return false end
		if sgs.ai_role[source:objectName()]  ~= "netural" then
			local sidieTargets=sgs.SPlayerList()
			for _,p in sgs.qlist(self.room:getOtherPlayers(friend)) do
				if friend:canSlash(p,nil,false) then
					sidieTargets:append(p)
				end
			end
			local sidieTarget =getSidieVictim(self, sidieTargets,friend)
			if sidieTarget then return true end
		end
	end
	
	if friend:hasArmorEffect("Vine") and source:hasSkill("shuzui") and card:isKindOf("NormalSlash") and self:isWeak(friend) then
		return true
	end


	if not self:hasHeavySlashDamage(source, card, friend) and card:getSkillName() ~= "lihuo"
			and (self:findLeijiTarget(friend, 50, source)
				or (friend:isLord() and source:hasSkill("guagu") and friend:getLostHp() >= 1 and getCardsNum("Jink", friend, source) == 0)
				or (friend:hasSkill("jieming") and source:hasSkill("nosrende") and (huatuo and self:isFriend(huatuo, source)))
				) then
		return true
	end
	if card:isKindOf("NatureSlash") and friend:isChained() and self:isGoodChainTarget(friend, source, nil, nil, card) then return true end
	if not self:hasHeavySlashDamage(source, card, friend) and friend:getHp() == 1 and self:canMaoyouSelf(friend) then
		return true
	end
	return
end

--¶«·½É±×Ô´´:Ä¿Ç°Ö»¿¼ÂÇ¶àÓàµÄÉ±É±¶ÓÓÑ,²»Í¬ÓÚÓÅÏÈÉ±£¬ÊôÓÚË³´ø
function SmartAI:isTargetForRedundantSlash(card, target,source,targets)
	source = source or self.player
	if not self:isFriend(source,target) then
		return false
	end

	targets = targets or sgs.SPlayerList()
	for _,p in sgs.qlist(targets) do
		if p:hasSkill("here")  then
			card= self:copyHereSlash(card)
			break
		end
	end
	if source:hasSkill("here")  then
		card= self:copyHereSlash(card)
	end
	for _, askill in sgs.qlist(target:getVisibleSkillList()) do
		local s_name = askill:objectName()
		if not target:hasSkill(s_name) then continue end
		local filter = sgs.ai_benefitBySlashed[s_name]
			if filter and type(filter) == "function"
			and filter(self, card,source,target) then
			return true
		end
	end
	return false
end

--²Á Ö»»á¶Ô×Åµ±Ç°·ÀÓùµÍµÄÏÂÊÖ£¬£¬£¬ÖÒÆ¾Ê²Ã´°ïÖ÷µ²µ¶¡£¡£
function SmartAI:useCardSlash(card, use)
	if not use.isDummy and not self:slashIsAvailable(self.player, card) then return end
	--if card:isKindOf("Slash") and self.player:hasSkill("here") then
	--  card= self:copyHereSlash(card)
	--end

	local basicnum = 0
	local blacknum = 0
	local cards = self.player:getCards("hes")
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if acard:getTypeId() == sgs.Card_TypeBasic and not acard:isKindOf("Peach") then basicnum = basicnum + 1 end
	end
	local blacknum =self:getSuitNum("black", true)
	local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, card) > 50
						or self.player:hasFlag("slashNoDistanceLimit")
						or card:getSkillName() == "qiaoshui"
	self.slash_targets = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card)
	if self.player:hasSkill("duanbing") then self.slash_targets = self.slash_targets + 1 end

	local rangefix = 0
	if card:isVirtualCard() then
		if self.player:getWeapon() and card:getSubcards():contains(self.player:getWeapon():getEffectiveId()) then
			if self.player:getWeapon():getClassName() ~= "Weapon" then
				rangefix = sgs.weapon_range[self.player:getWeapon():getClassName()] - self.player:getAttackRange(false)
			end
		end
		if self.player:getOffensiveHorse() and card:getSubcards():contains(self.player:getOffensiveHorse():getEffectiveId()) then
			rangefix = rangefix + 1
		end
	end

	local function canAppendTarget(target)
		if use.to:contains(target) then return false end
		local targets = sgs.PlayerList()
		for _, to in sgs.qlist(use.to) do
			targets:append(to)
		end
		local can =  card:targetFilter(targets, target, self.player)
		if can and self.player:hasSkill("shuangren") and use.to:isEmpty() then
			rangefix = -100
		end
		--self.player:gainMark("@aa", rangefix)
		return can
	end

	--for shikong
	--¼òµ¥´Ö±©
	if  self.player:hasSkill("shikong") and self.player:getPhase()==sgs.Player_Play
	and not self.player:hasFlag("slashTargetFixToOne") then
		--[[for _,p in sgs.qlist(self.player:getAliveSiblings()) {
			if (card:IsSpecificAssignee(p, self.player, this)) {
				return !targets.isEmpty();
			}
		}]]
		local shikong_f = 0
		local shikong_e = 0
		local shikongTargets = sgs.SPlayerList()
		for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if (self.player:canSlash(p, card, not no_distance, rangefix)
						or (use.isDummy and (self.player:distanceTo(p, rangefix) <= self.predictedRange))) then
				--if  use.to and self.player:inMyAttackRange(p) and canAppendTarget(p)
					shikongTargets:append(p)
					if self:isFriend(p) then
						shikong_f=shikong_f+1
					else
						shikong_e=shikong_e+1
					end
				--end
			end
		end
		if shikong_f > shikong_e then return end

		for _,p in sgs.qlist(shikongTargets) do
			--shikongTargets are checked
			use.card = card
			if use.to then
				use.to:append(p)
			end
		end
		--²»¿¼ÂÇË«½«¿ÉÒÔÌí¼ÓÆäËûÄ¿±ê£¬Ö±½Óreturn
		return
	end



	if not use.isDummy and self.player:hasSkill("qingnang") and self:isWeak() and self:getOverflow() == 0 then return end
	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card, friend)
		if self:isPriorFriendOfSlash(friend, card) then
			if not slash_prohibit then
				if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
					and (self.player:canSlash(friend, card, not no_distance, rangefix)
						or (use.isDummy and (self.player:distanceTo(friend, rangefix) <= self.predictedRange)))
					and self:slashIsEffective(card, friend) then
					use.card = card
					if use.to and canAppendTarget(friend) then
						use.to:append(friend)
					end
					if not use.to or self.slash_targets <= use.to:length() then return end
				end
			end
		end
	end




	local targets = {}
	local forbidden = {}
	self:sort(self.enemies, "defenseSlash")
	for _, enemy in ipairs(self.enemies) do
		if not self:slashProhibit(card, enemy) and sgs.isGoodTarget(enemy, self.enemies, self, true) then
			--if self:slashEightDiagram(enemy) then
				if not self:getDamagedEffects(enemy, self.player, true) and not self:touhouCardAttackWaste(card,self.player,enemy) then
					table.insert(targets, enemy)
				else
					table.insert(forbidden, enemy)
				end
			--end
		end
	end
	if #targets == 0 and #forbidden > 0 then targets = forbidden end

	if #targets == 1 and card:getSkillName() == "lihuo" and not targets[1]:hasArmorEffect("Vine") then return end
	--[[if self.player:hasSkill("guaili")  then
		for _,c in pairs (self:getCards("Slash")) do
			if c:isRed() and not card:isRed() then
				use.card = c
				break
			end
		end
	end]]


	for _, target in ipairs(targets) do
		local canliuli = false
		for _, friend in ipairs(self.friends_noself) do
			if self:canLiuli(target, friend) and self:slashIsEffective(card, friend) and #targets > 1 and friend:getHp() < 3 then canliuli = true end
		end
		if (not use.current_targets or not table.contains(use.current_targets, target:objectName()))
			and (self.player:canSlash(target, card, not no_distance, rangefix)
				or (use.isDummy and self.predictedRange and self.player:distanceTo(target, rangefix) <= self.predictedRange))
			and self:objectiveLevel(target) > 3
			and self:slashIsEffective(card, target)
			and not (target:hasSkill("xiangle") and basicnum < 2) and not canliuli
			and not (target:hasSkill("junwei") and target:getKingdom()~=self.player:getKingdom() and blacknum < 2)
			and not (not self:isWeak(target) and #self.enemies > 1 and #self.friends > 1 and self.player:hasSkill("keji")
			and self:getOverflow() > 0 and not self:hasCrossbowEffect()) then
			-- fill the card use struct
			local usecard = card
			if not use.to or use.to:isEmpty() then
				if self.player:hasWeapon("Spear") and card:getSkillName() == "Spear" then
				elseif self.player:hasWeapon("Crossbow") and self:getCardsNum("Slash") > 1 then
				elseif not use.isDummy then
					local Weapons = {}
					for _, acard in sgs.qlist(self.player:getHandcards()) do
						if acard:isKindOf("Weapon") then
							local callback = sgs.ai_slash_weaponfilter[acard:objectName()]
							if callback and type(callback) == "function" and callback(self, target, self.player) -- @todo: param of weapon_filter
								and self.player:distanceTo(target) <= (sgs.weapon_range[acard:getClassName()] or 1) then
								self:useEquipCard(acard, use)
								if use.card then table.insert(Weapons, acard) end
							end
						end
					end
					if #Weapons > 0 then
						local cmp = function(a, b)
							return self:evaluateWeapon(a) > self:evaluateWeapon(b)
						end
						table.sort(Weapons, cmp)
						use.card = Weapons[1]
						return
					end
				end
				if target:isChained() and self:isGoodChainTarget(target, nil, nil, nil, card) and not use.card then
					local here=false
					if self.player:hasSkill("here") or target:hasSkill("here") then
						here=true
					end
					--Á¬åóµÄ»° ÏÈÓÃÆÕÉ±£¿
					if here then
						--ÓÐusecard ²»»áÊ¹·µ»Øuse.cardÎª ¿Õ£¿
					else
						if  card:isKindOf("NatureSlash")   then
							if self:hasCrossbowEffect() and sgs.card_lack[target:objectName()]["Jink"] == 0 then
								local slashes = self:getCards("Slash")
									for _, slash in ipairs(slashes) do
									if not slash:isKindOf("NatureSlash") and self:slashIsEffective(slash, target)
										and not self:slashProhibit(slash, target) then
										usecard = slash
										break
									end
								end
							end
						elseif  not card:isKindOf("NatureSlash")  then
							if not self:hasCrossbowEffect() or  sgs.card_lack[target:objectName()]["Jink"] > 0 then
								local slash = self:getCard("NatureSlash")
								if slash and self:slashIsEffective(slash, target) and not self:slashProhibit(slash, target) then usecard = slash end
							end
						end
					end
				end
				local godsalvation = self:getCard("GodSalvation")
				if not use.isDummy and godsalvation and godsalvation:getId() ~= card:getId() and self:willUseGodSalvation(godsalvation) and
					(not target:isWounded() or not self:hasTrickEffective(godsalvation, target, self.player)) then
					use.card = godsalvation
					return
				end
			end


			use.card = use.card or usecard
			if use.to and not use.to:contains(target) and canAppendTarget(target) then
				use.to:append(target)
			end
			if not use.isDummy then
				--almostly for Skill "bllmwuyu"
				for _, id in sgs.qlist(use.card:getSubcards()) do
					sgs.Sanguosha:getCard(id):setFlags("AIGlobal_SearchForAnaleptic")
				end


				local analeptic = self:searchForAnaleptic(use, target, use.card)
				if analeptic and self:shouldUseAnaleptic(target, use.card) and analeptic:getEffectiveId() ~= card:getEffectiveId() then
					use.card = analeptic
					if use.to then use.to = sgs.SPlayerList() end
					return
				end
				for _, id in sgs.qlist(use.card:getSubcards()) do
					self.room:setCardFlag(sgs.Sanguosha:getCard(id), "-AIGlobal_SearchForAnaleptic")
				end


				if self.player:hasSkill("jilve") and self.player:getMark("@bear") > 0 and not self.player:hasFlag("JilveWansha") and target:getHp() == 1 and not self.room:getCurrent():hasSkill("wansha")
					and (target:isKongcheng() or getCardsNum("Jink", target, self.player) < 1 or sgs.card_lack[target:objectName()]["Jink"] == 1) then
					use.card = sgs.Card_Parse("@JilveCard=.")
					sgs.ai_skill_choice.jilve = "wansha"
					if use.to then use.to = sgs.SPlayerList() end
					return
				end
			end
			if not use.to or self.slash_targets <= use.to:length() then return end
		end
	end




	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = self:slashProhibit(card, friend)
		if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
			and not self:hasHeavySlashDamage(self.player, card, friend) and card:getSkillName() ~= "lihuo"
			and (not use.to or not use.to:contains(friend))
			and (self.player:hasSkill("pojun") and friend:getHp() > 4 and getCardsNum("Jink", friend, self.player) == 0 and friend:getHandcardNum() < 3)
			or (self:getDamagedEffects(friend, self.player) and not (friend:isLord() and #self.enemies < 1))
			or (self:needToLoseHp(friend, self.player, true, true) and not (friend:isLord() and #self.enemies < 1)) then

			if not slash_prohibit then
				if ((self.player:canSlash(friend, card, not no_distance, rangefix))
					or (use.isDummy and self.predictedRange and self.player:distanceTo(friend, rangefix) <= self.predictedRange))
					and self:slashIsEffective(card, friend) then
					use.card = card
					if use.to and canAppendTarget(friend) then
						use.to:append(friend)
					end
					if not use.to or self.slash_targets <= use.to:length() then return end
				end
			end
		end
	end
	--for RedundantSlash
	-- do not consider slash is effective
	for _, friend in ipairs(self.friends_noself) do
		if (not use.to or not use.to:contains(friend)) then
			if (self.player:canSlash(friend, card, not no_distance, rangefix)
			or (use.isDummy and (self.player:distanceTo(friend, rangefix) <= self.predictedRange)))
			and self:isTargetForRedundantSlash(card, friend,self.player,use.to) then
				use.card = card
				if use.to and canAppendTarget(friend) then
					use.to:append(friend)
				end
				if not use.to or self.slash_targets <= use.to:length() then return end
			end
		end
	end
end

function SmartAI:hasSlashTarget(card)
	if not self:slashIsAvailable(self.player, card) then return false end
	--if card:isKindOf("Slash") and self.player:hasSkill("here") then
	--  card= self:copyHereSlash(card)
	--end

	local basicnum = 0
	local blacknum = 0
	local cards = self.player:getCards("hes")
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if acard:getTypeId() == sgs.Card_TypeBasic and not acard:isKindOf("Peach") then basicnum = basicnum + 1 end
	end
	local blacknum =self:getSuitNum("black", true)
	local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, card) > 50
						or self.player:hasFlag("slashNoDistanceLimit")
						or card:getSkillName() == "qiaoshui"
	self.slash_targets = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card)
	if self.player:hasSkill("duanbing") then self.slash_targets = self.slash_targets + 1 end

	local rangefix = 0
	if card:isVirtualCard() then
		if self.player:getWeapon() and card:getSubcards():contains(self.player:getWeapon():getEffectiveId()) then
			if self.player:getWeapon():getClassName() ~= "Weapon" then
				rangefix = sgs.weapon_range[self.player:getWeapon():getClassName()] - self.player:getAttackRange(false)
			end
		end
		if self.player:getOffensiveHorse() and card:getSubcards():contains(self.player:getOffensiveHorse():getEffectiveId()) then
			rangefix = rangefix + 1
		end
	end

	--for shikong
	--¼òµ¥´Ö±©
	if  self.player:hasSkill("shikong") and self.player:getPhase()==sgs.Player_Play
	and not self.player:hasFlag("slashTargetFixToOne") then
		--[[for _,p in sgs.qlist(self.player:getAliveSiblings()) {
			if (card:IsSpecificAssignee(p, self.player, this)) {
				return !targets.isEmpty();
			}
		}]]
		local shikong_f = 0
		local shikong_e = 0
		local shikongTargets = sgs.SPlayerList()
		for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if (self.player:canSlash(p, card, not no_distance, rangefix)
						or ((self.player:distanceTo(p, rangefix) <= self.predictedRange))) then
				--if  use.to and self.player:inMyAttackRange(p) and canAppendTarget(p)
					shikongTargets:append(p)
					if self:isFriend(p) then
						shikong_f=shikong_f+1
					else
						shikong_e=shikong_e+1
					end
				--end
			end
		end
		if shikong_f > shikong_e then return false end
		
		--²»¿¼ÂÇË«½«¿ÉÒÔÌí¼ÓÆäËûÄ¿±ê£¬Ö±½Óreturn
		return true
	end



	if self.player:hasSkill("qingnang") and self:isWeak() and self:getOverflow() == 0 then return false end
	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card, friend)
		if self:isPriorFriendOfSlash(friend, card) then
			if not slash_prohibit then
				if (self.player:canSlash(friend, card, not no_distance, rangefix)
						or ((self.player:distanceTo(friend, rangefix) <= self.predictedRange)))
					and self:slashIsEffective(card, friend) then
					return true
				end
			end
		end
	end




	local targets = {}
	local forbidden = {}
	self:sort(self.enemies, "defenseSlash")
	for _, enemy in ipairs(self.enemies) do
		if not self:slashProhibit(card, enemy) and sgs.isGoodTarget(enemy, self.enemies, self, true) then
			--if self:slashEightDiagram(enemy) then
				if not self:getDamagedEffects(enemy, self.player, true) and not self:touhouCardAttackWaste(card,self.player,enemy) then
					table.insert(targets, enemy)
				else
					table.insert(forbidden, enemy)
				end
			--end
		end
	end
	if #targets == 0 and #forbidden > 0 then targets = forbidden end

	if #targets == 1 and card:getSkillName() == "lihuo" and not targets[1]:hasArmorEffect("Vine") then return false end
	--[[if self.player:hasSkill("guaili")  then
		for _,c in pairs (self:getCards("Slash")) do
			if c:isRed() and not card:isRed() then
				use.card = c
				break
			end
		end
	end]]


	for _, target in ipairs(targets) do
		local canliuli = false
		for _, friend in ipairs(self.friends_noself) do
			if self:canLiuli(target, friend) and self:slashIsEffective(card, friend) and #targets > 1 and friend:getHp() < 3 then canliuli = true end
		end
		if (self.player:canSlash(target, card, not no_distance, rangefix)
				or (self.predictedRange and self.player:distanceTo(target, rangefix) <= self.predictedRange))
			and self:objectiveLevel(target) > 3
			and self:slashIsEffective(card, target)
			and not (target:hasSkill("xiangle") and basicnum < 2) and not canliuli
			and not (target:hasSkill("junwei") and target:getKingdom()~=self.player:getKingdom() and blacknum < 2)
			and not (not self:isWeak(target) and #self.enemies > 1 and #self.friends > 1 and self.player:hasSkill("keji")
			and self:getOverflow() > 0 and not self:hasCrossbowEffect()) then
			-- fill the card use struct
			
			-- if not use.to or use.to:isEmpty() then
				if self.player:hasWeapon("Spear") and card:getSkillName() == "Spear" then
				elseif self.player:hasWeapon("Crossbow") and self:getCardsNum("Slash") > 1 then
				else
					local Weapons = {}
					for _, acard in sgs.qlist(self.player:getHandcards()) do
						if acard:isKindOf("Weapon") then
							local callback = sgs.ai_slash_weaponfilter[acard:objectName()]
							if callback and type(callback) == "function" and callback(self, target, self.player) -- @todo: param of weapon_filter
								and self.player:distanceTo(target) <= (sgs.weapon_range[acard:getClassName()] or 1) then
								return false
							end
						end
					end
					if #Weapons > 0 then
						local cmp = function(a, b)
							return self:evaluateWeapon(a) > self:evaluateWeapon(b)
						end
						table.sort(Weapons, cmp)
						return false
					end
				end
				if target:isChained() and self:isGoodChainTarget(target, nil, nil, nil, card) then
					local here=false
					if self.player:hasSkill("here") or target:hasSkill("here") then
						here=true
					end
					--Á¬åóµÄ»° ÏÈÓÃÆÕÉ±£¿
					if here then
						--ÓÐusecard ²»»áÊ¹·µ»Øuse.cardÎª ¿Õ£¿
					else
						if  card:isKindOf("NatureSlash")   then
							if self:hasCrossbowEffect() and sgs.card_lack[target:objectName()]["Jink"] == 0 then
								local slashes = self:getCards("Slash")
									for _, slash in ipairs(slashes) do
									if not slash:isKindOf("NatureSlash") and self:slashIsEffective(slash, target)
										and not self:slashProhibit(slash, target) then
										usecard = slash
										break
									end
								end
							end
						elseif  not card:isKindOf("NatureSlash")  then
							if not self:hasCrossbowEffect() or  sgs.card_lack[target:objectName()]["Jink"] > 0 then
								local slash = self:getCard("NatureSlash")
								if slash and self:slashIsEffective(slash, target) and not self:slashProhibit(slash, target) then usecard = slash end
							end
						end
					end
				end
				local godsalvation = self:getCard("GodSalvation")
				if godsalvation and godsalvation:getId() ~= card:getId() and self:willUseGodSalvation(godsalvation) and
					(not target:isWounded() or not self:hasTrickEffective(godsalvation, target, self.player)) then
					return true
				end
			-- end


			
			-- if not use.isDummy then
				--almostly for Skill "bllmwuyu"
				
				if self.player:hasSkill("jilve") and self.player:getMark("@bear") > 0 and not self.player:hasFlag("JilveWansha") and target:getHp() == 1 and not self.room:getCurrent():hasSkill("wansha")
					and (target:isKongcheng() or getCardsNum("Jink", target, self.player) < 1 or sgs.card_lack[target:objectName()]["Jink"] == 1) then
					return true
				end
			-- end
			return true
		end
	end




	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = self:slashProhibit(card, friend)
		if not self:hasHeavySlashDamage(self.player, card, friend) and card:getSkillName() ~= "lihuo"
			and (self.player:hasSkill("pojun") and friend:getHp() > 4 and getCardsNum("Jink", friend, self.player) == 0 and friend:getHandcardNum() < 3)
			or (self:getDamagedEffects(friend, self.player) and not (friend:isLord() and #self.enemies < 1))
			or (self:needToLoseHp(friend, self.player, true, true) and not (friend:isLord() and #self.enemies < 1)) then

			if not slash_prohibit then
				if ((self.player:canSlash(friend, card, not no_distance, rangefix)))
					and self:slashIsEffective(card, friend) then
					return true
				end
			end
		end
	end
	--for RedundantSlash
	-- do not consider slash is effective
	for _, friend in ipairs(self.friends_noself) do
		-- if (not use.to or not use.to:contains(friend)) then
			if (self.player:canSlash(friend, card, not no_distance, rangefix)
			or ((self.player:distanceTo(friend, rangefix) <= self.predictedRange))) then
				return true
			end
		-- end
	end
end

function countRangeFix(slash, from)
	local range_fix = 0
	if (slash:isVirtualCard()) then
		if (from:getWeapon() and slash:getSubcards():contains(from:getWeapon():getId())) then
				range_fix = range_fix +  sgs.weapon_range[from:getWeapon():getClassName()] - from:getAttackRange(false)
		end
		if (from:getOffensiveHorse() and slash:getSubcards():contains(from:getOffensiveHorse():getId())) then
				range_fix = range_fix + 1
		end
	end
	return range_fix
end

--±»¶¯ÓÃ£¿
sgs.ai_skill_use.slash = function(self, prompt)
	local parsedPrompt = prompt:split(":")
	local callback = sgs.ai_skill_cardask[parsedPrompt[1]] -- for askForUseSlashTo

	if self.player:hasFlag("slashTargetFixToOne") and type(callback) == "function" then
		local slash
		local target
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:hasFlag("SlashAssignee") then target = player break end
		end
		local target2 = nil
		if #parsedPrompt >= 3 then target2 = findPlayerByObjectName(self.room, parsedPrompt[3]) end
		if not target then return "." end
		local ret = callback(self, nil, nil, target, target2, prompt)
		if ret == nil or ret == "." then return "." end
		slash = sgs.Card_Parse(ret)
		local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50 or self.player:hasFlag("slashNoDistanceLimit")
		local targets = {}
		local use = { to = sgs.SPlayerList() }

		local range_fix = countRangeFix(slash, self.player)

		if self.player:canSlash(target, slash, not no_distance, range_fix) then use.to:append(target) else return "." end

		if parsedPrompt[1] ~= "@niluan-slash" and target:hasSkill("xiansi") and target:getPile("counter"):length() > 1
			and not (self:needKongcheng() and self.player:isLastHandCard(slash, true)) then
			return "@XiansiSlashCard=.->" .. target:objectName()
		end

		self:useCardSlash(slash, use)
		for _, p in sgs.qlist(use.to) do table.insert(targets, p:objectName()) end
		if table.contains(targets, target:objectName()) then return ret .. "->" .. table.concat(targets, "+") end
		return "."
	end
	local useslash, target
	local slashes = self:getCards("Slash")
	self:sort(self.enemies, "defenseSlash")
	for _, slash in ipairs(slashes) do
		--ë·Áî³öÉ±
		local range_fix = countRangeFix(slash, self.player)
		local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50 or self.player:hasFlag("slashNoDistanceLimit")
		for _, friend in ipairs(self.friends_noself) do
			local slash_prohibit = false
			slash_prohibit = self:slashProhibit(slash, friend)
			if not self:hasHeavySlashDamage(self.player, slash, friend)
				and self.player:canSlash(friend, slash, not no_distance, range_fix) and not self:slashProhibit(slash, friend)
				and self:slashIsEffective(slash, friend)
				and (self:findLeijiTarget(friend, 50, self.player)
					or (friend:isLord() and self.player:hasSkill("guagu") and friend:getLostHp() >= 1 and getCardsNum("Jink", friend, self.player) == 0)
					or (friend:hasSkill("jieming") and self.player:hasSkill("nosrende") and (huatuo and self:isFriend(huatuo)))
					or (slash:isKindOf("NormalSlash") and self.player:hasSkill("shuzui") and friend:hasArmorEffect("Vine")))
				and not (self.player:hasFlag("slashTargetFix") and not friend:hasFlag("SlashAssignee"))
				and not (slash:isKindOf("XiansiSlashCard") and friend:getPile("counter"):length() < 2) then

				useslash = slash
				target = friend
				break
			end
		end
	end
	if not useslash  then
		for _, slash in ipairs(slashes) do
			local range_fix = countRangeFix(slash, self.player)
			local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50 or self.player:hasFlag("slashNoDistanceLimit")
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, slash, not no_distance, range_fix) and not self:slashProhibit(slash, enemy)
					and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)
					and not (self.player:hasFlag("slashTargetFix") and not enemy:hasFlag("SlashAssignee")) then

					useslash = slash
					target = enemy
					break
				end
			end
		end
	end
	if useslash and target then
		local targets = {}
		local use = { to = sgs.SPlayerList() }
		use.to:append(target)

		if target:hasSkill("xiansi") and target:getPile("counter"):length() > 1 and not (self:needKongcheng() and self.player:isLastHandCard(slash, true)) then
			return "@XiansiSlashCard=.->" .. target:objectName()
		end

		self:useCardSlash(useslash, use)
		for _, p in sgs.qlist(use.to) do table.insert(targets, p:objectName()) end
		if table.contains(targets, target:objectName()) then return useslash:toString() .. "->" .. table.concat(targets, "+") end
	end
	return "."
end

sgs.ai_skill_playerchosen.slash_extra_targets = function(self, targets)
	local slash = sgs.cloneCard("slash")
	targets = sgs.QList2Table(targets)
	self:sort(targets, "defenseSlash")
	for _, target in ipairs(targets) do
		if self:isEnemy(target) and not self:slashProhibit(slash, target) and sgs.isGoodTarget(target, targetlist, self) and self:slashIsEffective(slash, target) then
			return target
		end
	end
	return nil
end

sgs.ai_skill_playerchosen.zero_card_as_slash = function(self, targets)
	local slash = sgs.cloneCard("slash")
	local targetlist = sgs.QList2Table(targets)
	local arrBestHp, canAvoidSlash, forbidden = {}, {}, {}
	self:sort(targetlist, "defenseSlash")

	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) and not self:slashProhibit(slash ,target) and sgs.isGoodTarget(target, targetlist, self) then
			if self:slashIsEffective(slash, target) then
				if self:getDamagedEffects(target, self.player, true) or self:needLeiji(target, self.player) then
					table.insert(forbidden, target)
				elseif self:needToLoseHp(target, self.player, true, true) then
					table.insert(arrBestHp, target)
				else
					return target
				end
			else
				table.insert(canAvoidSlash, target)
			end
		end
	end
	for i=#targetlist, 1, -1 do
		local target = targetlist[i]
		if not self:slashProhibit(slash, target) then
			if self:slashIsEffective(slash, target) then
				if self:isFriend(target) and (self:needToLoseHp(target, self.player, true, true)
					or self:getDamagedEffects(target, self.player, true) or self:needLeiji(target, self.player)) then
						return target
				end
			else
				table.insert(canAvoidSlash, target)
			end
		end
	end

	if #canAvoidSlash > 0 then return canAvoidSlash[1] end
	if #arrBestHp > 0 then return arrBestHp[1] end

	self:sort(targetlist, "defenseSlash")
	targetlist = sgs.reverse(targetlist)
	for _, target in ipairs(targetlist) do
		if target:objectName() ~= self.player:objectName() and not self:isFriend(target) and not table.contains(forbidden, target) then
			return target
		end
	end

	return targetlist[1]
end

--¶«·½É±Ïà¹Ø
--¡¾¶³½á¡¿¡¾ËÀµû¡¿¡¾ÉñÊÚ¡¿¡¾°µÓò¡¿¡¾»ÃÊÓ¡¿¡¾ÀíÖÇ¡¿
--ËÀµûÉñÊÚ¶ÔÓÚÔ­Ê¹ÓÃÕßµÄ³ðºÞ¸üÐÂ  ÔÝÊ±Ã»ÏëºÃ
sgs.ai_card_intention.Slash = function(self, card, from, tos)
	if sgs.ai_liuli_effect then sgs.ai_liuli_effect = false return end
	--if sgs.ai_huanshi_effect then sgs.ai_huanshi_effect = false return end
	--if sgs.ai_bihuo_effect then sgs.ai_bihuo_effect = false return end
	if sgs.ai_collateral then sgs.ai_collateral = false return end
	if card:hasFlag("nosjiefan-slash") then return end
	if card:getSkillName() =="shenshou" then return end
	if card:getSkillName() =="sidie" then return end
	if card:getSkillName() =="xiefa" then return end

	local kosuzu = self.room:findPlayerBySkillName("bihuo")

	--Ê§¿ØÉ±³ðºÞ²»²ÎÓë±ðµÄ¼¼ÄÜÁª¶¯¼ÆËã¡£¡£¡£
	if from:hasSkill("shikong") and from:getPhase() == sgs.Player_Play then
		for _, to in ipairs(tos) do
			if not self:isFriend(from,to) then
				sgs.updateIntention(from, to, 60)
			end
		end
		return
	end
	for _, to in ipairs(tos) do
		local value = 80
		speakTrigger(card, from, to)
		for _, askill in sgs.qlist(to:getVisibleSkillList()) do
			local s_name = askill:objectName()
			if not to:hasSkill(s_name) then continue end
			local filter = sgs.ai_benefitBySlashed[s_name]
			if filter and type(filter) == "function"
				and filter(self, card,from,to) then
				value = 0
				continue
			end
		end
		if to:hasSkill("xunshi") and #tos > 1 then value = 0 end
		if  from:hasSkill("dongjie") and not from:hasFlag("dongjie") and not to:faceUp() then value= 0 end --self:isFriend(from,to) and
		if self:sidieEffect(from)  then value = 0 end
		if from:hasSkill("lizhi") and self:isFriend(from,to) then value = 0 end
		--if to:hasSkill("huanshi") and self:touhouCanHuanshi(card,from,to)>0 then
		--  value = 0
		--end
		if kosuzu and to:hasFlag("bihuo_"..kosuzu:objectName()) then
			value=0
		end
		if to:hasSkill("anyu") and not to:faceUp() and card:isBlack() then value = 0 end
		if to:hasSkill("yiji") then value = 0 end
		if to:hasSkill("leiji") and (getCardsNum("Jink", to, from) > 0 or to:hasArmorEffect("EightDiagram")) and not self:hasHeavySlashDamage(from, card, to)
			and (hasExplicitRebel(self.room) or sgs.explicit_renegade) and not self:canLiegong(to, from) then value = 0 end
		if not self:hasHeavySlashDamage(from, card, to) and (self:getDamagedEffects(to, from, true) or self:needToLoseHp(to, from, true, true)) then value = 0 end
		if from:hasSkill("pojun") and to:getHp() > (2 + self:hasHeavySlashDamage(from, card, to, true)) then value = 0 end
		if self:needLeiji(to, from) then value = -10 end
		if self:canMaoyouSelf(to) and to:getHp() == 1 then
			value = -10
		end
		--slm 008
		if  not card:isKindOf("NatureSlash") and to:hasSkill("wushou")
		and to:getHp()<=3 and to:hasArmorEffect("Vine") then
			value = -30
		end
		sgs.updateIntention(from, to, value)
	end
end

--¶«·½É±Ïà¹Ø
--¡¾¶³½á¡¿¡¾ËÀµû¡¿¡¾ÍþÑ¹¡¿¡¾ÐéÓ°¡¿¡¾ÀíÖÇ¡¿
--¶³½áÉ±Ó¦¸Ã×ö³ÉÒ»¸öÅÐ¶Ïº¯Êý£¿£¿
--self:touhouDamage
sgs.ai_skill_cardask["slash-jink"] = function(self, data, pattern, target)
	local function getJink()--Ö÷ÒªÊÇÉÁÎÞÐ§
		--ÍþÑ¹ÎÞÐ§
		if self:hasWeiya() then
			if (self:getCardsNum("Jink") < 2 or self:getCardsNum("Jink", "hs", true) < 1)
			and not (self.player:getHandcardNum() == 1 and self:hasSkills(sgs.need_kongcheng)) then return "." end
		end
		--ÌìÈË³öÉÁµÄñîºÏ¡£¡£¡£

		--[[if target and target:hasSkill("dahe") and self.player:hasFlag("dahe") then
			for _, card in ipairs(self:getCards("Jink")) do
				if card:getSuit() == sgs.Card_Heart then return card:getId() end
			end
			return "."
		end]]
		--return self:getCardId("Jink") or "."
		return self:getCardId("Jink")
	end

	local slash
	if type(data) == "userdata" then
		local effect = data:toSlashEffect()
		slash = effect.slash
	else
		slash = sgs.cloneCard("slash")
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	if (not target or self:isFriend(target)) and slash:hasFlag("nosjiefan-slash") then return "." end
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end

	if not target then return getJink() end

	if not self:hasHeavySlashDamage(target, slash, self.player) and self:getDamagedEffects(self.player, target, slash) then return "." end
	if slash:isKindOf("NatureSlash") and self.player:isChained() and self:isGoodChainTarget(self.player, nil, nil, nil, slash) then return "." end

	---***µ÷ÓÃ¶«·½É±Ïà¹Øº¯Êý
	if self.player:hasSkill("xuying") then--ÐéÓ°±ØÐëÒªÉÁ
		return getJink()
	end
	
	--¼ì²éÊÇ·ñÐèÒªÉÁ±Ü
	local fakeDamage=sgs.DamageStruct()
	fakeDamage.card=slash
	fakeDamage.nature= self:touhouDamageNature(slash,target,player)
	fakeDamage.damage=1
	fakeDamage.from=target
	fakeDamage.to=self.player

	if not self:touhouNeedAvoidAttack(fakeDamage,target,self.player) then
		return "."
	end
	if fakeDamage.nature ~= sgs.DamageStruct_Normal  then
		x,y=self:touhouChainDamage(fakeDamage,target,self.player)
		--×Ô¼ºµÄËÀ»îÉ¶µÄÔÝÊ±²»¹ÜÁË
		if x-y>0 then
			return "."
		end
	end
	
	--***ÒÔÉÏÎª¶«·½É±Ïà¹Ø
	
	--¶«·½ÆôÁé¸ó¡¾¶Ý¼×¡¿
	if slash:hasFlag("DunjiaFlag") and ((self.player:getHandcardNum() <= 5 and not self:hasIndul()) or (self.player:hasSkill("fengshi")
			and self.player:getMark("fengshi") == 0)) and not self.player:getHp() == 1 then
		local nature = self:touhouDamageNature(slash, target, player)
		if nature == sgs.DamageStruct_Normal or not self.player:isChained() then
			return "."
		end
		local yes = true
		local x = 0
		local y = 0
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if p:isChained() then
				if self:isFriend(self.player, p) then
					y = y + 1
					if p:getHp() == 1 then
						yes = false
						break
					end
				else
					x = x + 1
				end
			end
		end
		if yes and x < y then return "." end
	end

	--»¹¸ÃÓÐÒ»¸öÂôÑª±£ÉÁ ±£É±µÄº¯Êý£¿£¿
	if self:isFriend(target) then
		if self:findLeijiTarget(self.player, 50, target) then return getJink() end
		if target:hasSkill("jieyin") and not self.player:isWounded() and self.player:isMale() and not self.player:hasSkills("leiji|nosleiji") then return "." end

		if (target:hasSkill("nosrende") or (target:hasSkill("rende") and not target:hasUsed("RendeCard"))) and self.player:hasSkill("jieming") then return "." end
		if target:hasSkill("pojun") and not self.player:faceUp() then return "." end
	else
		if self:hasHeavySlashDamage(target, slash) then return getJink() end

		local current = self.room:getCurrent()
		if current and current:hasSkill("juece") and self.player:getHp() > 0 then
			local use = false
			for _, card in ipairs(self:getCards("Jink")) do
				if not self.player:isLastHandCard(card, true) then
					use = true
					break
				end
			end
			if not use then return "." end
		end
		if self.player:getHandcardNum() == 1 and self:needKongcheng() then return getJink() end
		if not self:hasLoseHandcardEffective() and not self.player:isKongcheng() then return getJink() end
		if target:hasSkill("mengjin") then
			if self:doNotDiscard(self.player, "hes", true) then return getJink() end
			if self.player:getCards("hes"):length() == 1 and not self.player:getArmor() then return getJink() end
			if self.player:hasSkills("jijiu|qingnang") and self.player:getCards("hes"):length() > 1 then return "." end
			if self:canUseJieyuanDecrease(target) then return "." end
			if (self:getCardsNum("Peach") > 0 or (self:getCardsNum("Analeptic") > 0 and self:isWeak()))
				and not self.player:hasSkills("tuntian+zaoxian") and not self:willSkipPlayPhase() then
				return "."
			end
		end

		if target:hasWeapon("Axe") then
			if target:hasSkills(sgs.lose_equip_skill) and target:getEquips():length() > 1 and target:getCards("hes"):length() > 2 then return "." end
			if target:getHandcardNum() - target:getHp() > 2 and not self:isWeak() and not self:getOverflow() then return "." end
		--elseif target:hasWeapon("Blade") then
		--  if slash:isKindOf("NatureSlash") and self.player:hasArmorEffect("Vine")
		--      or self.player:hasArmorEffect("RenwangShield")
		--      or self:hasEightDiagramEffect()
		--      or self:hasHeavySlashDamage(target, slash)
		--      or (self.player:getHp() == 1 and #self.friends_noself == 0) then
		--  elseif (self:getCardsNum("Jink") <= getCardsNum("Slash", target, self.player) or self.player:hasSkill("qingnang")) and self.player:getHp() > 1
		--          or self.player:hasSkill("jijiu") and getKnownCard(self.player, self.player, "red") > 0
		--          or self:canUseJieyuanDecrease(target)
		--      then
		--      return "."
		--  end
		end

	end
	return getJink() --or "."
end
sgs.ai_choicemade_filter.cardResponded["slash-jink"] = function(self, player, args)
	--Ä¿Ç°Ö»ÓÐÉÁËÀµûÉ±ÓÐ³ðºÞ
	if args[#args] ~= "_nil_" then
		local uuz = findPlayerByObjectName(self.room, (args[2]:split(":"))[2])
		if uuz and uuz:isLord() and self:sidieEffect(uuz) then
			if not player:hasSkill("xuying") then
				sgs.updateIntention(player, uuz, 50)
			end
		end
	end
end
sgs.dynamic_value.damage_card.Slash = true

sgs.ai_use_value.Slash = 4.5
sgs.ai_keep_value.Slash = 3.6
sgs.ai_use_priority.Slash = 2.6

--canÃüÖÐ?
function SmartAI:canHit(to, from, conservative)
	from = from or self.room:getCurrent()
	to = to or self.player
	local jink = sgs.cloneCard("jink")
	if to:isCardLimited(jink, sgs.Card_MethodUse) then return true end
	if self:canLiegong(to, from) then return true end
	if not self:isFriend(to, from) then
		if from:hasWeapon("Axe") and from:getCards("hes"):length() > 2 then return true end
		--if from:hasWeapon("Blade") and getCardsNum("Jink", to, from) <= getCardsNum("Slash", from, from) then return true end
		if from:hasSkill("mengjin")
			and not self:hasHeavySlashDamage(from, nil, to) and not self:needLeiji(to, from) then
				if self:doNotDiscard(to, "hes", true) then
				elseif to:getCards("hes"):length() == 1 and not to:getArmor() then
				elseif self:canUseJieyuanDecrease(from, to) then return false
				elseif self:willSkipPlayPhase() then
				elseif (getCardsNum("Peach", to, from) > 0 or getCardsNum("Analeptic", to, from) > 0) then return true
				elseif not self:isWeak(to) and to:getArmor() and not self:needToThrowArmor() then return true
				elseif not self:isWeak(to) and to:getDefensiveHorse() then return true
				end
		end
	end

	local hasHeart, hasRed, hasBlack
	for _, card in ipairs(self:getCards("Jink"), to) do
		if card:getSuit() == sgs.Card_Heart then hasHeart = true end
		if card:isRed() then hasRed = true end
		if card:isBlack() then hasBlack = true end
	end

	if not conservative and self:hasHeavySlashDamage(from, nil, to) then conservative = true end
	if not conservative and from:hasSkill("moukui") then conservative = true end
	if not conservative and self:hasEightDiagramEffect(to) and not IgnoreArmor(from, to) then return false end
	local need_double_jink = from and (from:hasSkill("wushuang")
			or (from:hasSkill("roulin") and to:isFemale()) or (from:isFemale() and to:hasSkill("roulin")))
	if to:objectName() == self.player:objectName() then
		if getCardsNum("Jink", to, from) == 0 then return true end
		if need_double_jink and self:getCardsNum("Jink", to, from) < 2 then return true end
	end
	if getCardsNum("Jink", to, from) == 0 then return true end
	if need_double_jink and getCardsNum("Jink", to, from) < 2 then return true end
	return false
end

--¶«·½É±Ïà¹Ø
--Ó¦¸Ã½¨Ò»¸öº¯Êý¡¾¾ÈÊê¡¿
--¡¾¶·¾Æ¡¿
function SmartAI:useCardPeach(card, use)
	--¶ÔÖ÷¹«·¢¶¯Ñç»áÊ¹ÓÃÌÒ
	if (self.player:getKingdom() == "zhan") then
		local yanhuiTargets = {}
		for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if p:hasLordSkill("yanhui") and p:isAlive() and p:isWounded() and p:getHp()<=self.player:getHp() then
				if self:isFriend(p) then
					table.insert(yanhuiTargets,p)
				end
			end
		end
		if (#yanhuiTargets >= 1) then
			self:sort(yanhuiTargets,"hp")
			use.card = card
			if use.to then
				use.to:append(yanhuiTargets[1])
				if use.to:length() >= 1 then return end
			end
		end
	end


	local mustusepeach = false
	if not self.player:isWounded() then return end
	if self.player:hasSkill("jiushu") and self.player:getHp()>2 then
		if self:getCardsNum("Peach")<2 then
			return
		end
	end
	if self.player:hasSkill("bumie")  then
		if self.player:getHp()==1 and self.player:getHandcardNum()==1 then
			return
		end
	end
	if self:cautionDoujiu(self.player,card) then
		return
	end
	if self.player:hasSkill("yongsi") and self:getCardsNum("Peach") > self:getOverflow(nil, true) then
		use.card = card
		return
	end

	local peaches = 0
	local cards = self.player:getHandcards()
	local lord= getLord(self.player)

	cards = sgs.QList2Table(cards)
	for _,card in ipairs(cards) do
		if isCard("Peach", card, self.player) then peaches = peaches + 1 end
	end

	if (self.player:hasSkill("nosrende") or (self.player:hasSkill("rende") and not self.player:hasUsed("RendeCard"))) and self:findFriendsByType(sgs.Friend_Draw) then return end

	if self.player:hasArmorEffect("SilverLion") then
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:isKindOf("Armor") and self:evaluateArmor(card) > 0 then
				use.card = card
				return
			end
		end
	end

	local SilverLion, OtherArmor
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:isKindOf("SilverLion") then
			SilverLion = card
		elseif card:isKindOf("Armor") and not card:isKindOf("SilverLion") and self:evaluateArmor(card) > 0 then
			OtherArmor = true
		end
	end
	if SilverLion and OtherArmor then
		use.card = SilverLion
		return
	end

	for _, enemy in ipairs(self.enemies) do
		if self.player:getHandcardNum() < 3 and
				(self:hasSkills(sgs.drawpeach_skill,enemy) or getCardsNum("Dismantlement", enemy) >= 1
					or enemy:hasSkill("jixi") and enemy:getPile("field"):length() >0 and enemy:distanceTo(self.player) == 1
					or enemy:hasSkill("qixi") and getKnownCard(enemy, self.player, "black", nil, "hes") >= 1
					or getCardsNum("Snatch", enemy) >= 1 and enemy:distanceTo(self.player) == 1
					or (enemy:hasSkill("tiaoxin") and self.player:inMyAttackRange(enemy) and self:getCardsNum("Slash") < 1 or not self.player:canSlash(enemy)))
				then
			mustusepeach = true
			break
		end
	end

	if self.player:getHp() == 1 and not (lord and self:isFriend(lord) and lord:getHp() < 2 and self:isWeak(lord)) then
		mustusepeach = true
	end

	if mustusepeach or (self.player:hasSkill("nosbuqu") and self.player:getHp() < 1 and self.player:getMaxCards() == 0) or peaches > self.player:getHp() then
		use.card = card
		return
	end



	if self:getOverflow() <= 0 and #self.friends_noself > 0 then
		return
	end

	if self:needToLoseHp(self.player, nil, nil, nil, true) then return end

	if lord and self:isFriend(lord) and lord:getHp() <= 2 and self:isWeak(lord) then
		if self.player:isLord() then use.card = card end
		if self:getCardsNum("Peach") > 1 and self:getCardsNum("Peach") + self:getCardsNum("Jink") > self.player:getMaxCards() then use.card = card end
		return
	end

	self:sort(self.friends, "hp")
	if self.friends[1]:objectName()==self.player:objectName() or self.player:getHp()<2 then
		use.card = card
		return
	end
	--ÅÅ³ý²»ÓÃÁôÌÒµÄ¶ÔÏó
	local friends={}
	for i = 1, #self.friends, 1 do
		if not self.friends[i]:hasSkills("skltkexue|bumie|huanmeng") then
			table.insert(friends,self.friends[i])
		end
	end
	if #friends > 1 and ((not hasBuquEffect(friends[2]) and friends[2]:getHp() < 3 and self:getOverflow() < 2)
								or (not hasBuquEffect(friends[1]) and friends[1]:getHp() < 2 and peaches <= 1 and self:getOverflow() < 3)) then
		return
	end
	--Îª¶ÓÓÑÁôÌÒ×Ó
	--[[if #self.friends > 1 and ((not hasBuquEffect(self.friends[2]) and self.friends[2]:getHp() < 3 and self:getOverflow() < 2)
								or (not hasBuquEffect(self.friends[1]) and self.friends[1]:getHp() < 2 and peaches <= 1 and self:getOverflow() < 3)) then
		return
	end]]
	-- Ôç×ä³ÔÌÒµÄÇé¿ö
	if self:hasSkill("zaozu") then
		if self:getOverflow() > peaches then
			return
		end
	end
	if self.player:hasSkill("jieyin") and self:getOverflow() > 0 then
		self:sort(self.friends, "hp")
		for _, friend in ipairs(self.friends) do
			if friend:isWounded() and friend:isMale() then return end
		end
	end
	--³ÔÌÒÇ°ÏÈ¸ÊÂ¶»»×°
	if self.player:hasSkill("ganlu") and not self.player:hasUsed("GanluCard") then
		local dummy_use = {isDummy = true}
		self:useSkillCard(sgs.Card_Parse("@GanluCard=."),dummy_use)
		if dummy_use.card then return end
	end

	use.card = card
end

sgs.ai_card_intention.Peach = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if to:hasSkill("wuhun") then continue end
		if to:hasSkill("guizha") and not card:isVirtualCard() then
			continue
		end
		if to:hasSkill("xunshi") and #tos > 1 then continue end
		sgs.updateIntention(from, to, -120)
	end
end

sgs.ai_use_value.Peach = 6
sgs.ai_keep_value.Peach = 7
sgs.ai_use_priority.Peach = 0.9

sgs.ai_use_value.Jink = 8.9
sgs.ai_keep_value.Jink = 5.2

sgs.dynamic_value.benefit.Peach = true

sgs.ai_keep_value.Weapon = 2.08
sgs.ai_keep_value.Armor = 2.06
sgs.ai_keep_value.Horse = 2.04

sgs.weapon_range.Weapon = 1
sgs.weapon_range.Crossbow = 1
sgs.weapon_range.DoubleSword = 2
sgs.weapon_range.QinggangSword = 2
sgs.weapon_range.IceSword = 2
sgs.weapon_range.GudingBlade = 2
sgs.weapon_range.Axe = 3
sgs.weapon_range.Blade = 3
sgs.weapon_range.Spear = 3
sgs.weapon_range.Halberd = 4
sgs.weapon_range.KylinBow = 5

sgs.ai_skill_invoke.DoubleSword = function(self, data)
	local target = self.player:getTag("DoubleSwordTarget"):toPlayer()
	if target then
		return not self:needKongcheng(self.player, true) and  not self:needKongcheng(target, true)
		--and self:doNotDiscard(target, "hs")
		and not (self:getLeastHandcardNum(target) > 0 and self:hasLoseHandcardEffective(target))
	end
	return not self:needKongcheng(self.player, true)
end



function sgs.ai_slash_weaponfilter.DoubleSword(self, to, player)
	--return player:getGender()~=to:getGender()
	return self:touhouIsSameWithLordKingdom(player)~=self:touhouIsSameWithLordKingdom(to)
end

function sgs.ai_weapon_value.DoubleSword(self, enemy, player)
	--if enemy and  and enemy:isMale() ~= player:isMale()
	--           then return 4 end
	if enemy and  self:touhouIsSameWithLordKingdom(player)~=self:touhouIsSameWithLordKingdom(enemy)
				 then return 4 end
end

function SmartAI:getExpectedJinkNum(use)
	local jink_list = use.from:getTag("Jink_" .. use.card:toString()):toStringList()
	local index, jink_num = 1, 1
	for _, p in sgs.qlist(use.to) do
		if p:objectName() == self.player:objectName() then
			local n = tonumber(jink_list[index])
			if n == 0 then return 0
			elseif n > jink_num then jink_num = n end
		end
		index = index + 1
	end
	return jink_num
end

sgs.ai_skill_cardask["double-sword-card"] = function(self, data, pattern, target)
	if self.player:isKongcheng() then return "." end
	local use = data:toCardUse()
	local jink_num = self:getExpectedJinkNum(use)
	if jink_num > 1 and self:getCardsNum("Jink") == jink_num then return "." end

	if self:needKongcheng(self.player, true) and self.player:getHandcardNum() <= 2 then
		if self.player:getHandcardNum() == 1 then
			local card = self.player:getHandcards():first()
			return (jink_num > 0 and isCard("Jink", card, self.player)) and "." or ("$" .. card:getEffectiveId())
		end
		if self.player:getHandcardNum() == 2 then
			local first = self.player:getHandcards():first()
			local last = self.player:getHandcards():last()
			local jink = isCard("Jink", first, self.player) and first or (isCard("Jink", last, self.player) and last)
			if jink then
				return first:getEffectiveId() == jink:getEffectiveId() and ("$"..last:getEffectiveId()) or ("$"..first:getEffectiveId())
			end
		end
	end
	if target and self:isFriend(target) then return "." end
	if self:needBear() then return "." end
	if target and self:needKongcheng(target, true) then return "." end
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if (card:isKindOf("Slash") and self:getCardsNum("Slash") > 1)
			or (card:isKindOf("Jink") and self:getCardsNum("Jink") > 2)
			or card:isKindOf("Disaster")
			or (card:isKindOf("EquipCard") and not self:hasSkills(sgs.lose_equip_skill))
			or (not self.player:hasSkills("nosjizhi|jizhi") and (card:isKindOf("Collateral") or card:isKindOf("GodSalvation")
															or card:isKindOf("FireAttack") or card:isKindOf("IronChain") or card:isKindOf("AmazingGrace"))) then
			return "$" .. card:getEffectiveId()
		end
	end
	return "."
end


sgs.ai_choicemade_filter.cardResponded["double-sword-card"] = function(self, player, args)
	local user = findPlayerByObjectName(self.room, (args[2]:split(":"))[2])
	if user and args[#args] ~= "_nil_" then
		sgs.updateIntention(player, user, 80)
	end
end

function sgs.ai_weapon_value.QinggangSword(self, enemy)
	if enemy and enemy:getArmor() then return 3 end
end

sgs.ai_skill_invoke.IceSword = function(self, data)
	local damage = self.player:getTag("IceSword"):toDamage()
	local target = damage.to
	if self:isFriend(target) then
		if self:getDamagedEffects(target, self.players, true) or self:needToLoseHp(target, self.player, true) then return false
		elseif target:isChained() and self:isGoodChainTarget(target, self.player, nil, nil, damage.card) then return false
		elseif self:isWeak(target) or damage.damage > 1 then return true
		elseif target:getLostHp() < 1 then return false end
		return true
	else
		local canDamage = self:touhouNeedAvoidAttack(damage,self.player,target,true)
		if not canDamage then return true end
		if self:isWeak(target) then return false end
		if damage.damage > 1 or self:hasHeavySlashDamage(self.player, damage.card, target) then return false end
		if target:hasSkill("lirang") and #self:getFriendsNoself(target) > 0 then return false end
		if target:getArmor() and self:evaluateArmor(target:getArmor(), target) > 3 and not (target:hasArmorEffect("SilverLion") and target:isWounded()) then return true end
		local num = target:getHandcardNum()
		if self.player:hasSkill("tieji") or self:canLiegong(target, self.player) then return false end
		if target:hasSkills("tuntian+zaoxian") then return false end
		if self:hasSkills(sgs.need_kongcheng, target) then return false end
		if target:getCards("hes"):length()<4 and target:getCards("hes"):length()>1 then return true end
		return false
	end
end

function sgs.ai_slash_weaponfilter.GudingBlade(self, to)
	return to:isKongcheng()
end

function sgs.ai_weapon_value.GudingBlade(self, enemy)
	if not enemy then return end
	local value = 2
	if enemy:getHandcardNum() < 1 and not enemy:hasArmorEffect("SilverLion") then value = 4 end
	return value
end

function SmartAI:needToThrowAll(player)
	player = player or self.player
	if player:hasSkill("conghui") then return false end
	if not player:hasSkill("yongsi") then return false end
	if player:getPhase() == sgs.Player_NotActive or player:getPhase() == sgs.Player_Finish then return false end
	local zhanglu = self.room:findPlayerBySkillName("xiliang")
	if zhanglu and self:isFriend(zhanglu, player) then return false end
	local erzhang = self.room:findPlayerBySkillName("guzheng")
	if erzhang and not zhanglu and self:isFriend(erzhang, player) then return false end

	self.yongsi_discard = nil
	local index = 0

	local kingdom_num = 0
	local kingdoms = {}
	for _, ap in sgs.qlist(self.room:getAlivePlayers()) do
		if not kingdoms[ap:getKingdom()] then
			kingdoms[ap:getKingdom()] = true
			kingdom_num = kingdom_num + 1
		end
	end

	local cards = self.player:getCards("hes")
	local Discards = {}
	for _, card in sgs.qlist(cards) do
		local shouldDiscard = true
		if card:isKindOf("Axe") then shouldDiscard = false end
		if isCard("Peach", card, player) or isCard("Slash", card, player) then
			local dummy_use = { isDummy = true }
			self:useBasicCard(card, dummy_use)
			if dummy_use.card then shouldDiscard = false end
		end
		if card:getTypeId() == sgs.Card_TypeTrick then
			local dummy_use = { isDummy = true }
			self:useTrickCard(card, dummy_use)
			if dummy_use.card then shouldDiscard = false end
		end
		if shouldDiscard then
			if #Discards < 2 then table.insert(Discards, card:getId()) end
			index = index + 1
		end
	end

	if #Discards == 2 and index < kingdom_num then
		self.yongsi_discard = Discards
		return true
	end
	return false
end

sgs.ai_skill_cardask["@Axe"] = function(self, data, pattern, target)
	if target and self:isFriend(target) then return "." end
	local effect = data:toSlashEffect()
	local allcards = self.player:getCards("hes")
	allcards = sgs.QList2Table(allcards)
	--¿¼ÂÇÓÀºã£¿£¿
	if self:hasHeavySlashDamage(self.player, effect.slash, target)
	  or (#allcards - 3 >= self.player:getHp())
	  or (self.player:hasSkill("kuanggu") and self.player:isWounded() and self.player:distanceTo(effect.to) == 1)
	  or (effect.to:getHp() == 1 and not effect.to:hasSkill("buqu"))
	  or (self:needKongcheng() and self.player:getHandcardNum() > 0)
	  or (self:hasSkills(sgs.lose_equip_skill, self.player) and self.player:getEquips():length() > 1 and self.player:getHandcardNum() < 2)
	  or self:needToThrowAll() then
		local discard = self.yongsi_discard
		if discard then return "$"..table.concat(discard, "+") end

		local hcards = {}
		for _, c in sgs.qlist(self.player:getHandcards()) do
			if not (isCard("Slash", c, self.player) and self:hasCrossbowEffect()) then table.insert(hcards, c) end
		end
		self:sortByKeepValue(hcards)
		local cards = {}
		local hand, armor, def, off = 0, 0, 0, 0
		if self:needToThrowArmor() then
			table.insert(cards, self.player:getArmor():getEffectiveId())
			armor = 1
		end
		if (self:hasSkills(sgs.need_kongcheng) or not self:hasLoseHandcardEffective()) and self.player:getHandcardNum() > 0 then
			hand = 1
			for _, card in ipairs(hcards) do
				table.insert(cards, card:getEffectiveId())
				if #cards == 2 then break end
			end
		end
		if #cards < 2 and self:hasSkills(sgs.lose_equip_skill, self.player) then
			if #cards < 2 and self.player:getOffensiveHorse() then
				off = 1
				table.insert(cards, self.player:getOffensiveHorse():getEffectiveId())
			end
			if #cards < 2 and self.player:getArmor() then
				armor = 1
				table.insert(cards, self.player:getArmor():getEffectiveId())
			end
			if #cards < 2 and self.player:getDefensiveHorse() then
				def = 1
				table.insert(cards, self.player:getDefensiveHorse():getEffectiveId())
			end
		end

		if #cards < 2 and hand < 1 and self.player:getHandcardNum() > 2 then
			hand = 1
			for _, card in ipairs(hcards) do
				table.insert(cards, card:getEffectiveId())
				if #cards == 2 then break end
			end
		end

		if #cards < 2 and off < 1 and self.player:getOffensiveHorse() then
			off = 1
			table.insert(cards, self.player:getOffensiveHorse():getEffectiveId())
		end
		if #cards < 2 and hand < 1 and self.player:getHandcardNum() > 0 then
			hand = 1
			for _, card in ipairs(hcards) do
				table.insert(cards, card:getEffectiveId())
				if #cards == 2 then break end
			end
		end
		if #cards < 2 and armor < 1 and self.player:getArmor() then
			armor = 1
			table.insert(cards, self.player:getArmor():getEffectiveId())
		end
		if #cards < 2 and def < 1 and self.player:getDefensiveHorse() then
			def = 1
			table.insert(cards, self.player:getDefensiveHorse():getEffectiveId())
		end

		if #cards == 2 then
			local num = 0
			for _, id in ipairs(cards) do
				if self.player:hasEquip(sgs.Sanguosha:getCard(id)) then num = num + 1 end
			end
			self.equipsToDec = num
			local eff = self:damageIsEffective(effect.to, effect.nature, self.player)
			self.equipsToDec = 0
			if not eff then return "." end
			return "$" .. table.concat(cards, "+")
		end
	end
end
sgs.ai_choicemade_filter.cardResponded["@Axe"] = function(self, player, args)
	if args[#args] ~= "_nil_" then
		local target = player:getTag("axe_target"):toPlayer()
		if not target then return end
		sgs.updateIntention(player, target, 80)
	end
end

function sgs.ai_slash_weaponfilter.Axe(self, to, player)
	return self:getOverflow(player) > 0
end

function sgs.ai_weapon_value.Axe(self, enemy, player)
	if player:hasSkills("jiushi|jiuchi|luoyi|pojun") then return 6 end
	if enemy and self:getOverflow() > 0 then return 2 end
	if enemy and enemy:getHp() < 3 then return 3 - enemy:getHp() end
end

sgs.ai_skill_invoke.Blade = function(self)
	return self:invokeTouhouJudge()
end
--[[sgs.ai_skill_cardask["blade-slash"] = function(self, data, pattern, target)
	if target and self:isFriend(target) and not self:findLeijiTarget(target, 50, self.player) then
		return "."
	end
	for _, slash in ipairs(self:getCards("Slash")) do
		if self:slashIsEffective(slash, target) and (self:isWeak(target) or self:getOverflow() > 0) then
			return slash:toString()
		end
	end
	return "."
end]]

function sgs.ai_weapon_value.Blade(self, enemy)
	local invoke, value = self:invokeTouhouJudge()
	return 3 + math.abs(value)
	--if not enemy then return math.min(self:getCardsNum("Slash"), 3) end
end

function cardsView_spear(self, player, skill_name)
	local cards = player:getCards("hes")
	cards=self:touhouAppendExpandPileToList(player,cards)
	cards = sgs.QList2Table(cards)
	if skill_name ~= "fuhun" or player:hasSkill("wusheng") then
		for _, acard in ipairs(cards) do
			if isCard("Slash", acard, player) then return end
		end
	end
	local cards = player:getCards("hs")
	cards=self:touhouAppendExpandPileToList(player,cards)
	cards = sgs.QList2Table(cards)
	local newcards = {}
	for _, card in ipairs(cards) do
		if not isCard("Slash", card, player) and not isCard("Peach", card, player) and not (isCard("ExNihilo", card, player) and player:getPhase() == sgs.Player_Play) then table.insert(newcards, card) end
	end
	if #newcards < 2 then return end
	self:sortByKeepValue(newcards)

	local card_id1 = newcards[1]:getEffectiveId()
	local card_id2 = newcards[2]:getEffectiveId()

	local card_str = ("slash:%s[%s:%s]=%d+%d"):format(skill_name, "to_be_decided", 0, card_id1, card_id2)
	return card_str
end

function sgs.ai_cardsview.Spear(self, class_name, player)
	if self:touhouClassMatch(class_name, "Slash") then
		return cardsView_spear(self, player, "Spear")
	end
end

function turnUse_spear(self, inclusive, skill_name)
	local cards = self.player:getCards("hes")
	cards=self:touhouAppendExpandPileToList(self.player,cards)
	cards = sgs.QList2Table(cards)
	if skill_name ~= "fuhun" or self.player:hasSkill("wusheng") then
		for _, acard in ipairs(cards) do
			if isCard("Slash", acard, self.player) then return end
		end
	end

	local cards = self.player:getCards("hs")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards)
	local newcards = {}
	for _, card in ipairs(cards) do
		if not isCard("Slash", card, self.player) and not isCard("Peach", card, self.player) and not (isCard("ExNihilo", card, self.player) and self.player:getPhase() == sgs.Player_Play) then table.insert(newcards, card) end
	end
	if not self.player:hasSkill("yongheng") then
		if #cards <= self.player:getHp() - 1 and self.player:getHp() <= 4 and not self:hasHeavySlashDamage(self.player)
		and not self:hasSkills("kongcheng|lianying|paoxiao|shangshi|noshangshi|zhiji|benghuai") then return end
	end
	if #newcards < 2 then return end

	local card_id1 = newcards[1]:getEffectiveId()
	local card_id2 = newcards[2]:getEffectiveId()

	if newcards[1]:isBlack() and newcards[2]:isBlack() then
		local black_slash = sgs.cloneCard("slash", sgs.Card_NoSuitBlack)
		local nosuit_slash = sgs.cloneCard("slash")

		self:sort(self.enemies, "defenseSlash")
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy) and not self:slashProhibit(nosuit_slash, enemy) and self:slashIsEffective(nosuit_slash, enemy)
				and self:canAttack(enemy) and self:slashProhibit(black_slash, enemy) and self:isWeak(enemy) then
				local redcards, blackcards = {}, {}
				for _, acard in ipairs(newcards) do
					if acard:isBlack() then table.insert(blackcards, acard) else table.insert(redcards, acard) end
				end
				if #redcards == 0 then break end

				local redcard, othercard

				self:sortByUseValue(blackcards, true)
				self:sortByUseValue(redcards, true)
				redcard = redcards[1]

				othercard = #blackcards > 0 and blackcards[1] or redcards[2]
				if redcard and othercard then
					card_id1 = redcard:getEffectiveId()
					card_id2 = othercard:getEffectiveId()
					break
				end
			end
		end
	end

	local card_str = ("slash:%s[%s:%s]=%d+%d"):format(skill_name, "to_be_decided", 0, card_id1, card_id2)
	local slash = sgs.Card_Parse(card_str)
	return slash
end

local Spear_skill = {}
Spear_skill.name = "Spear"
table.insert(sgs.ai_skills, Spear_skill)
Spear_skill.getTurnUseCard = function(self, inclusive)
	return turnUse_spear(self, inclusive, "Spear")
end

function sgs.ai_weapon_value.Spear(self, enemy, player)
	if self.player:hasSkill("yongheng") then
		return 4
	end
	if enemy and getCardsNum("Slash", player, self.player) == 0 then
		if self:getOverflow(player) > 0 then return 2
		elseif player:getHandcardNum() > 2 then return 1
		end
	end
	return 0
end

function sgs.ai_slash_weaponfilter.Fan(self, to)
	return to:hasArmorEffect("Vine")
end

sgs.ai_skill_invoke.KylinBow = function(self, data)
	local damage = self.player:getTag("KylinBow"):toDamage()  --data:toDamage()
	if damage.from and damage.from:hasSkill("kuangfu") and damage.to:getCards("e"):length() == 1 then return false end
	if self:hasSkills(sgs.lose_equip_skill, damage.to) then
		return self:isFriend(damage.to)
	end
	return self:isEnemy(damage.to)
end

--sgs.ai_skill_choice.KylinBow= function(self, choices, data)

function sgs.ai_slash_weaponfilter.KylinBow(self, to)
	return to:getDefensiveHorse() or to:getOffensiveHorse()
end

function sgs.ai_weapon_value.KylinBow(self, enemy)
	if enemy and (enemy:getOffensiveHorse() or enemy:getDefensiveHorse()) then return 1 end
end

--¶«·½É±Ïà¹Ø
--¡¾ÃüÔË¡¿¡¾ç³Ïë¡¿¡¾ËÀµû¡¿
sgs.ai_skill_invoke.EightDiagram = function(self, data)

	local s = data:toCardAsked()
	if (s.prompt:split(":"))[1] == "slash-jink" then
		local slash_source = findPlayerByObjectName(self.room, (s.prompt:split(":"))[2])
		--ÓÄÓÄ×ÓËÀµû
		if slash_source and self:isFriend(slash_source)
		and self:sidieEffect(slash_source) then
			return false
		end
	end


	local dying = 0
	local handang = self.room:findPlayerBySkillName("nosjiefan")
	for _, aplayer in sgs.qlist(self.room:getAlivePlayers()) do
		if aplayer:getHp() < 1 and not aplayer:hasSkill("nosbuqu") then dying = 1 break end
	end
	if handang and self:isFriend(handang) and dying > 0 then return false end

	local heart_jink = false
	for _, card in sgs.qlist(self.player:getCards("hes")) do
		if card:getSuit() == sgs.Card_Heart and isCard("Jink", card, self.player) then
			heart_jink = true
			break
		end
	end

	if self:hasSkills("tiandu|leiji|nosleiji|gushou") then
		if self.player:hasFlag("dahe") and not heart_jink then return true end
		if sgs.hujiasource and not self:isFriend(sgs.hujiasource) and (sgs.hujiasource:hasFlag("dahe") or self.player:hasFlag("dahe")) then return true end
		if sgs.lianlisource and not self:isFriend(sgs.lianlisource) and (sgs.lianlisource:hasFlag("dahe") or self.player:hasFlag("dahe")) then return true end
		if self.player:hasFlag("dahe") and handang and self:isFriend(handang) and dying > 0 then return true end
	end
	if self.player:getHandcardNum() == 1 and self:getCardsNum("Jink") == 1 and self.player:hasSkills("zhiji|beifa") and self:needKongcheng() then
		local enemy_num = self:getEnemyNumBySeat(self.room:getCurrent(), self.player, self.player)
		if self.player:getHp() > enemy_num and enemy_num <= 1 then return false end
	end
	if handang and self:isFriend(handang) and dying > 0 then return false end
	--if self.player:hasFlag("dahe") then return false end
	--if sgs.hujiasource and (not self:isFriend(sgs.hujiasource) or sgs.hujiasource:hasFlag("dahe")) then return false end
	if sgs.lianlisource and (not self:isFriend(sgs.lianlisource) or sgs.lianlisource:hasFlag("dahe")) then return false end
	if self:getDamagedEffects(self.player, nil, true) or self:needToLoseHp(self.player, nil, true, true) then return false end
	if self:getCardsNum("Jink") == 0 then return true end
	local zhangjiao = self.room:findPlayerBySkillName("guidao")
	if zhangjiao and self:isEnemy(zhangjiao) then
		if getKnownCard(zhangjiao, self.player, "black", false, "hes") > 1 then return false end
		if self:getCardsNum("Jink") > 1 and getKnownCard(zhangjiao, self.player, "black", false, "hes") > 0 then return false end
	end

	--¶«·½ÅÐ¶¨Ïµ  ÐèÒªÌáÇ°¡£¡£¡£
	local tianzi = self.room:findPlayerBySkillName("feixiang")
	local leimi=self.room:findPlayerBySkillName("mingyun")
	if leimi and self:isEnemy(leimi) then
		if tianzi and self:isFriend(tianzi) then
			return true
		else
			return false
		end
	end
	if tianzi and self:isEnemy(tianzi) then
		return false
	end
	return true
end
--¡¾ÃüÔË¡¿¡¾ç³Ïë¡¿¡¾ºì°×¡¿¡¾ÎÞÊÙ¡¿¡¾¶·»ê¡¿
function sgs.ai_armor_value.EightDiagram(player, self)
	--ÆäÊµÐèÒªÏÈÈ·¶¨Ä¿µÄ
	--Ö±½ÓreturnÆäÊµºÜÎä¶Ï
	--[[if self.player:hasSkill("douhun") then
		if self.role == "loyalist" and self.player:getKingdom()=="zhan"
		and getLord(self.player) and getLord(self.player):hasLordSkill("tianren") then
			return 3
		else
			return 0
		end
	end]]

	if self:isValidJiahuTarget(player) then
		return 0
	end
	
	local haszj = self:hasSkills("guidao", self:getEnemies(player))
	if haszj then
		return 2
	end
	local enemyfeixiang = self:hasSkills("feixiang|mingyun", self:getEnemies(player))
	if enemyfeixiang then
		return 2
	end

	local friendfeixiang = self:hasSkills("feixiang|mingyun", self:getFriends(player))
	if friendfeixiang then
		return 6
	end
	if player:hasSkills("wushou|yangchong") then
		return 6
	end
	if player:hasSkills("tiandu|leiji|nosleiji|noszhenlie|gushou") then
		return 6
	end

	if self.role == "loyalist" and self.player:getKingdom()=="wei" and not self.player:hasSkill("bazhen") and getLord(self.player) and getLord(self.player):hasLordSkill("hujia") then
		return 5
	end
	if self.role == "loyalist" and self.player:getKingdom()=="zhan" and not self.player:hasSkill("bazhen") and getLord(self.player) and getLord(self.player):hasLordSkill("tianren") then
		return 5
	end

	return 4
end

function sgs.ai_armor_value.RenwangShield(player, self)
	if self:isValidJiahuTarget(player) then return 0 end
	if player:hasSkill("yizhong") then return 0 end
	if player:hasSkill("bazhen") then return 0 end
	if player:hasSkill("leiji") and getKnownCard(player, self.player, "Jink", true) > 1 and player:hasSkill("guidao")
		and getKnownCard(player, self.player, "black", false, "hes") > 0 then
			return 0
	end
	return 4.5
end

function sgs.ai_armor_value.SilverLion(player, self)
	if self:hasWizard(self:getEnemies(player), true) then
		for _, player in sgs.qlist(self.room:getAlivePlayers()) do
			if player:containsTrick("lightning") then return 5 end
		end
	end
	if self.player:isWounded() and not self.player:getArmor() then return 9 end
	if self.player:isWounded() and self:getCardsNum("Armor", "hs") >= 2 and not self.player:hasArmorEffect("SilverLion") then return 8 end
	if self:isHugeDamageResisted() and self.player:isWounded() then return 8 end
	if self:isValidJiahuTarget() then return 0 end
	return 1
end

sgs.ai_use_priority.OffensiveHorse = 2.69

sgs.ai_use_priority.Axe = 2.688
sgs.ai_use_priority.Halberd = 2.685
sgs.ai_use_priority.KylinBow = 2.68
sgs.ai_use_priority.Blade = 2.675
sgs.ai_use_priority.GudingBlade = 2.67
sgs.ai_use_priority.DoubleSword =2.665
sgs.ai_use_priority.Spear = 2.66
sgs.ai_use_priority.IceSword = 2.65
-- sgs.ai_use_priority.Fan = 2.655
sgs.ai_use_priority.QinggangSword = 2.645
sgs.ai_use_priority.Crossbow = 2.63

sgs.ai_use_priority.SilverLion = 1.0
-- sgs.ai_use_priority.Vine = 0.95
sgs.ai_use_priority.EightDiagram = 0.8
sgs.ai_use_priority.RenwangShield = 0.85
sgs.ai_use_priority.DefensiveHorse = 2.75

sgs.dynamic_value.damage_card.ArcheryAttack = true
sgs.dynamic_value.damage_card.SavageAssault = true

sgs.ai_use_value.ArcheryAttack = 3.8
sgs.ai_use_priority.ArcheryAttack = 3.5
sgs.ai_keep_value.ArcheryAttack = 3.38
sgs.ai_use_value.SavageAssault = 3.9
sgs.ai_use_priority.SavageAssault = 3.5
sgs.ai_keep_value.SavageAssault = 3.36

--¶«·½É±Ïà¹Ø
--¡¾ÍþÑ¹¡¿¡¾ÀíÖÇ¡¿
--self:touhouDamage
sgs.ai_skill_cardask.aoe = function(self, data, pattern, target, name) --这里要改：考虑冥乐
	if self.room:getMode():find("_mini_35") and self.player:getLostHp() == 1 and name == "archery_attack" then return "." end
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end

	local aoe
	if type(data) == "userdata" then aoe = data:toCardEffect().card else aoe = sgs.cloneCard(name) end
	assert(aoe ~= nil)

	local attacker = target


	if not self:damageIsEffective(nil, nil, attacker) then return "." end
	--if self:getDamagedEffects(self.player, attacker) or self:needToLoseHp(self.player, attacker) then return "." end

	local fakeDamage=sgs.DamageStruct(aoe, attacker, self.player, 1, self:touhouDamageNature(aoe, attacker, self.player))
	local effect, willEffect = self:touhouDamageEffect(fakeDamage,attacker,self.player)
	if not effect and (self:getDamagedEffects(self.player, attacker) or self:needToLoseHp(self.player, attacker)) then return "." end

	---*******¶«·½É±Ïà¹Ø
	--ÊÕµ½aoe 0ÉËº¦ ²»ÓÃ³öÉ±ÉÁ
	if not self:touhouNeedAvoidAttack(fakeDamage,attacker,self.player) then
		return "."
	end
	if self:hasWeiya()then
		if  not (self.player:getHandcardNum() == 1 and self:hasSkills(sgs.need_kongcheng)) then
			if pattern=="jink" and (self:getCardsNum("Jink") < 2 or self:getCardsNum("Jink", "hs", true) < 1) then
				return "."
			end
			if pattern=="slash" and (self:getCardsNum("Slash") < 2 or self:getCardsNum("Slash", "hs", true) < 1)then
				return "."
			end
		end
	end
	---*******ÒÔÉÏÎª¶«·½É±Ïà¹Ø


	if self.player:hasSkill("jianxiong") and (self.player:getHp() > 1 or self:getAllPeachNum() > 0)
		and not self:willSkipPlayPhase() then
		if not self:needKongcheng(self.player, true) and self:getAoeValue(aoe) > -10 then return "." end
		if sgs.ai_qice_data then
			local damagecard = sgs.ai_qice_data:toCardUse().card
			if damagecard:subcardsLength() > 2 then self.jianxiong = true return "." end
			for _, id in sgs.qlist(damagecard:getSubcards()) do
				local card = sgs.Sanguosha:getCard(id)
				if not self:needKongcheng(self.player, true) and isCard("Peach", card, self.player) then return "." end
			end
		end
	end

	local current = self.room:getCurrent()
	if current and current:hasSkill("juece") and self:isEnemy(current) and self.player:getHp() > 0 then
		local classname
		if name == "SavageAssault" then
			classname = "Slash"
		elseif name == "ArcheryAttack" then
			classname = "Jink"
		elseif name == "NecroMusic" then
			classname = "^BasicCard"
		end
		local use = false
		for _, card in ipairs(self:getCards(classname)) do
			if not self.player:isLastHandCard(card, true) then
				use = true
				break
			end
		end
		if not use then return "." end
	end
end

sgs.ai_skill_cardask["savage-assault-slash"] = function(self, data, pattern, target)
	return sgs.ai_skill_cardask.aoe(self, data, pattern, target, "savage_assault")
end
sgs.ai_choicemade_filter.cardResponded["savage-assault-slash"] = function(self, player, args)
	local user = findPlayerByObjectName(self.room, (args[2]:split(":"))[2])
	if user and user:hasSkill("lizhi") and args[#args] ~= "_nil_" then
		sgs.updateIntention(player, user, 80)
	end
end

sgs.ai_skill_cardask["archery-attack-jink"] = function(self, data, pattern, target)
	return sgs.ai_skill_cardask.aoe(self, data, pattern, target, "archery_attack")
end
sgs.ai_choicemade_filter.cardResponded["archery-attack-jink"] = function(self, player, args)
	local user = findPlayerByObjectName(self.room, (args[2]:split(":"))[2])
	if user and user:hasSkill("lizhi") and args[#args] ~= "_nil_" then
		sgs.updateIntention(player, user, 80)
	end
end

sgs.ai_keep_value.Nullification = 3.8
sgs.ai_use_value.Nullification = 8

--¡¾Î±×°¡¿¡¾¸ß°Á¡¿¡¾ÊÕ»ñ¡¿¡¾ÓÀºã¡¿¡¾¿ØÆ±¡¿¡¾ÎüÉ¢¡¿
function SmartAI:useCardAmazingGrace(card, use)
	if (self.role == "lord" or self.role == "loyalist" ) and sgs.turncount <= 2 and self.player:getSeat() <= 3 and self.player:aliveCount() > 5 then return end
	if (self.role == "renegade" and sgs.turncount <=1 and self.player:getSeat() <= 3) then return end
	--ÄÚ¼é¿ª¾ÖÎå¹È¼òÖ±¿Óµù Ìù±ðÊÇ¶þºÅÎ»»¹ÊÇÖÒ
	local value = 1
	local suf, coeff = 0.8, 0.8
	if self:needKongcheng() and self.player:getHandcardNum() == 1 or self.player:hasSkills("nosjizhi|jizhi") then
		suf = 0.6
		coeff = 0.6
	end
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		local index = 0
		if self:hasTrickEffective(card, player, self.player) then
			if self:isFriend(player) then
				index = 1
			elseif self:isEnemy(player)
				then index = -1
			end
		end
		if player:hasSkills("kongpiao|xisan") then
			if self:isFriend(player) then
				index=-0.5
			elseif self:isEnemy(player) then
				index=0.5
			end
		end
		--Ò»°ã¶øÑÔ¶ÔÓÚplayerÕâÊÇ»ØºÏÍâ£¬ÇÒÊ¹ÓÃÕß²»ÊÇ×Ô¼º
		if player:hasSkills("weizhuang|yongheng|gaoao") then
			if self:isFriend(player) then
				index=-1
			elseif self:isEnemy(player) then
				index=1
			end
		end
		if player:hasSkill("shouhuo") then
			if self:isFriend(player) then
				index=2
			elseif self:isEnemy(player) then
				index=-2
			end
		end

		value = value + index * suf
		if value < 0 then return end
		suf = suf * coeff
	end
	use.card = card
end

sgs.ai_use_value.AmazingGrace = 3
sgs.ai_keep_value.AmazingGrace = -1
sgs.ai_use_priority.AmazingGrace = 1.2
sgs.dynamic_value.benefit.AmazingGrace = true
sgs.ai_card_intention.AmazingGrace = function(self, card, from, tos)
	if sgs.turncount <= 1 and  from:getSeat() <= 3 then
		local lord = self.room:getLord()
		if lord then
			sgs.updateIntention(from, lord, 10)
		end
	end
end


function SmartAI:willUseGodSalvation(card)
	if not card then self.room:writeToConsole(debug.traceback()) return false end
	local good, bad = 0, 0
	local wounded_friend = 0
	local wounded_enemy = 0


	if self.player:hasSkills("nosjizhi|jizhi") then good = good + 6 end
	if (self.player:hasSkill("kongcheng") and self.player:getHandcardNum() == 1) or not self:hasLoseHandcardEffective() then good = good + 5 end

	for _, friend in ipairs(self.friends) do
		good = good + 10 * getCardsNum("Nullification", friend, self.player)
		if self:hasTrickEffective(card, friend, self.player) then
			if friend:isWounded() then
				wounded_friend = wounded_friend + 1
				good = good + 10
				if friend:isLord() then good = good + 10 / math.max(friend:getHp(), 1) end
				if self:hasSkills(sgs.masochism_skill, friend) then
					good = good + 5
				end
				if friend:getHp() <= 1 and self:isWeak(friend) then
					good = good + 5
					if friend:isLord() then good = good + 10 end
				else
					if friend:isLord() then good = good + 5 end
				end
				if self:needToLoseHp(friend, nil, nil, true, true) then good = good - 3 end
			elseif friend:hasSkill("huiwu") then good = good + 5
			end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		bad = bad + 10 * getCardsNum("Nullification", enemy, self.player)
		if self:hasTrickEffective(card, enemy, self.player) then
			if enemy:isWounded() then
				wounded_enemy = wounded_enemy + 1
				bad = bad + 10
				if enemy:isLord() then
					bad = bad + 10 / math.max(enemy:getHp(), 1)
				end
				if self:hasSkills(sgs.masochism_skill, enemy) then
					bad = bad + 5
				end
				if enemy:getHp() <= 1 and self:isWeak(enemy) then
					bad = bad + 5
					if enemy:isLord() then bad = bad + 10 end
				else
					if enemy:isLord() then bad = bad + 5 end
				end
				if self:needToLoseHp(enemy, nil, nil, true, true) then bad = bad - 3 end
			end
		end
	end
	return (good - bad > 5 and wounded_friend > 0)  or (wounded_friend == 0 and wounded_enemy == 0 and self.player:hasSkills("nosjizhi|jizhi"))
end

function SmartAI:useCardGodSalvation(card, use)
	if self:willUseGodSalvation(card) then
		use.card = card
	end
end

sgs.ai_use_priority.GodSalvation = 1.1
sgs.ai_keep_value.GodSalvation = 3.32
sgs.dynamic_value.benefit.GodSalvation = true
sgs.ai_card_intention.GodSalvation = function(self, card, from, tos)
	local can, first
	for _, to in ipairs(tos) do
		if to:isWounded() and not first then
			first = to
			can = true
		elseif first and to:isWounded() and not self:isFriend(first, to) then
			can = false
			break
		end
	end
	if can then
		sgs.updateIntention(from, first, -10)
	end
end

function SmartAI:JijiangSlash(player)
	if not player then self.room:writeToConsole(debug.traceback()) return 0 end
	if not player:hasLordSkill("jijiang")
	or not player:hasLordSkill("tianren")  then return 0 end
	local slashs = 0
	local kingdom= "shu"
	if player:hasLordSkill("tianren") then
		kingdom= "zhan"
	end
	for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
		local slash_num = getCardsNum("Slash", p, self.player)
		if p:getKingdom() == kingdom and slash_num >= 1 and sgs.card_lack[p:objectName()]["Slash"] ~= 1 and
			(sgs.turncount <= 1 and sgs.ai_role[p:objectName()] == "neutral" or self:isFriend(player, p)) then
				slashs = slashs + slash_num
		end
	end
	return slashs
end

function SmartAI:useCardDuel(duel, use)

	self:updatePlayers() --ÎªºÎself.enemiesÃ»ÓÐ±»¼°Ê±¸üÐÂ£¿£¿£¿ phasestartÊ±²»ÊÇÒª¸üÐÂÃ´ ÎÔ²Û

	local enemies = self:exclude(self.enemies, duel)
	local friends = self:exclude(self.friends_noself, duel)
	local n1 = self:getCardsNum("Slash")
	local huatuo = self.room:findPlayerBySkillName("jijiu")
	local targets = {}

	local canUseDuelTo=function(target)
		local prohibit=self:trickProhibit(duel, target, self.player)
		if not prohibit and  not self:isFriend(target)
		and self:touhouCardAttackWaste(duel,self.player,target) then
			prohibit=true
		end
		return not prohibit and self:hasTrickEffective(duel, target) and self:damageIsEffective(target,sgs.DamageStruct_Normal) and not self.room:isProhibited(self.player, target, duel)
	end

	for _, friend in ipairs(friends) do
		if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
			and friend:hasSkill("jieming") and canUseDuelTo(friend) and self.player:hasSkill("nosrende") and (huatuo and self:isFriend(huatuo)) then
			table.insert(targets, friend)
		end
	end

	for _, enemy in ipairs(enemies) do
		if (not use.current_targets or not table.contains(use.current_targets, enemy:objectName()))
			and self.player:hasFlag("duelTo_" .. enemy:objectName()) and canUseDuelTo(enemy) then
			table.insert(targets, enemy)
		end
	end

	local cmp = function(a, b)
		local v1 = getCardsNum("Slash", a) + a:getHp()
		local v2 = getCardsNum("Slash", b) + b:getHp()

		if self:getDamagedEffects(a, self.player) then v1 = v1 + 20 end
		if self:getDamagedEffects(b, self.player) then v2 = v2 + 20 end

		if not self:isWeak(a) and a:hasSkill("jianxiong") then v1 = v1 + 10 end
		if not self:isWeak(b) and b:hasSkill("jianxiong") then v2 = v2 + 10 end

		if self:needToLoseHp(a) then v1 = v1 + 5 end
		if self:needToLoseHp(b) then v2 = v2 + 5 end

		if self:hasSkills(sgs.masochism_skill, a) then v1 = v1 + 5 end
		if self:hasSkills(sgs.masochism_skill, b) then v2 = v2 + 5 end

		if not self:isWeak(a) and a:hasSkill("jiang") then v1 = v1 + 5 end
		if not self:isWeak(b) and b:hasSkill("jiang") then v2 = v2 + 5 end

		if a:hasLordSkill("jijiang") or a:hasLordSkill("tianren")  then v1 = v1 + self:JijiangSlash(a) * 2 end
		if b:hasLordSkill("jijiang") or b:hasLordSkill("tianren")  then v2 = v2 + self:JijiangSlash(b) * 2 end

		if v1 == v2 then return sgs.getDefenseSlash(a, self) < sgs.getDefenseSlash(b, self) end

		return v1 < v2
	end

	table.sort(enemies, cmp)

	for _, enemy in ipairs(enemies) do
		local useduel
		local n2 = getCardsNum("Slash",enemy)
		if sgs.card_lack[enemy:objectName()]["Slash"] == 1 then n2 = 0 end
		useduel = n1 >= n2 or self:needToLoseHp(self.player, nil, nil, true)
					or self:getDamagedEffects(self.player, enemy) or (n2 < 1 and sgs.isGoodHp(self.player))
					or ((self:hasSkill("jianxiong") or self.player:getMark("shuangxiong") > 0) and sgs.isGoodHp(self.player))
		
		local duel_to_flandre = true
		if enemy:hasSkill("wosui") and self.player:getHp() >= enemy:getHp() then
			if getKnownCard(enemy, self.player, "Slash", true) > 0 then duel_to_flandre = false end
			if not enemy:isKongcheng() or enemy:getPile("wooden_ox"):length() > 0 then
				local all_num = enemy:getHandcardNum() + enemy:getPile("wooden_ox"):length()
				local visible_num = 0
				local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
				for _, c in sgs.qlist(enemy:getHandcards()) do
					if (c:hasFlag("visible") or c:hasFlag(flag)) and not c:isKindOf("Slash") then
						visible_num = visible_num + 1
					end
				end
				for _, id in sgs.qlist(enemy:getPile("wooden_ox")) do
					local c = sgs.Sanguosha:getCard(id)
					if (c:hasFlag("visible") or c:hasFlag(flag)) and not c:isKindOf("Slash") then
						visible_num = visible_num + 1
					end
				end
				if visible_num < all_num then duel_to_flandre = false end
			end
		end

		if (not use.current_targets or not table.contains(use.current_targets, enemy:objectName()))
				and self:objectiveLevel(enemy) > 3 and canUseDuelTo(enemy) and not self:cantbeHurt(enemy) and useduel
				and duel_to_flandre and sgs.isGoodTarget(enemy, enemies, self) then
			if not table.contains(targets, enemy) then table.insert(targets, enemy) end
		end
	end

	if #targets > 0 then

		local godsalvation = self:getCard("GodSalvation")
		if godsalvation and godsalvation:getId() ~= duel:getId() and self:willUseGodSalvation(godsalvation) then
			local use_gs = true
			for _, p in ipairs(targets) do
				if not p:isWounded() or not self:hasTrickEffective(godsalvation, p, self.player) then break end
				use_gs = false
			end
			if use_gs then
				use.card = godsalvation
				return
			end
		end

		local targets_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, duel)
		if use.isDummy and use.xiechan then targets_num = 100 end
		local enemySlash = 0
		local setFlag = false

		use.card = duel

		for i = 1, #targets, 1 do
			local n2 = getCardsNum("Slash", targets[i])
			if sgs.card_lack[targets[i]:objectName()]["Slash"] == 1 then n2 = 0 end
			if self:isEnemy(targets[i]) then enemySlash = enemySlash + n2 end

			if use.to then
				if i == 1 and not use.current_targets then
					use.to:append(targets[i])
					if not use.isDummy then self:speak("duel", self.player:isFemale()) end
				elseif n1 >= enemySlash then
					use.to:append(targets[i])
				end
				if not setFlag and self.player:getPhase() == sgs.Player_Play and self:isEnemy(targets[i]) then
					self.player:setFlags("duelTo" .. targets[i]:objectName())
					setFlag = true
				end
				if use.to:length() == targets_num then return end
			end
		end
	end

end


sgs.ai_card_intention.Duel = function(self, card, from, tos)
	if string.find(card:getSkillName(), "lijian") then return end
	if string.find(card:getSkillName(), "chuangshi") then return end
	if string.find(card:getSkillName(), "nihuo") then return end
	sgs.updateIntentions(from, tos, 80)
end

sgs.ai_use_value.Duel = 3.7
sgs.ai_use_priority.Duel = 2.9
sgs.ai_keep_value.Duel = 3.42

sgs.dynamic_value.damage_card.Duel = true

--²»¿¼ÂÇÍþÑ¹£¿£¿
--¡¾Õ½Òâ¡¿
sgs.ai_skill_cardask["duel-slash"] = function(self, data, pattern, target)
	if self.player:getPhase()==sgs.Player_Play then return self:getCardId("Slash") end
	if self:hasSkills("zhanyi|chaoren") and not self:isFriend(target) then
		return self:getCardId("Slash")
	end
	if self.player:hasSkill("wosui") and not self:isFriend(target) and target:getHp() >= self.player:getHp() then
		return self:getCardId("Slash")
	end
	if target:hasSkill("souji") and target:isAlive() and self.room:getCurrent():objectName()==target:objectName()  then
		return "."
	end

	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if self.player:hasFlag("AIGlobal_NeedToWake") and self.player:getHp() > 1 then return "." end
	if self.player:hasSkill("wuhun") and self:isEnemy(target) and target:isLord() and #self.friends_noself > 0 then return "." end

	if self:cantbeHurt(target) then return "." end

	if self:isFriend(target) and target:hasSkill("rende") and self.player:hasSkill("jieming") then return "." end
	if self:isEnemy(target) and not self:isWeak() and self:getDamagedEffects(self.player, target) then return "." end

	if self:isFriend(target) then
		if self:getDamagedEffects(self.player, target) or self:needToLoseHp(self.player, target) then return "." end
		if self:getDamagedEffects(target, self.player) or self:needToLoseHp(target, self.player) then
			return self:getCardId("Slash")
		else
			if target:isLord() and not sgs.isLordInDanger() and not sgs.isGoodHp(self.player) then return self:getCardId("Slash") end
			if self.player:isLord() and sgs.isLordInDanger() then return self:getCardId("Slash") end
			return "."
		end
	end

	if (not self:isFriend(target) and self:getCardsNum("Slash") >= getCardsNum("Slash", target, self.player))
		or (target:getHp() > 2 and self.player:getHp() <= 1 and self:getCardsNum("Peach") == 0 and not self.player:hasSkill("buqu")) then
		return self:getCardId("Slash")
	else return "." end

end

function SmartAI:useCardExNihilo(card, use)
	local xiahou = self.room:findPlayerBySkillName("yanyu")
	if xiahou and self:isEnemy(xiahou) and xiahou:getMark("YanyuDiscard2") > 0 then return end

	use.card = card
	if not use.isDummy then
		self:speak("lucky")
	end
end

sgs.ai_card_intention.ExNihilo = -80

sgs.ai_keep_value.ExNihilo = 3.9
sgs.ai_use_value.ExNihilo = 10
sgs.ai_use_priority.ExNihilo = 9.3

sgs.dynamic_value.benefit.ExNihilo = true

function SmartAI:getDangerousCard(who)
	local weapon = who:getWeapon()
	local armor = who:getArmor()
	local treasure = who:getTreasure()
	local offensiveHorse = who:getOffensiveHorse()
	local defensiveHorse = who:getDefensiveHorse()

	--Ä§²Ù¶Ô²ß
	local shrx = self.room:findPlayerBySkillName("mocao")
	if shrx and shrx:objectName() ~= who:objectName()  and self:isFriend(shrx, who) and who:getLostHp() >= 2 then
		for _,equip in sgs.qlist(who:getEquips()) do
			return equip:getEffectiveId()
		end
	end
	if treasure and treasure:isKindOf("WoodenOx") and who:getPile("wooden_ox"):length() > 1 then
		return treasure:getEffectiveId()
	end
	--±£Ö÷
	local lord = self.room:getLord()
	if lord and self:isEnemy(lord, who)  and sgs.isLordInDanger() then
		local distance = who:distanceTo(lord)
		if weapon and who:getAttackRange() > distance then
			return weapon:getEffectiveId()
		end
		if offensiveHorse and distance <= 2 then
			return offensiveHorse:getEffectiveId()
		end
	end



	if weapon and (weapon:isKindOf("Crossbow") or weapon:isKindOf("GudingBlade")) then
		for _, friend in ipairs(self.friends) do
			if weapon:isKindOf("Crossbow") and who:distanceTo(friend) <= 1 and getCardsNum("Slash", who, self.player) > 0 then
				return weapon:getEffectiveId()
			end
			if weapon:isKindOf("GudingBlade") and who:inMyAttackRange(friend) and friend:isKongcheng() and not friend:hasSkills("kongcheng|tianming") and getCardsNum("Slash", who) > 0 then
				return weapon:getEffectiveId()
			end
		end
	end
	if who:hasSkills(sgs.attackRange_skill) then
		if weapon and who:getAttackRange() > 1  then
			return weapon:getEffectiveId()
		end
		if offensiveHorse then
			return offensiveHorse:getEffectiveId()
		end
	end

	if (weapon and weapon:isKindOf("Spear") and who:hasSkill("paoxiao") and who:getHandcardNum() >=1 ) then return weapon:getEffectiveId() end
	if weapon and weapon:isKindOf("Axe")  then
		if  self:hasSkills("luoyi|pojun|jiushi|jiuchi|jie|wenjiu|shenli|jieyuan", who) or who:getCards("hes"):length() >=4 then
			return weapon:getEffectiveId()
		end
	end
	if armor and armor:isKindOf("EightDiagram") and who:hasSkill("leiji") then return armor:getEffectiveId() end
	if armor and who:hasSkill("wunian") then return armor:getEffectiveId() end
	if defensiveHorse and who:hasSkill("wunian") then return defensiveHorse:getEffectiveId() end


	if lord and lord:hasLordSkill("hujia") and self:isEnemy(lord) and armor and armor:isKindOf("EightDiagram") and who:getKingdom() == "wei" then
		return armor:getEffectiveId()
	end
	if lord and lord:hasLordSkill("tianren") and self:isEnemy(lord) and armor and armor:isKindOf("EightDiagram") and who:getKingdom() == "zhan" then
		return armor:getEffectiveId()
	end
	if (weapon and weapon:isKindOf("SPMoonSpear") and self:hasSkills("guidao|longdan|guicai|jilve|huanshi|qingguo|kanpo", who)) then
		return weapon:getEffectiveId()
	end
	if (weapon and who:hasSkill("liegong")) then return weapon:getEffectiveId() end
end

function SmartAI:getValuableCard(who)
	local weapon = who:getWeapon()
	local armor = who:getArmor()
	local offhorse = who:getOffensiveHorse()
	local defhorse = who:getDefensiveHorse()
	self:sort(self.friends, "hp")
	local friend
	if #self.friends > 0 then friend = self.friends[1] end
	if friend and self:isWeak(friend) and who:distanceTo(friend) <= who:getAttackRange() and not self:doNotDiscard(who, "e", true) then
		if weapon and (who:distanceTo(friend) > 1) then
			return weapon:getEffectiveId()
		end
		if offhorse and who:distanceTo(friend) > 1 then
			return offhorse:getEffectiveId()
		end
	end
	--¸÷Àà×°±¸µÄÖØÊÓ³Ì¶È£¿£¿
	if weapon then
		if (weapon:isKindOf("MoonSpear") and who:hasSkill("keji") and who:getHandcardNum() > 5)
		  or self:hasSkills("qiangxi|zhulou|taichen", who) then
			return weapon:getEffectiveId()
		end
	end
	if defhorse and not self:doNotDiscard(who, "e") then
		return defhorse:getEffectiveId()
	end

	if armor and self:evaluateArmor(armor, who) > 3
	  and not self:needToThrowArmor(who)
	  and not self:doNotDiscard(who, "e") then
		return armor:getEffectiveId()
	end

	if treasure then
		if treasure:isKindOf("WoodenOx") and who:getPile("wooden_ox"):length() > 1 then
			return treasure:getEffectiveId()
		end
	end
	if offhorse then
		if self:hasSkills("nosqianxi|kuanggu|duanbing|qianxi", who) then
			return offhorse:getEffectiveId()
		end
	end

	local equips = sgs.QList2Table(who:getEquips())
	for _,equip in ipairs(equips) do
		if self:hasSkills("guose|yanxiao", who) and equip:getSuit() == sgs.Card_Diamond then  return equip:getEffectiveId() end
		if who:hasSkill("baobian") and who:getHp() <= 2 then return  equip:getEffectiveId() end
		if self:hasSkills("qixi|duanliang|yinling|guidao", who) and equip:isBlack() then  return equip:getEffectiveId() end
		if self:hasSkills("wusheng|jijiu|xueji|nosfuhun", who) and equip:isRed() then  return equip:getEffectiveId() end
		if self:hasSkills(sgs.need_equip_skill, who) and not self:hasSkills(sgs.lose_equip_skill, who) then return equip:getEffectiveId() end
	end

	if armor and not self:needToThrowArmor(who) and not self:doNotDiscard(who, "e") then
		return armor:getEffectiveId()
	end

	if offhorse and who:getHandcardNum() > 1 then
		if not self:doNotDiscard(who, "e", true) then
			for _,friend in ipairs(self.friends) do
				if who:distanceTo(friend) == who:getAttackRange() and who:getAttackRange() > 1 then
					return offhorse:getEffectiveId()
				end
			end
		end
	end

	if treasure then
		return treasure:getEffectiveId()
	end

	if weapon and who:getHandcardNum() > 1 then
		if not self:doNotDiscard(who, "e", true) then
			for _,friend in ipairs(self.friends) do
				if (who:distanceTo(friend) <= who:getAttackRange()) and (who:distanceTo(friend) > 1) then
					return weapon:getEffectiveId()
				end
			end
		end
	end
end

function SmartAI:findSnatchOrDismantlementTarget(card, use)
	local isJixi = card:getSkillName() == "jixi"
	local isDiscard = card:isKindOf("Dismantlement")
	local name = card:objectName()
	local using_2013 = (name == "dismantlement") and self.room:getMode() == "02_1v1" and sgs.GetConfig("1v1/Rule", "Classical") ~= "Classical"

	local players = self.room:getOtherPlayers(self.player)
	local tricks
	local usecard = false

	local targets = {}
	local targets_num = (1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card))

	local addTarget = function(player, cardid)
		if not table.contains(targets, player:objectName())
				and (not use.current_targets or not table.contains(use.current_targets, player:objectName())) then
			table.insert(targets, player:objectName())
			return true
		end
	end


	--this part check "Lightning"
	players = self:exclude(players, card)
	if not using_2013 then
		for _, player in ipairs(players) do
			local judgeMode=self:getFinalRetrial(player)
			if not player:getJudgingArea():isEmpty() and self:hasTrickEffective(card, player)
				and ((player:containsTrick("lightning") and  judgeMode== 2)
				or (#self.enemies == 0 and not (player:containsTrick("lightning") and  judgeMode== 1))) then  --È·ÈÏµÄµÐÈËÎª0Äã¾Í²ðÁË ¿Óµù  ÖÒÄÚ²Ð¾Ö ÒòÎªÓÐµÐÈË·´µ¹²»²ðÁË ²ÁÄãÃÃ
				if player:hasSkill("baoyi") then continue end
				local tricks = player:getCards("j")
				for _, trick in sgs.qlist(tricks) do
					if trick:isKindOf("Lightning") and (not isDiscard or self.player:canDiscard(player, trick:getId())) then
						if addTarget(player, trick:getEffectiveId()) then return true end
					end
				end
			end
		end
	end
	local enemies = {}
	if #self.enemies == 0 and self:getOverflow() > 0 then--ÄáÂêÆäÊµµ±Ê±getoverflowÎª0£¬×°ÁË¼¸¸ö×°±¸£¨ÓÅÏÈ¶ÈµÍ£©¾Í±ä³ÉÀË·ÑÅÆÁË ²Ý
		--sgs.ai_use_priority.Snatch = true and 0 or 4.3
		local lord = self.room:getLord()
		for _, player in ipairs(players) do
			if not self:isFriend(player) then
				if lord and self.player:isLord() then
					local kingdoms = {}
					if lord:getGeneral():isLord() then table.insert(kingdoms, lord:getGeneral():getKingdom()) end
					if lord:getGeneral2() and lord:getGeneral2():isLord() then table.insert(kingdoms, lord:getGeneral2():getKingdom()) end
					if not table.contains(kingdoms, player:getKingdom()) and not lord:hasSkill("yongsi") then table.insert(enemies, player) end
				elseif lord and player:objectName() ~= lord:objectName() then
					table.insert(enemies, player)
				elseif not lord then
					table.insert(enemies, player)
				end
			end
		end
		enemies = self:exclude(enemies, card)
		self:sort(enemies, "defense")--²ðË³Ã¤¾ÑµÄÇé¿öÏÂ£¬ÓÃdefenseÅÆÐòÊÇÊ²Ã´ÒâË¼¡£ ²»¸ÃÊÇthreatÖ®Á÷µÄÃ´¡£¡£¡£
		enemies = sgs.reverse(enemies)--Õâ¾ÍÊÇÂèµÄ¶þÎ»ÖÒÄ¬Ä¬×°ÉÏ×°±¸¹ý£¬ÈýÎ»ÖÒÆðÊÖ¾Í²ðµÄÔ­ÒòÃ´¡£¡£¡£È«³¡¾Í¶þºÅÎ»×î¸ß¡£¡£¡£
		--½â¾ö·½°¸£¬1 self.enemy µÄÅÐ¶Ï¼ÓÈëÃ¤¾ÑÒòËØ
		--2ÕâÀïÒýÈë¶ÔÖ÷¹«µÄÍþÐ²ÖµµÄÅÐ¶Ï¡£
	else
		enemies = self:exclude(self.enemies, card)
		self:sort(enemies, "defense")
	end

	for _, enemy in ipairs(enemies) do
		if self:slashIsAvailable() then
			for _, slash in ipairs(self:getCards("Slash")) do
				if not self:slashProhibit(slash, enemy) and enemy:getHandcardNum() == 1 and enemy:getHp() == 1 and self:hasLoseHandcardEffective(enemy)
					and self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy) and not enemy:hasSkills("kongcheng|tianming")
					and self.player:canSlash(enemy, slash) and self:hasTrickEffective(card, enemy)
					and (not enemy:isChained() or self:isGoodChainTarget(enemy, nil, nil, nil, slash))
					and (not self:hasEightDiagramEffect(enemy) or IgnoreArmor(self.player, enemy)) then
					if addTarget(enemy, enemy:getHandcards():first():getEffectiveId()) then return true end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			local dangerous = self:getDangerousCard(enemy)
			if dangerous and (not isDiscard or self.player:canDiscard(enemy, dangerous)) then
				if addTarget(enemy, dangerous) then return true end
			end
		end
	end

	--this part check   friend who has DelayedTrick
	self:sort(self.friends_noself, "defense")
	local friends = self:exclude(self.friends_noself, card)
	if not using_2013 then
		for _, friend in ipairs(friends) do
			if friend:hasSkill("baoyi") then continue end
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) and not friend:containsTrick("YanxiaoCard")
				and self:hasTrickEffective(card, friend) then
				local cardchosen
				tricks = friend:getJudgingArea()
				for _, trick in sgs.qlist(tricks) do
					if trick:isKindOf("Indulgence") and (not isDiscard or self.player:canDiscard(friend, trick:getId())) then
						if friend:getHp() <= friend:getHandcardNum() or friend:isLord() or name == "snatch" then
							cardchosen = trick:getEffectiveId()
							break
						end
					end
					if trick:isKindOf("SupplyShortage") and (not isDiscard or self.player:canDiscard(friend, trick:getId())) then
						cardchosen = trick:getEffectiveId()
						break
					end
					if trick:isKindOf("Indulgence") and (not isDiscard or self.player:canDiscard(friend, trick:getId())) then
						cardchosen = trick:getEffectiveId()
						break
					end
				end
				if cardchosen then
					if addTarget(friend, cardchosen) then return true end
				end
			end
		end
	end

	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(card, friend) and self:needToThrowArmor(friend) and (not isDiscard or self.player:canDiscard(friend, friend:getArmor():getEffectiveId())) then
			hasLion = true
			target = friend
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			local valuable = self:getValuableCard(enemy)
			if valuable and (not isDiscard or self.player:canDiscard(enemy, valuable)) then
				if addTarget(enemy, valuable) then return true end
			end
		end
	end

	local new_enemies = table.copyFrom(enemies)
	local compare_JudgingArea = function(a, b)
		return a:getJudgingArea():length() > b:getJudgingArea():length()
	end
	table.sort(new_enemies, compare_JudgingArea)
	local yanxiao_card, yanxiao_target, yanxiao_prior
	if not using_2013 then
		for _, enemy in ipairs(new_enemies) do
			for _, acard in sgs.qlist(enemy:getJudgingArea()) do
				if acard:isKindOf("YanxiaoCard") and self:hasTrickEffective(card, enemy) and (not isDiscard or self.player:canDiscard(enemy, acard:getId())) then
					yanxiao_card = acard
					yanxiao_target = enemy
					if enemy:containsTrick("indulgence") or enemy:containsTrick("supply_shortage") then yanxiao_prior = true end
					break
				end
			end
			if yanxiao_card and yanxiao_target then break end
		end
		if yanxiao_prior and yanxiao_card and yanxiao_target then
			if addTarget(yanxiao_target, yanxiao_card:getEffectiveId()) then return true end
		end
	end

	for _, enemy in ipairs(enemies) do
		local cards = sgs.QList2Table(enemy:getHandcards())
		local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
		if #cards <= 2 and self:hasTrickEffective(card, enemy) and not enemy:isKongcheng() and not self:doNotDiscard(enemy, "hs", true) then
			for _, cc in ipairs(cards) do
				if (cc:hasFlag("visible") or cc:hasFlag(flag)) and (cc:isKindOf("Peach") or cc:isKindOf("Analeptic")) then
					if addTarget(enemy, self:getCardRandomly(enemy, "hs")) then return true end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			if self:hasSkills("jijiu|qingnang|jieyin", enemy) then
				local cardchosen
				local equips = { enemy:getDefensiveHorse(), enemy:getArmor(), enemy:getOffensiveHorse(), enemy:getWeapon() }
				for _, equip in ipairs(equips) do
					if equip and (not enemy:hasSkill("jijiu") or equip:isRed()) and (not isDiscard or self.player:canDiscard(enemy, equip:getEffectiveId())) then
						cardchosen = equip:getEffectiveId()
						break
					end
				end

				if not cardchosen and enemy:getDefensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId())) then cardchosen = enemy:getDefensiveHorse():getEffectiveId() end
				if not cardchosen and enemy:getArmor() and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then
					cardchosen = enemy:getArmor():getEffectiveId()
				end
				if not cardchosen and not enemy:isKongcheng() and enemy:getHandcardNum() <= 3 and (not isDiscard or self.player:canDiscard(enemy, "hs")) then
					cardchosen = self:getCardRandomly(enemy, "hs")
				end

				if cardchosen then
					if addTarget(enemy, cardchosen) then return true end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if self:hasTrickEffective(card, enemy) and enemy:hasArmorEffect("EightDiagram") and enemy:getArmor()
			and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then
			addTarget(enemy, enemy:getArmor():getEffectiveId())
		end
	end

	for i = 1, 2 + (isJixi and 3 or 0), 1 do
		for _, enemy in ipairs(enemies) do
			if not enemy:isNude() and self:hasTrickEffective(card, enemy)
				and not (self:needKongcheng(enemy) and i <= 2) and not self:doNotDiscard(enemy) then
				if (enemy:getHandcardNum() == i and sgs.getDefenseSlash(enemy) < 6 + (isJixi and 6 or 0) and enemy:getHp() <= 3 + (isJixi and 2 or 0)) then
					local cardchosen
					if self.player:distanceTo(enemy) == self.player:getAttackRange() + 1 and enemy:getDefensiveHorse() and not self:doNotDiscard(enemy, "e")
						and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId()))then
						cardchosen = enemy:getDefensiveHorse():getEffectiveId()
					elseif enemy:getArmor() and not self:needToThrowArmor(enemy) and not self:doNotDiscard(enemy, "e")
						and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId()))then
						cardchosen = enemy:getArmor():getEffectiveId()
					elseif not isDiscard or self.player:canDiscard(enemy, "hs") then
						cardchosen = self:getCardRandomly(enemy, "hs")
					end
					if cardchosen then
						if addTarget(enemy, cardchosen) then return true end
					end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			local valuable = self:getValuableCard(enemy)
			if valuable and (not isDiscard or self.player:canDiscard(enemy, valuable)) then
				if addTarget(enemy, valuable) then return true end
			end
		end
	end

	if hasLion and (not isDiscard or self.player:canDiscard(target, target:getArmor():getEffectiveId())) then
		if addTarget(target, target:getArmor():getEffectiveId()) then return true end
	end

	if not using_2013
		and yanxiao_card and yanxiao_target and (not isDiscard or self.player:canDiscard(yanxiao_target, yanxiao_card:getId())) then
		if addTarget(yanxiao_target, yanxiao_card:getEffectiveId()) then return true end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isKongcheng() and not self:doNotDiscard(enemy, "hs") and self:hasTrickEffective(card, enemy)
			and self:hasSkills(sgs.cardneed_skill, enemy) and (not isDiscard or self.player:canDiscard(enemy, "hs")) then
			if addTarget(enemy, self:getCardRandomly(enemy, "hs")) then return true end
		end
	end

	for _, enemy in ipairs(enemies) do
		if enemy:hasEquip() and not self:doNotDiscard(enemy, "e") and self:hasTrickEffective(card, enemy) then
			local cardchosen
			if enemy:getDefensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId())) then
				cardchosen = enemy:getDefensiveHorse():getEffectiveId()
			elseif enemy:getArmor() and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then
				cardchosen = enemy:getArmor():getEffectiveId()
			elseif enemy:getOffensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getOffensiveHorse():getEffectiveId())) then
				cardchosen = enemy:getOffensiveHorse():getEffectiveId()
			elseif enemy:getWeapon() and (not isDiscard or self.player:canDiscard(enemy, enemy:getWeapon():getEffectiveId())) then
				cardchosen = enemy:getWeapon():getEffectiveId()
			end
			if cardchosen then
				if addTarget(enemy, cardchosen) then return true end
			end
		end
	end

	if name == "snatch" or self:getOverflow() > 0 then
		for _, enemy in ipairs(enemies) do
			local equips = enemy:getEquips()
			if not enemy:isNude() and self:hasTrickEffective(card, enemy) and not self:doNotDiscard(enemy, "hes") then
				local cardchosen
				if not equips:isEmpty() and not self:doNotDiscard(enemy, "e") then
					cardchosen = self:getCardRandomly(enemy, "e")
				else
					cardchosen = self:getCardRandomly(enemy, "hs") end
				if cardchosen then
					if addTarget(enemy, cardchosen) then return true end
				end
			end
		end
	end

	return false
end


--¶«·½É±Ïà¹Ø
--Ö÷À×Ã× ¶þºÅÎ»ÖÒ¹Òµç  ÈýºÅÎ»ÖÒ¸ø²ðÁËµÄÄªÃûÆäÃî ÐÞ¸ÄºúÂÒ²ðÉÁµçµÄÇé¿ö
function SmartAI:useCardSnatchOrDismantlement(card, use)
	local isJixi = card:getSkillName() == "jixi"
	local isDiscard = card:isKindOf("Dismantlement")
	local name = card:objectName()
	local using_2013 = (name == "dismantlement") and self.room:getMode() == "02_1v1" and sgs.GetConfig("1v1/Rule", "Classical") ~= "Classical"

	local players = self.room:getOtherPlayers(self.player)
	local tricks
	local usecard = false

	local targets = {}
	local targets_num = (1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card))

	local addTarget = function(player, cardid)
		if not table.contains(targets, player:objectName())
			and (not use.current_targets or not table.contains(use.current_targets, player:objectName())) then
			if not usecard then
				use.card = card
				usecard = true
			end
			table.insert(targets, player:objectName())
			if usecard and use.to and use.to:length() < targets_num then
				use.to:append(player)
				if not use.isDummy then
					sgs.Sanguosha:getCard(cardid):setFlags("AIGlobal_SDCardChosen_" .. name)
					if use.to:length() == 1 then self:speak("hostile", self.player:isFemale()) end
				end
			end
			if #targets == targets_num then return true end
		end
	end


	--this part check "Lightning"
	players = self:exclude(players, card)
	if not using_2013 then
		for _, player in ipairs(players) do
			local judgeMode=self:getFinalRetrial(player)
			if not player:getJudgingArea():isEmpty() and self:hasTrickEffective(card, player)
				and ((player:containsTrick("lightning") and  judgeMode== 2)
				or (#self.enemies == 0 and not (player:containsTrick("lightning") and  judgeMode== 1))) then  --È·ÈÏµÄµÐÈËÎª0Äã¾Í²ðÁË ¿Óµù  ÖÒÄÚ²Ð¾Ö ÒòÎªÓÐµÐÈË·´µ¹²»²ðÁË ²ÁÄãÃÃ
				if player:hasSkill("baoyi") then continue end
				local tricks = player:getCards("j")
				for _, trick in sgs.qlist(tricks) do
					if trick:isKindOf("Lightning") and (not isDiscard or self.player:canDiscard(player, trick:getId())) then
						if addTarget(player, trick:getEffectiveId()) then return end
					end
				end
			end
		end
	end
	local enemies = {}
	if #self.enemies == 0 and self:getOverflow() > 0 then--ÄáÂêÆäÊµµ±Ê±getoverflowÎª0£¬×°ÁË¼¸¸ö×°±¸£¨ÓÅÏÈ¶ÈµÍ£©¾Í±ä³ÉÀË·ÑÅÆÁË ²Ý
		--sgs.ai_use_priority.Snatch = true and 0 or 4.3
		local lord = self.room:getLord()
		for _, player in ipairs(players) do
			if not self:isFriend(player) then
				if lord and self.player:isLord() then
					local kingdoms = {}
					if lord:getGeneral():isLord() then table.insert(kingdoms, lord:getGeneral():getKingdom()) end
					if lord:getGeneral2() and lord:getGeneral2():isLord() then table.insert(kingdoms, lord:getGeneral2():getKingdom()) end
					if not table.contains(kingdoms, player:getKingdom()) and not lord:hasSkill("yongsi") then table.insert(enemies, player) end
				elseif lord and player:objectName() ~= lord:objectName() then
					table.insert(enemies, player)
				elseif not lord then
					table.insert(enemies, player)
				end
			end
		end
		enemies = self:exclude(enemies, card)
		self:sort(enemies, "defense")--²ðË³Ã¤¾ÑµÄÇé¿öÏÂ£¬ÓÃdefenseÅÆÐòÊÇÊ²Ã´ÒâË¼¡£ ²»¸ÃÊÇthreatÖ®Á÷µÄÃ´¡£¡£¡£
		enemies = sgs.reverse(enemies)--Õâ¾ÍÊÇÂèµÄ¶þÎ»ÖÒÄ¬Ä¬×°ÉÏ×°±¸¹ý£¬ÈýÎ»ÖÒÆðÊÖ¾Í²ðµÄÔ­ÒòÃ´¡£¡£¡£È«³¡¾Í¶þºÅÎ»×î¸ß¡£¡£¡£
		--½â¾ö·½°¸£¬1 self.enemy µÄÅÐ¶Ï¼ÓÈëÃ¤¾ÑÒòËØ
		--2ÕâÀïÒýÈë¶ÔÖ÷¹«µÄÍþÐ²ÖµµÄÅÐ¶Ï¡£
	else
		enemies = self:exclude(self.enemies, card)
		self:sort(enemies, "defense")
	end

	for _, enemy in ipairs(enemies) do
		if self:slashIsAvailable() then
			for _, slash in ipairs(self:getCards("Slash")) do
				if not self:slashProhibit(slash, enemy) and enemy:getHandcardNum() == 1 and enemy:getHp() == 1 and self:hasLoseHandcardEffective(enemy)
					and self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy) and not enemy:hasSkills("kongcheng|tianming")
					and self.player:canSlash(enemy, slash) and self:hasTrickEffective(card, enemy)
					and (not enemy:isChained() or self:isGoodChainTarget(enemy, nil, nil, nil, slash))
					and (not self:hasEightDiagramEffect(enemy) or IgnoreArmor(self.player, enemy)) then
					if addTarget(enemy, enemy:getHandcards():first():getEffectiveId()) then return end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			local dangerous = self:getDangerousCard(enemy)
			if dangerous and (not isDiscard or self.player:canDiscard(enemy, dangerous)) then
				if addTarget(enemy, dangerous) then return end
			end
		end
	end

	--this part check   friend who has DelayedTrick
	self:sort(self.friends_noself, "defense")
	local friends = self:exclude(self.friends_noself, card)
	if not using_2013 then
		for _, friend in ipairs(friends) do
			if friend:hasSkill("baoyi") then continue end
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) and not friend:containsTrick("YanxiaoCard")
				and self:hasTrickEffective(card, friend) then
				local cardchosen
				tricks = friend:getJudgingArea()
				for _, trick in sgs.qlist(tricks) do
					if trick:isKindOf("Indulgence") and (not isDiscard or self.player:canDiscard(friend, trick:getId())) then
						if friend:getHp() <= friend:getHandcardNum() or friend:isLord() or name == "snatch" then
							cardchosen = trick:getEffectiveId()
							break
						end
					end
					if trick:isKindOf("SupplyShortage") and (not isDiscard or self.player:canDiscard(friend, trick:getId())) then
						cardchosen = trick:getEffectiveId()
						break
					end
					if trick:isKindOf("Indulgence") and (not isDiscard or self.player:canDiscard(friend, trick:getId())) then
						cardchosen = trick:getEffectiveId()
						break
					end
				end
				if cardchosen then
					if addTarget(friend, cardchosen) then return end
				end
			end
		end
	end

	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(card, friend) and self:needToThrowArmor(friend) and (not isDiscard or self.player:canDiscard(friend, friend:getArmor():getEffectiveId())) then
			hasLion = true
			target = friend
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			local valuable = self:getValuableCard(enemy)
			if valuable and (not isDiscard or self.player:canDiscard(enemy, valuable)) then
				if addTarget(enemy, valuable) then return end
			end
		end
	end

	local new_enemies = table.copyFrom(enemies)
	local compare_JudgingArea = function(a, b)
		return a:getJudgingArea():length() > b:getJudgingArea():length()
	end
	table.sort(new_enemies, compare_JudgingArea)
	local yanxiao_card, yanxiao_target, yanxiao_prior
	if not using_2013 then
		for _, enemy in ipairs(new_enemies) do
			for _, acard in sgs.qlist(enemy:getJudgingArea()) do
				if acard:isKindOf("YanxiaoCard") and self:hasTrickEffective(card, enemy) and (not isDiscard or self.player:canDiscard(enemy, acard:getId())) then
					yanxiao_card = acard
					yanxiao_target = enemy
					if enemy:containsTrick("indulgence") or enemy:containsTrick("supply_shortage") then yanxiao_prior = true end
					break
				end
			end
			if yanxiao_card and yanxiao_target then break end
		end
		if yanxiao_prior and yanxiao_card and yanxiao_target then
			if addTarget(yanxiao_target, yanxiao_card:getEffectiveId()) then return end
		end
	end

	for _, enemy in ipairs(enemies) do
		local cards = sgs.QList2Table(enemy:getHandcards())
		local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
		if #cards <= 2 and self:hasTrickEffective(card, enemy) and not enemy:isKongcheng() and not self:doNotDiscard(enemy, "hs", true) then
			for _, cc in ipairs(cards) do
				if (cc:hasFlag("visible") or cc:hasFlag(flag)) and (cc:isKindOf("Peach") or cc:isKindOf("Analeptic")) then
					if addTarget(enemy, self:getCardRandomly(enemy, "hs")) then return end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			if self:hasSkills("jijiu|qingnang|jieyin", enemy) then
				local cardchosen
				local equips = { enemy:getDefensiveHorse(), enemy:getArmor(), enemy:getOffensiveHorse(), enemy:getWeapon() }
				for _, equip in ipairs(equips) do
					if equip and (not enemy:hasSkill("jijiu") or equip:isRed()) and (not isDiscard or self.player:canDiscard(enemy, equip:getEffectiveId())) then
						cardchosen = equip:getEffectiveId()
						break
					end
				end

				if not cardchosen and enemy:getDefensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId())) then cardchosen = enemy:getDefensiveHorse():getEffectiveId() end
				if not cardchosen and enemy:getArmor() and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then
					cardchosen = enemy:getArmor():getEffectiveId()
				end
				if not cardchosen and not enemy:isKongcheng() and enemy:getHandcardNum() <= 3 and (not isDiscard or self.player:canDiscard(enemy, "hs")) then
					cardchosen = self:getCardRandomly(enemy, "hs")
				end

				if cardchosen then
					if addTarget(enemy, cardchosen) then return end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if self:hasTrickEffective(card, enemy) and enemy:hasArmorEffect("EightDiagram") and enemy:getArmor()
			and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then
			addTarget(enemy, enemy:getArmor():getEffectiveId())
		end
	end

	for i = 1, 2 + (isJixi and 3 or 0), 1 do
		for _, enemy in ipairs(enemies) do
			if not enemy:isNude() and self:hasTrickEffective(card, enemy)
				and not (self:needKongcheng(enemy) and i <= 2) and not self:doNotDiscard(enemy) then
				if (enemy:getHandcardNum() == i and sgs.getDefenseSlash(enemy) < 6 + (isJixi and 6 or 0) and enemy:getHp() <= 3 + (isJixi and 2 or 0)) then
					local cardchosen
					if self.player:distanceTo(enemy) == self.player:getAttackRange() + 1 and enemy:getDefensiveHorse() and not self:doNotDiscard(enemy, "e")
						and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId()))then
						cardchosen = enemy:getDefensiveHorse():getEffectiveId()
					elseif enemy:getArmor() and not self:needToThrowArmor(enemy) and not self:doNotDiscard(enemy, "e")
						and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId()))then
						cardchosen = enemy:getArmor():getEffectiveId()
					elseif not isDiscard or self.player:canDiscard(enemy, "hs") then
						cardchosen = self:getCardRandomly(enemy, "hs")
					end
					if cardchosen then
						if addTarget(enemy, cardchosen) then return end
					end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			local valuable = self:getValuableCard(enemy)
			if valuable and (not isDiscard or self.player:canDiscard(enemy, valuable)) then
				if addTarget(enemy, valuable) then return end
			end
		end
	end

	if hasLion and (not isDiscard or self.player:canDiscard(target, target:getArmor():getEffectiveId())) then
		if addTarget(target, target:getArmor():getEffectiveId()) then return end
	end

	if not using_2013
		and yanxiao_card and yanxiao_target and (not isDiscard or self.player:canDiscard(yanxiao_target, yanxiao_card:getId())) then
		if addTarget(yanxiao_target, yanxiao_card:getEffectiveId()) then return end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isKongcheng() and not self:doNotDiscard(enemy, "hs") and self:hasTrickEffective(card, enemy)
			and self:hasSkills(sgs.cardneed_skill, enemy) and (not isDiscard or self.player:canDiscard(enemy, "hs")) then
			if addTarget(enemy, self:getCardRandomly(enemy, "hs")) then return end
		end
	end

	for _, enemy in ipairs(enemies) do
		if enemy:hasEquip() and not self:doNotDiscard(enemy, "e") and self:hasTrickEffective(card, enemy) then
			local cardchosen
			if enemy:getDefensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId())) then
				cardchosen = enemy:getDefensiveHorse():getEffectiveId()
			elseif enemy:getArmor() and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then
				cardchosen = enemy:getArmor():getEffectiveId()
			elseif enemy:getOffensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getOffensiveHorse():getEffectiveId())) then
				cardchosen = enemy:getOffensiveHorse():getEffectiveId()
			elseif enemy:getWeapon() and (not isDiscard or self.player:canDiscard(enemy, enemy:getWeapon():getEffectiveId())) then
				cardchosen = enemy:getWeapon():getEffectiveId()
			end
			if cardchosen then
				if addTarget(enemy, cardchosen) then return end
			end
		end
	end

	if name == "snatch" or self:getOverflow() > 0 then
		for _, enemy in ipairs(enemies) do
			local equips = enemy:getEquips()
			if not enemy:isNude() and self:hasTrickEffective(card, enemy) and not self:doNotDiscard(enemy, "hes") then
				local cardchosen
				if not equips:isEmpty() and not self:doNotDiscard(enemy, "e") then
					cardchosen = self:getCardRandomly(enemy, "e")
				else
					cardchosen = self:getCardRandomly(enemy, "hs") end
				if cardchosen then
					if addTarget(enemy, cardchosen) then return end
				end
			end
		end
	end
end

SmartAI.useCardSnatch = SmartAI.useCardSnatchOrDismantlement

sgs.ai_use_value.Snatch = 9
sgs.ai_use_priority.Snatch = 4.3
sgs.ai_keep_value.Snatch = 3.46

sgs.dynamic_value.control_card.Snatch = true

SmartAI.useCardDismantlement = SmartAI.useCardSnatchOrDismantlement

sgs.ai_use_value.Dismantlement = 5.6
sgs.ai_use_priority.Dismantlement = 4.4
sgs.ai_keep_value.Dismantlement = 3.44

sgs.dynamic_value.control_card.Dismantlement = true

sgs.ai_choicemade_filter.cardChosen.snatch = function(self, player, args)
	local from = findPlayerByObjectName(self.room, args[3])
	local to = findPlayerByObjectName(self.room, args[4])
	if from then
		local snatch = from:getTag("SnatchCard"):toCard()
		if snatch then
			if snatch:getSkillName() == "liyou" then return end
			if snatch:hasFlag("xunshi") then return end
		end
		local dismantlement = from:getTag("DismantlementCard"):toCard()
		if dismantlement then
			if dismantlement:hasFlag("xunshi") then return end
		end
	end
	if from and to then
		local id = tonumber(args[2])
		local place = self.room:getCardPlace(id)
		local card = sgs.Sanguosha:getCard(id)
		local intention = 70
		if to:hasSkills("tuntian+zaoxian") and to:getPile("field") == 2 and to:getMark("zaoxian") == 0 then intention = 0 end
		if place == sgs.Player_PlaceDelayedTrick then
			--×°±¸Çø»¹ÐèÒª¸ü¼ÓÏ¸»¯...
			if not card:isKindOf("Disaster") then
				intention = -intention
			else
				--¼ì²â¸ÄÅÐÕß ÈçÌì×Ó
				if not self:invokeTouhouJudge(to) then
					intention = -intention
				elseif self:getFinalRetrial(to)==0 then --Ã»ÓÐ¸ÄÅÐ
					intention = 0
				end
			end
			if card:isKindOf("YanxiaoCard") then intention = -intention end
		elseif place == sgs.Player_PlaceEquip then
			if card:isKindOf("Armor") and self:evaluateArmor(card, to) <= -2 then intention = 0 end
			if card:isKindOf("SilverLion") then
				if to:getLostHp() > 1 then
					if to:hasSkills(sgs.use_lion_skill) then
						intention = self:willSkipPlayPhase(to) and -intention or 0
					else
						intention = self:isWeak(to) and -intention or 0
					end
				else
					intention = 0
				end
			elseif to:hasSkills(sgs.lose_equip_skill) then
				if self:isWeak(to) and (card:isKindOf("DefensiveHorse") or card:isKindOf("Armor")) then
					intention = math.abs(intention)
				else
					intention = 0
				end
			end
		elseif place == sgs.Player_PlaceHand then
			if self:needKongcheng(to, true) and to:getHandcardNum() == 1 then
				intention = 0
			end
		end
		sgs.updateIntention(from, to, intention)
	end
end

sgs.ai_choicemade_filter.cardChosen.dismantlement = sgs.ai_choicemade_filter.cardChosen.snatch

function SmartAI:useCardCollateral(card, use)

	local fromList = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	local toList   = sgs.QList2Table(self.room:getAlivePlayers())

	local cmp = function(a, b)
		local alevel = self:objectiveLevel(a)
		local blevel = self:objectiveLevel(b)

		if alevel ~= blevel then return alevel > blevel end

		local anum = getCardsNum("Slash", a)
		local bnum = getCardsNum("Slash", b)

		if anum ~= bnum then return anum < bnum end
		return a:getHandcardNum() < b:getHandcardNum()
	end

	table.sort(fromList, cmp)
	self:sort(toList, "defense")

	local needCrossbow = false
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) and self:objectiveLevel(enemy) > 3
			and sgs.isGoodTarget(enemy, self.enemies, self) and not self:slashProhibit(nil, enemy) then
			needCrossbow = true
			break
		end
	end

	needCrossbow = needCrossbow and self:getCardsNum("Slash") > 2 and not self.player:hasSkill("paoxiao")

	if needCrossbow then
		for i = #fromList, 1, -1 do
			local friend = fromList[i]
			if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
				and friend:getWeapon() and friend:getWeapon():isKindOf("Crossbow") and self:hasTrickEffective(card, friend) then
				for _, enemy in ipairs(toList) do
					if friend:canSlash(enemy, nil) and friend:objectName() ~= enemy:objectName() then
						self.room:setPlayerFlag(self.player, "needCrossbow")
						use.card = card
						if use.to then use.to:append(friend) end
						if use.to then use.to:append(enemy) end
						return
					end
				end
			end
		end
	end

	local n = nil
	local final_enemy = nil
	for _, enemy in ipairs(fromList) do
		local collateral_to_flandre_as_first_target = true
		if enemy:hasSkill("wosui") and self.player:getHp() >= enemy:getHp() then
			if getKnownCard(enemy, self.player, "Slash", true) > 0 then collateral_to_flandre_as_first_target = false end
			if not enemy:isKongcheng() or enemy:getPile("wooden_ox"):length() > 0 then
				local all_num = enemy:getHandcardNum() + enemy:getPile("wooden_ox"):length()
				local visible_num = 0
				local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
				for _, c in sgs.qlist(enemy:getHandcards()) do
					if (c:hasFlag("visible") or c:hasFlag(flag)) and not c:isKindOf("Slash") then
						visible_num = visible_num + 1
					end
				end
				for _, id in sgs.qlist(enemy:getPile("wooden_ox")) do
					local c = sgs.Sanguosha:getCard(id)
					if (c:hasFlag("visible") or c:hasFlag(flag)) and not c:isKindOf("Slash") then
						visible_num = visible_num + 1
					end
				end
				if visible_num < all_num then collateral_to_flandre_as_first_target = false end
			end
		end
		if not collateral_to_flandre_as_first_target then continue end
		if (not use.current_targets or not table.contains(use.current_targets, enemy:objectName()))
			and self:hasTrickEffective(card, enemy)
			and not self:hasSkills(sgs.lose_equip_skill, enemy)
			and not (enemy:hasSkill("weimu") and card:isBlack())
			and not (enemy:hasSkill("tuntian") and enemy:hasSkill("zaoxian"))
			and self:objectiveLevel(enemy) >= 0
			and enemy:getWeapon() then

			for _, enemy2 in ipairs(toList) do
				if enemy:canSlash(enemy2) and self:objectiveLevel(enemy2) > 3 and enemy:objectName() ~= enemy2:objectName() then
					n = 1
					final_enemy = enemy2
					break
				end
			end

			if not n then
				for _, enemy2 in ipairs(toList) do
					if enemy:canSlash(enemy2) and self:objectiveLevel(enemy2) <=3 and self:objectiveLevel(enemy2) >=0 and enemy:objectName() ~= enemy2:objectName() then
						n = 1
						final_enemy = enemy2
						break
					end
				end
			end

			if not n then
				for _, friend in ipairs(toList) do
					if enemy:canSlash(friend) and self:objectiveLevel(friend) < 0 and enemy:objectName() ~= friend:objectName()
							and (self:needToLoseHp(friend, enemy, true, true) or self:getDamagedEffects(friend, enemy, true)) then
						n = 1
						final_enemy = friend
						break
					end
				end
			end

			if not n then
				for _, friend in ipairs(toList) do
					if enemy:canSlash(friend) and self:objectiveLevel(friend) < 0 and enemy:objectName() ~= friend:objectName()
							and (getKnownCard(friend, self.player, "Jink", true, "hes") >= 2 or getCardsNum("Slash", enemy) < 1) then
						n = 1
						final_enemy = friend
						break
					end
				end
			end

			if n then
				use.card = card
				if use.to then use.to:append(enemy) end
				if use.to then use.to:append(final_enemy) end
				return
			end
		end
		n = nil
	end

	for _, friend in ipairs(fromList) do
		if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
			and friend:getWeapon() and (getKnownCard(friend, self.player, "Slash", true, "hes") > 0 or getCardsNum("Slash", friend) > 1 and friend:getHandcardNum() >= 4)
			and self:hasTrickEffective(card, friend)
			and self:objectiveLevel(friend) < 0
			and not self.room:isProhibited(self.player, friend, card) then

			for _, enemy in ipairs(toList) do
				if friend:canSlash(enemy, nil) and self:objectiveLevel(enemy) > 3 and friend:objectName() ~= enemy:objectName()
						and sgs.isGoodTarget(enemy, self.enemies, self) and not self:slashProhibit(nil, enemy) then
					use.card = card
					if use.to then use.to:append(friend) end
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
	end

	self:sortEnemies(toList)

	for _, friend in ipairs(fromList) do
		if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
			and friend:getWeapon() and friend:hasSkills(sgs.lose_equip_skill)
			and self:hasTrickEffective(card, friend)
			and self:objectiveLevel(friend) < 0
			and not (friend:getWeapon():isKindOf("Crossbow") and getCardsNum("Slash", friend) > 1)
			and not self.room:isProhibited(self.player, friend, card) then

			for _, enemy in ipairs(toList) do
				if friend:canSlash(enemy, nil) and friend:objectName() ~= enemy:objectName() then
					use.card = card
					if use.to then use.to:append(friend) end
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
	end
end

sgs.ai_use_value.Collateral = 5.8
sgs.ai_use_priority.Collateral = 2.75
sgs.ai_keep_value.Collateral = 3.40

sgs.ai_card_intention.Collateral = function(self,card, from, tos)
	assert(#tos == 1)
	sgs.ai_collateral = true
end

sgs.dynamic_value.control_card.Collateral = true

sgs.ai_skill_cardask["collateral-slash"] = function(self, data, pattern, target2, target, prompt)
	-- self.player = killer
	-- target = user
	-- target2 = victim

	--step1: check canSlash


	local slashes = {}
	for _, slash in ipairs(self:getCards("Slash")) do
		local range_fix = countRangeFix(slash, self.player)
		local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50 or self.player:hasFlag("slashNoDistanceLimit")
		if self.player:canSlash(target2, slash, not no_distance, range_fix) then
			table.insert(slashes, slash)
		end
	end
	if #slashes == 0 then  return "." end

	local current = self.room:getCurrent()
	if self:isFriend(current) and (current:hasFlag("needCrossbow") or
			(getCardsNum("Slash", current, self.player) >= 2 and self.player:getWeapon():isKindOf("Crossbow"))) then
		if current:hasFlag("needCrossbow") then self.room:setPlayerFlag(current, "-needCrossbow") end
		return "."
	end

	if self:isFriend(target2) and self:needLeiji(target2, self.player) then
		for _, slash in ipairs(slashes) do
			if  self:slashIsEffective(slash, target2) then
				return slash:toString()
			end
		end
	end

	if target2 and (self:getDamagedEffects(target2, self.player, true) or self:needToLoseHp(target2, self.player, true)) then
		for _, slash in ipairs(slashes) do
			if  self:slashIsEffective(slash, target2) and self:isFriend(target2) then
				return slash:toString()
			end
			if not self:slashIsEffective(slash, target2, self.player, true) and self:isEnemy(target2) then
				return slash:toString()
			end
		end
		for _, slash in ipairs(slashes) do
			if  not self:getDamagedEffects(target2, self.player, true) and self:isEnemy(target2) then
				return slash:toString()
			end
		end
	end

	if target2 and not self:hasSkills(sgs.lose_equip_skill) and self:isEnemy(target2) then
		for _, slash in ipairs(slashes) do
			if  self:slashIsEffective(slash, target2) then
				return slash:toString()
			end
		end
	end
	if target2 and not self:hasSkills(sgs.lose_equip_skill) and self:isFriend(target2) then
		for _, slash in ipairs(slashes) do
			if  not self:slashIsEffective(slash, target2) then
				return slash:toString()
			end
		end
		for _, slash in ipairs(slashes) do
			if  (target2:getHp() > 3 or not self:canHit(target2, self.player, self:hasHeavySlashDamage(self.player, slash, target2)))
				and not target2:getRole() == "lord" and self.player:getHandcardNum() > 1 then
					return slash:toString()
			end
			if self:needToLoseHp(target2, self.player) then return slash:toString() end
		end
	end
	self:speak("collateral", self.player:isFemale())
	return "."
end

SmartAI.useCardHaze = SmartAI.useCardSnatchOrDismantlement

function removeElementByKey(tbl, key)
	local tmp = {}

	for i in pairs(tbl) do
		table.insert(tmp, i)
	end

	local newTbl = {}
	local i = 1
	while i <= #tmp do
		local val = tmp [i]
		if val == key then
			table.remove(tmp, i)
		else
			newTbl[val] = tbl[val]
			i = i + 1
		end
	end
	return newTbl
end

sgs.ai_skill_choice.haze = function(self, choices)
	local target = self.room:getTag("HazeTarget"):toPlayer()
	local suits = {"heart", "diamond", "spade", "club"}
	local has_suit = {}
	for _, c in sgs.qlist(target:getEquips()) do
		if not table.contains(has_suit, c:getSuitString()) then
			table.insert(has_suit, c:getSuitString())
		end
	end
	for _, c in ipairs(self:getVisibleCards(target)) do
		if not table.contains(has_suit, c:getSuitString()) then
			table.insert(has_suit, c:getSuitString())
		end
	end
	for _, suit in ipairs(suits) do
		if not table.contains(has_suit, suit) then
			return suit
		end
	end

	for _, c in sgs.qlist(target:getEquips()) do
		if table.contains(self:getDefensiveKeyEquips(target), c) then
			return c:getSuitString()
		end
	end
	for _, c in sgs.qlist(target:getEquips()) do
		if table.contains(self:getOffensiveKeyEquips(target), c) then
			return c:getSuitString()
		end
	end

	local cards = self:getVisibleCards(target)
	if #cards > 0 then
		self:sortByKeepValue(cards)
		return cards[1]:getSuitString()
	end

	return "heart"
end

sgs.ai_skill_cardask["@haze"] = function(self, data, pattern, target)
	if not self:isEnemy(target) and self:hasJudges() then
		return "."
	end
	
	local cards = sgs.QList2Table(self.player:getCards("he"))
	local convert = { [".|spade"] = "spade", [".|diamond"] = "diamond", [".|heart"] = "heart", [".|club"] = "club"}
	
	if #cards == 1 and cards[1]:getSuitString() == convert[pattern] then
		return cards[1]:getId()
	end
	
	local card
	for _, c in ipairs(cards) do
		if c:getSuitString() == convert[pattern] then
			if not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
					and not (c:isKindOf("Jink") and self:getCardsNum("Jink") <= 1)
					and not (c:isKindOf("Analeptic") and self:isVeryWeak())
					and not c:isKindOf("ExNihilo") then
				return c:getId()
			else
				if #cards <= 3 and ((self:getCardsNum("Jink") + self:getCardsNum("Peach") == #cards)
						or self:getCardsNum("Peach") + self:getCardsNum("Jink") + self:getCardsNum("Analeptic") == #cards
						and self:isWeak()) and (not c:isKindOf("Peach") or self:getCardsNum("Peach") == #cards)then
					return c:getId()
				end
			end
		end
	end
	
	return "."
end

sgs.ai_choicemade_filter.cardChosen.haze = sgs.ai_choicemade_filter.cardChosen.snatch

sgs.ai_use_value.Haze = 8.9
sgs.ai_use_priority.Haze = 4.6
sgs.ai_keep_value.Haze = 3.55

sgs.dynamic_value.control_card.Haze = true

function SmartAI:useCardMindReading(mind_reading, use)
	if self.player:hasSkill("noswuyan") then return end
	
	local targets = {}
	local max_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, mind_reading)
	
	self:sort(self.enemies, "chaofeng")
	for _, p in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, p, mind_reading) and mind_reading:targetFilter(sgs.PlayerList(), p, self.player) then
			table.insert(targets, p)
		end
	end
	
	if #targets >= max_num then
		use.card = mind_reading
		self:sort(targets, "handcard", true)
		for _, p in ipairs(targets) do
			if use.to then
				if use.to:length() < max_num then
					use.to:append(p)
				end
				if use.to:length() == max_num then return end
			end
		end
	end
	
	self:sort(self.friends_noself, "chaofeng")
	for _, p in ipairs(self.friends_noself) do
		if not self.room:isProhibited(self.player, p, mind_reading) and mind_reading:targetFilter(sgs.PlayerList(), p, self.player) then
			table.insert(targets, p)
		end
	end
	
	if #targets > 0 then
		if max_num > #targets then max_num = #targets end
		for _, p in ipairs(targets) do
			if use.to then
				if use.to:length() < max_num then
					use.to:append(p)
				end
				if use.to:length() == max_num then return end
			end
		end
	end
end

sgs.ai_use_value.MindReading = 7.7
sgs.ai_use_priority.MindReading = 8.5
sgs.ai_keep_value.MindReading = 3.22

sgs.dynamic_value.control_card.MindReading = true

function SmartAI:useCardIcyFog(icy_fog, use)
	if self.player:hasSkill("noswuyan") then return end
	
	local targets = {}
	local max_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, icy_fog)
	
	self:sort(self.friends_noself, "hp")
	for _, p in ipairs(self.friends_noself) do
		if self:hasLion(p) and p:isWounded() and not self.room:isProhibited(self.player, p, icy_fog) and icy_fog:targetFilter(sgs.PlayerList(), p, self.player) then
			table.insert(targets, p)
		end
	end
	
	if #targets >= max_num then
		use.card = icy_fog
		self:sort(targets, "hp")
		for _, p in ipairs(targets) do
			if use.to then
				if use.to:length() < max_num then
					sgs.updateIntention(self.player, p, -80)
					use.to:append(p)
				end
				if use.to:length() == max_num then return end
			end
		end
	end
	
	self:sort(self.enemies, "chaofeng")
	for _, p in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, p, icy_fog) and icy_fog:targetFilter(sgs.PlayerList(), p, self.player)
				and not (p:getEquips():length() == 1 and self:hasLion(p)) then
			table.insert(targets, p)
		end
	end
	
	if #targets > 0 then
		if #targets < max_num then max_num = #targets end
		use.card = icy_fog
		for _, p in ipairs(targets) do
			if use.to then
				if use.to:length() < max_num then
					if self:isFriend(p) then
						sgs.updateIntention(self.player, p, -80)
					else
						sgs.updateIntention(self.player, p, 80)
					end
					use.to:append(p)
				end
				if use.to:length() == max_num then return end
			end
		end
	end
end

sgs.ai_use_priority.IcyFog = 4.4
sgs.ai_use_value.IcyFog = 7.7
sgs.ai_keep_value.IcyFog = 3.26

sgs.dynamic_value.control_card.IcyFog = true

local function hp_subtract_handcard(a,b)
	local diff1 = a:getHp() - a:getHandcardNum()
	local diff2 = b:getHp() - b:getHandcardNum()

	return diff1 < diff2
end

function SmartAI:enemiesContainsTrick(EnemyCount)
	local trick_all, possible_indul_enemy, possible_ss_enemy = 0, 0, 0
	local indul_num = self:getCardsNum("Indulgence")
	local ss_num = self:getCardsNum("SupplyShortage")
	local enemy_num, temp_enemy = 0

	local zhanghe = self.room:findPlayerBySkillName("qiaobian")
	if zhanghe and (not self:isEnemy(zhanghe) or zhanghe:isKongcheng() or not zhanghe:faceUp()) then zhanghe = nil end

	if self.player:hasSkill("guose") then
		for _, acard in sgs.qlist(self.player:getCards("hes")) do
			if acard:getSuit() == sgs.Card_Diamond then indul_num = indul_num + 1 end
		end
	end

	if self.player:hasSkill("duanliang") then
		for _, acard in sgs.qlist(self.player:getCards("hes")) do
			if acard:isBlack() then ss_num = ss_num + 1 end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if not enemy:containsTrick("YanxiaoCard") then
			if enemy:containsTrick("indulgence") then
				if not enemy:hasSkills("keji|conghui") and  (not zhanghe or self:playerGetRound(enemy) >= self:playerGetRound(zhanghe)) then
					trick_all = trick_all + 1
					if not temp_enemy or temp_enemy:objectName() ~= enemy:objectName() then
						enemy_num = enemy_num + 1
						temp_enemy = enemy
					end
				end
			else
				possible_indul_enemy = possible_indul_enemy + 1
			end
			if self.player:distanceTo(enemy) == 1 or self.player:hasSkill("duanliang") and self.player:distanceTo(enemy) <= 2 then
				if enemy:containsTrick("supply_shortage") then
					if not self:hasSkills("shensu|jisu", enemy) and (not zhanghe or self:playerGetRound(enemy) >= self:playerGetRound(zhanghe)) then
						trick_all = trick_all + 1
						if not temp_enemy or temp_enemy:objectName() ~= enemy:objectName() then
							enemy_num = enemy_num + 1
							temp_enemy = enemy
						end
					end
				else
					possible_ss_enemy  = possible_ss_enemy + 1
				end
			end
		end
	end
	indul_num = math.min(possible_indul_enemy, indul_num)
	ss_num = math.min(possible_ss_enemy, ss_num)
	if not EnemyCount then
		return trick_all + indul_num + ss_num
	else
		return enemy_num + indul_num + ss_num
	end
end

function SmartAI:playerGetRound(player, source)
	if not player then return self.room:writeToConsole(debug.traceback()) end
	source = source or self.room:getCurrent()
	if player:objectName() == source:objectName() then return 0 end
	local players_num = self.room:alivePlayerCount()
	local round = (player:getSeat() - source:getSeat()) % players_num
	return round
end

function SmartAI:useCardIndulgence(card, use)
	local enemies = {}

	if #self.enemies == 0 then
		if sgs.turncount <= 1 and self.role == "lord" and not sgs.isRolePredictable()
			and sgs.evaluatePlayerRole(self.player:getNextAlive()) == "neutral"
			and not (self.player:hasLordSkill("shichou") and self.player:getNextAlive():getKingdom() == "shu") then
			enemies = self:exclude({self.player:getNextAlive()}, card)
		end
	else
		enemies = self:exclude(self.enemies, card)
	end

	local zhanghe = self.room:findPlayerBySkillName("qiaobian")
	local zhanghe_seat = zhanghe and zhanghe:faceUp() and not zhanghe:isKongcheng() and not self:isFriend(zhanghe) and zhanghe:getSeat() or 0

	local sb_daqiao = self.room:findPlayerBySkillName("yanxiao")
	local yanxiao = sb_daqiao and not self:isFriend(sb_daqiao) and sb_daqiao:faceUp() and
					(getKnownCard(sb_daqiao, self.player, "diamond", nil, "hes") > 0
					or sb_daqiao:getHandcardNum() + self:ImitateResult_DrawNCards(sb_daqiao, sb_daqiao:getVisibleSkillList()) > 3
					or sb_daqiao:containsTrick("YanxiaoCard"))
	--¶«·½É±ÖÐÀàÕÅàAµÄ½ÇÉ«
	local mouko =self.room:findPlayerBySkillName("sidou")
	local mouko_seat = mouko and mouko:faceUp()  and not self:isFriend(mouko) and mouko:getSeat() or 0
	local tiger = self.room:findPlayerBySkillName("jinghua")
	local tiger_seat = tiger and tiger:faceUp()  and not self:isFriend(tiger) and tiger:getSeat() or 0
	local marisa = self.room:findPlayerBySkillName("jiezou")
	local marisa_seat = marisa and card:getSuit()==sgs.Card_Spade and marisa:faceUp()  and not self:isFriend(marisa) and marisa:getSeat() or 0



	local getvalue = function(enemy)
		if self:touhouDelayTrickBadTarget(card, enemy, self.player) then return -100 end
		if enemy:containsTrick("indulgence") or enemy:containsTrick("YanxiaoCard") then return -100 end
		if enemy:hasSkill("qiaobian") and not enemy:containsTrick("supply_shortage") and not enemy:containsTrick("indulgence") then return -100 end
		if zhanghe_seat > 0 and (self:playerGetRound(zhanghe) <= self:playerGetRound(enemy) and self:enemiesContainsTrick() <= 1 or not enemy:faceUp()) then
			return -100 end
		if mouko_seat > 0 and (self:playerGetRound(mouko) <= self:playerGetRound(enemy) and self:enemiesContainsTrick() <= 1 or not enemy:faceUp()) then
			return -100 end
		if tiger_seat > 0 and (self:playerGetRound(tiger) <= self:playerGetRound(enemy) and self:enemiesContainsTrick() <= 1 or not enemy:faceUp()) then
			return -100 end
		if marisa_seat > 0 and (self:playerGetRound(marisa) < self:playerGetRound(enemy) and self:enemiesContainsTrick() <= 1 or not enemy:faceUp()) then
			return - 100 end
		if yanxiao and (self:playerGetRound(sb_daqiao) <= self:playerGetRound(enemy) and self:enemiesContainsTrick(true) <= 1 or not enemy:faceUp()) then
			return -100 end

		local value = enemy:getHandcardNum() - enemy:getHp()

		if enemy:hasSkills("noslijian|lijian|fanjian|neofanjian|dimeng|jijiu|jieyin|anxu|yongsi|zhiheng|manjuan|nosrende|rende") then value = value + 10 end
		if enemy:hasSkills("houyuan|qixi|qice|guose|duanliang|yanxiao|nosjujian|luoshen|nosjizhi|jizhi|jilve|wansha|mingce|sizhan") then value = value + 5 end
		if enemy:hasSkills("guzheng|luoying|xiliang|guixin|lihun|yinling|gongxin|shenfen|ganlu|duoshi|jueji|zhenggong") then value = value + 3 end
		
		--¶«·½ÆôÁé¸ó¡¤¼¼ÄÜ¼ÓÈ¨
		if enemy:hasSkills("yuzhu|huangyan|hunqu|citan|herong|yaodao|huawu|guiqiao") then value = value + 10 end
		if enemy:hasSkills("julang") then value = value + 8 end
		if enemy:hasSkills("heiyan|anji|kuaizhao|tuji|xianbo|yuanqi|jinpan|shenfeng") then value = value + 5 end
		if enemy:hasFlag("CannotDuannian") then value = value + 10 end
		if enemy:hasSkill("longzuan") then value = value - 10 end
		if self:isWeak(enemy) then value = value + 3 end
		if enemy:isLord() then value = value + 3 end

		if self:objectiveLevel(enemy) < 3 then value = value - 10 end
		if not enemy:faceUp() then value = value - 10 end
		if enemy:hasSkills("keji|shensu|conghui") then value = value - enemy:getHandcardNum() end
		if enemy:hasSkills("guanxing|xiuluo") then value = value - 5 end
		if enemy:hasSkills("lirang|longluo") then value = value - 5 end
		if enemy:hasSkills("tuxi|noszhenlie|guanxing|qinyin|zongshi|tiandu") then value = value - 3 end
		if enemy:hasSkill("conghui") then value = value - 20 end
		if self:needBear(enemy) then value = value - 20 end
		--if self:needChuixue(enemy) then
		if self:touhouNeedBear(card) then value = value - 20 end
		if not sgs.isGoodTarget(enemy, self.enemies, self) then value = value - 1 end
		return value
	end

	--ÄãÃÃ Õâ¸öÇóÖµ¸ù±¾²»¿¼ÂÇ×ù´Î¼ÓÈ¨¡£¡£¡£Öµ¸ßÁË Ö±½ÓÀÖÉÏ¼Ò£¬
--¸ôÁË6£¬7¸öÈË»¹ÏëÀÖÖÐ£¿£¿£¿£¿£¿£¿£¿¸÷ÖÖ²ð½Ì×öÈË ±ØÐë×ùÎ»¼ÓÈ¨
--½øÒ»²½¸ÃÉý¼¶¿¼Á¿»ØºÏÄÚÍâµÄÄÜÁ¦
--¿¼Á¿ÕÒµ½ÀÖ²ðµÄÄÜÁ¦
	local getSeatWeight = function(enemies,enemy)
		--local weight= #enemies
		local weight = 0
		for _,s in pairs(enemies) do
			if (self:playerGetRound(s) < self:playerGetRound(enemy)) then
				weight=weight-10
			end
		end
		return weight
	end

	local cmp = function(a,b)
		return (getvalue(a)+getSeatWeight(enemies,a)) >  (getvalue(b)+getSeatWeight(enemies,b))
	end
	if #enemies > 0 then
		table.sort(enemies, cmp)

		local target = enemies[1]
		if getvalue(target) > -100 then
			use.card = card
			if use.to then use.to:append(target) end
			return
		end
	end
	--use to friends
	local friends  = self:exclude(self.friends_noself, card)
	local friendNames ={}
	for _,p in pairs(friends) do
		table.insert(friendNames,p:objectName())
	end
	local baoyi = self.room:findPlayerBySkillName("baoyi")

	if baoyi and self:isFriend(baoyi) and table.contains(friendNames,baoyi:objectName())  then
		use.card = card
		if use.to then use.to:append(baoyi) end
		return
	end
end

sgs.ai_use_value.Indulgence = 8
sgs.ai_use_priority.Indulgence = 0.5
--sgs.ai_card_intention.Indulgence = 120
sgs.ai_card_intention.Indulgence = function(self, card, from, tos)
	if not tos[1]:hasSkill("baoyi") then
		sgs.updateIntentions(from, tos, 120)
	end
end
sgs.ai_keep_value.Indulgence = 3.5
sgs.ai_judge_model.indulgence = function(self, who)
	local judge = sgs.JudgeStruct()
	judge.who = who
	judge.pattern = ".|heart"
	judge.good = true
	judge.reason = "indulgence"
	return judge
end

sgs.dynamic_value.control_usecard.Indulgence = true

--¶«·½É±Ïà¹Ø
--¡¾ÃüÔË¡¿¡¾ç³Ïë¡¿
function SmartAI:willUseLightning(card)
	if not card then self.room:writeToConsole(debug.traceback()) return false end
	if self.player:containsTrick("lightning") then return end
	if self.player:hasSkill("weimu") and card:isBlack() then return end
	if self.room:isProhibited(self.player, self.player, card) then return end

	local rebel_num = sgs.current_mode_players["rebel"]
	local avoidLightning ="jingdian|bingpo|bumie"
	local retrialType, wizarder= self:getFinalRetrial(self.player)
	--ÖÒÄÚ²Ð¾ÖÊ¹ÓÃÉÁµçµÄÔ­Ôò ¶ÀÁ¢ÓÚÒ»°ãÊ±ºò
	if rebel_num==0 and (self.player:getRole() == "loyalist" or self.player:isLord()) then
		if wizarder and wizarder:isLord() then
			return true
		elseif not wizarder and self.room:getLord():hasSkill("jingdian") then
			return true
		else
			return false
		end
	end


	local function hasDangerousFriend()
		local hashy = false
		for _, aplayer in ipairs(self.enemies) do
			if aplayer:hasSkill("hongyan") then hashy = true break end
		end
		for _, aplayer in ipairs(self.enemies) do
			if aplayer:hasSkill("guanxing") or (aplayer:hasSkill("gongxin") and hashy)
			or aplayer:hasSkill("xinzhan") or aplayer:hasSkill("tianyan") or aplayer:hasSkill("shenmi")  then
				if self:isFriend(aplayer:getNextAlive()) then return true end
			end
		end
		return false
	end
	--¾²µç¹û¶Ï¹Ò ÎÞÄîµÈÒ²¿ÉÒÔ?
	if self.player:hasSkill("jingdian") then
		return retrialType ==1 or  not wizarder
	end
	if retrialType == 2 then
	return
	elseif retrialType == 1 then
		return true
	elseif not hasDangerousFriend() then
		local players = self.room:getAllPlayers()
		players = sgs.QList2Table(players)

		local friends = 0
		local enemies = 0
		for _,player in ipairs(players) do
			if self:objectiveLevel(player) >= 4
			  and not (player:hasSkill("weimu") and card:isBlack())
			  and not player:hasSkills(avoidLightning) then
				enemies = enemies + 1
			elseif self:isFriend(player)
			  and not (player:hasSkill("weimu") and card:isBlack())
			  and not player:hasSkills(avoidLightning) then
				friends = friends + 1
			end
		end

		local ratio

		if friends == 0 then ratio = 999
		else ratio = enemies/friends
		end

		if ratio > 1.5 then
			return true
		end
	end
end

function SmartAI:useCardLightning(card, use)
	if self:willUseLightning(card) then
		use.card = card
	end
end

sgs.ai_use_priority.Lightning = 0
sgs.dynamic_value.lucky_chance.Lightning = true
sgs.ai_judge_model.lightning = function(self, who)
	local judge = sgs.JudgeStruct()
	judge.who = who
	judge.pattern = ".|spade|2~9"
	judge.good = false
	judge.reason = "lightning"
	return judge
end
sgs.ai_keep_value.Lightning = -1

--¶«·½É±Ïà¹Ø
--²Ð¾ÖÄÚ¼é¹ÒµçÃ»³ðºÞÌ«¹ý·ÖÁË
sgs.ai_card_intention.Lightning = function(self, card, from, to)
	local rebel_num = sgs.current_mode_players["rebel"]
	local loyalist_num=self.room:getAlivePlayers():length()-rebel_num
	local lord=getLord(self.player)
	if rebel_num<= loyalist_num-1 then
		if lord and not self:touhouHasLightningBenefit(lord) then
			if not from:isLord() then
				sgs.updateIntention(from, lord, 50)
				return 0
			end
		end
	end
	local retrialType, wizarder= self:getFinalRetrial(from)
	--[[local wizard={}
	for _, aplayer in sgs.qlist(self.room:getOtherPlayers(from)) do
		if self:hasSkills(sgs.wizard_harm_skill .. "|huanshi", aplayer) then
			table.insert(wizard,aplayer)
		end
	end]]
	if wizarder then
		sgs.updateIntention(from, wizarder, -50)
	end
	return 0
end

sgs.ai_skill_askforag.amazing_grace = function(self, card_ids)

	local NextPlayerCanUse, NextPlayerisEnemy
	local NextPlayer = self.player:getNextAlive()
	if sgs.turncount > 1 and not self:willSkipPlayPhase(NextPlayer) then
		if self:isFriend(NextPlayer) and sgs.evaluatePlayerRole(NextPlayer) ~= "neutral" then
			NextPlayerCanUse = true
		else
			NextPlayerisEnemy = true
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasSkill("lihun") and enemy:faceUp() and not NextPlayer:faceUp() and NextPlayer:getHandcardNum() > 4 and NextPlayer:isMale() then
			NextPlayerCanUse = false
		end
	end

	local cards = {}
	local trickcard = {}
	for _, card_id in ipairs(card_ids) do
		local acard = sgs.Sanguosha:getCard(card_id)
		table.insert(cards, acard)
		if acard:isKindOf("TrickCard") then
			table.insert(trickcard , acard)
		end
	end

	local nextfriend_num = 0
	local aplayer = self.player:getNextAlive()
	for i =1, self.player:aliveCount() do
		if self:isFriend(aplayer) then
			aplayer = aplayer:getNextAlive()
			nextfriend_num = nextfriend_num + 1
		else
			break
		end
	end

	local SelfisCurrent
	if self.room:getCurrent():objectName() == self.player:objectName() then SelfisCurrent = true end

---------------

	local needbuyi
	for _, friend in ipairs(self.friends) do
		if friend:hasSkill("buyi") and self.player:getHp() == 1 then
			needbuyi = true
		end
	end
	if needbuyi then
		local maxvaluecard, minvaluecard
		local maxvalue, minvalue = -100, 100
		for _, bycard in ipairs(cards) do
			if not bycard:isKindOf("BasicCard") then
				local value = self:getUseValue(bycard)
				if value > maxvalue then
					maxvalue = value
					maxvaluecard = bycard
				end
				if value < minvalue then
					minvalue = value
					minvaluecard = bycard
				end
			end
		end
		if minvaluecard and NextPlayerCanUse then
			return minvaluecard:getEffectiveId()
		end
		if maxvaluecard then
			return maxvaluecard:getEffectiveId()
		end
	end

	local friendneedpeach, peach
	local peachnum, jinknum = 0, 0
	if NextPlayerCanUse then
		if (not self.player:isWounded() and NextPlayer:isWounded()) or
			(self.player:getLostHp() < self:getCardsNum("Peach")) or
			(not SelfisCurrent and self:willSkipPlayPhase() and self.player:getHandcardNum() + 2 > self.player:getMaxCards()) then
			friendneedpeach = true
		end
	end
	for _, card in ipairs(cards) do
		if isCard("Peach", card, self.player) then
			peach = card:getEffectiveId()
			peachnum = peachnum + 1
		end
		if card:isKindOf("Jink") then jinknum = jinknum + 1 end
	end
	if (not friendneedpeach and peach) or peachnum > 1 then return peach end

	local exnihilo, jink, analeptic, nullification, snatch, dismantlement, oracle
	for _, card in ipairs(cards) do
		if isCard("ExNihilo", card, self.player) then
			if not NextPlayerCanUse or (not self:willSkipPlayPhase() and (self.player:hasSkills("nosjizhi|jizhi|zhiheng|nosrende|rende") or not NextPlayer:hasSkills("nosjizhi|jizhi|zhiheng|nosrende|rende"))) then
				exnihilo = card:getEffectiveId()
			end
		elseif isCard("Jink", card, self.player) then
			jink = card:getEffectiveId()
		elseif isCard("Analeptic", card, self.player) then
			analeptic = card:getEffectiveId()
		elseif isCard("Nullification", card, self.player) then
			nullification = card:getEffectiveId()
		elseif isCard("Snatch", card, self.player) then
			snatch = card
		elseif isCard("Dismantlement", card, self.player) then
			dismantlement = card
		elseif isCard("Oracle", card, self.player) then
			oralce = card
		end

	end

	for _, target in sgs.qlist(self.room:getAlivePlayers()) do
		if self:willSkipPlayPhase(target) or self:willSkipDrawPhase(target) then
			if nullification then return nullification
			elseif self:isFriend(target) and snatch and self:hasTrickEffective(snatch, target, self.player) and
				not self:willSkipPlayPhase() and self.player:distanceTo(target) == 1 then
				return snatch:getEffectiveId()
			elseif self:isFriend(target) and dismantlement and self:hasTrickEffective(dismantlement, target, self.player) and
				not self:willSkipPlayPhase() and self.player:objectName() ~= target:objectName() then
				return dismantlement:getEffectiveId()
			elseif self:isFriend(target) and oracle and self:isWounded(target) and self:hasTrickEffective(oracle, target, self.player) and
				not self:willSkipPlayPhase() and self.player:objectName() ~= target:objectName() then
				return oracle:getEffectiveId()
			end
		end
	end

	if SelfisCurrent then
		if exnihilo then return exnihilo end
		if (jink or analeptic) and (self:getCardsNum("Jink") == 0 or (self:isWeak() and self:getOverflow() <= 0)) then
			return jink or analeptic
		end
	else
		local CP = self.room:getCurrent()
		local possible_attack = 0
		for _, enemy in ipairs(self.enemies) do
			if enemy:inMyAttackRange(self.player) and self:playerGetRound(CP, enemy) < self:playerGetRound(CP, self.player) then
				possible_attack = possible_attack + 1
			end
		end
		if possible_attack > self:getCardsNum("Jink") and self:getCardsNum("Jink") <= 2 and sgs.getDefenseSlash(self.player) <= 2 then
			if jink or analeptic or exnihilo then return jink or analeptic or exnihilo end
		else
			if exnihilo then return exnihilo end
		end
	end

	if nullification and (self:getCardsNum("Nullification") < 2 or not NextPlayerCanUse) then
		return nullification
	end

	if oracle and #self.friends_noself > 0 and not (self.role == "lord" and (sgs.isLordInDanger() or self:isWeak())) then
		return oracle:getEffectiveId()
	end

	if jinknum == 1 and jink and self:isEnemy(NextPlayer) and (NextPlayer:isKongcheng() or sgs.card_lack[NextPlayer:objectName()]["Jink"] == 1) then
		return jink
	end

	if oracle and (#self.friends_noself > 0 or (NextPlayerisEnemy and not self:willSkipPlayPhase(NextPlayer) and self:hasWeakEnemy())) then
		return oracle:getEffectiveId()
	end

	self:sortByUseValue(cards)
	for _, card in ipairs(cards) do
		for _, skill in sgs.qlist(self.player:getVisibleSkillList()) do
			local callback = sgs.ai_cardneed[skill:objectName()]
			if type(callback) == "function" and callback(self.player, card, self) then
				return card:getEffectiveId()
			end
		end
	end

	local eightdiagram, silverlion, vine, renwang, ironarmor, DefHorse, OffHorse, wooden_ox
	local weapon, crossbow, halberd, double, qinggang, axe, gudingdao
	for _, card in ipairs(cards) do
		if card:isKindOf("EightDiagram") then eightdiagram = card:getEffectiveId()
		elseif card:isKindOf("SilverLion") then silverlion = card:getEffectiveId()
		elseif card:isKindOf("Vine") then vine = card:getEffectiveId()
		elseif card:isKindOf("RenwangShield") then renwang = card:getEffectiveId()
		elseif card:isKindOf("DefensiveHorse") and not self:getSameEquip(card) then DefHorse = card:getEffectiveId()
		elseif card:isKindOf("OffensiveHorse") and not self:getSameEquip(card) then OffHorse = card:getEffectiveId()
		elseif card:isKindOf("Crossbow") then crossbow = card
		elseif card:isKindOf("DoubleSword") then double = card:getEffectiveId()
		elseif card:isKindOf("QinggangSword") then qinggang = card:getEffectiveId()
		elseif card:isKindOf("Axe") then axe = card:getEffectiveId()
		elseif card:isKindOf("GudingBlade") then gudingdao = card:getEffectiveId()
		elseif card:isKindOf("Halberd") then halberd = card:getEffectiveId()
		elseif card:isKindOf("Weapon") then weapon = card:getEffectiveId()
		elseif card:isKindOf("WoodenOx") then wooden_ox = card:getEffectiveId()
		elseif card:isKindOf("IronArmor") then ironarmor = card:getEffectiveId() end
	end

	if eightdiagram then
		local lord = getLord(self.player)
		if not self:hasSkills("yizhong|bazhen") and self:hasSkills("tiandu|leiji|noszhenlie|gushou|hongyan") and not self:getSameEquip(card) then
			return eightdiagram
		end
		if NextPlayerisEnemy and self:hasSkills("tiandu|leiji|noszhenlie|gushou|hongyan", NextPlayer) and not self:getSameEquip(card, NextPlayer) then
			return eightdiagram
		end
		if self.role == "loyalist" and self.player:getKingdom()=="wei" and not self.player:hasSkill("bazhen") and
			lord and lord:hasLordSkill("hujia") and (lord:objectName() ~= NextPlayer:objectName() and NextPlayerisEnemy or lord:getArmor()) then
			return eightdiagram
		end
		if self.role == "loyalist" and self.player:getKingdom()=="zhan" and not self.player:hasSkill("bazhen") and
			lord and lord:hasLordSkill("tianren") and (lord:objectName() ~= NextPlayer:objectName() and NextPlayerisEnemy or lord:getArmor()) then
			return eightdiagram
		end
	end

	if silverlion then
		local lightning, canRetrial
		for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if aplayer:hasSkill("leiji") and self:isEnemy(aplayer) then
				return silverlion
			end
			if aplayer:containsTrick("lightning") then
				lightning = true
			end
			if self:hasSkills("guicai|guidao", aplayer) and self:isEnemy(aplayer) then
				canRetrial = true
			end
		end
		if lightning and canRetrial then return silverlion end
		if self.player:isChained() then
			for _, friend in ipairs(self.friends) do
				if friend:hasArmorEffect("Vine") and friend:isChained() then
					return silverlion
				end
			end
		end
		if self.player:isWounded() then return silverlion end
	end

	if vine then
		if sgs.ai_armor_value.Vine(self.player, self) > 0 and self.room:alivePlayerCount() <= 3 then
			return vine
		end
	end

	if renwang then
		if sgs.ai_armor_value.RenwangShield(self.player, self) > 0 and self:getCardsNum("Jink") == 0 then return renwang end
	end

	if ironarmor then
		if self.player:hasSkill("here") then return ironarmor end
		for _, enemy in ipairs(self.enemies) do
			if enemy:hasSkill("zhence") then return ironarmor end
			if enemy:hasSkill("here") then return ironarmor end
			if getCardsNum("FireAttack", enemy, self.player) > 0 then return ironarmor end
			if getCardsNum("FireSlash", enemy, self.player) > 0 then return ironarmor end
			--if enemy:getFormation():contains(self.player) and getCardsNum("BurningCamps", enemy, self.player) > 0 then return ironarmor end
		end
	end

	if DefHorse and (not self.player:hasSkill("leiji") or self:getCardsNum("Jink") == 0) then
		local before_num, after_num = 0, 0
		for _, enemy in ipairs(self.enemies) do
			if enemy:canSlash(self.player, nil, true) then
				before_num = before_num + 1
			end
			if enemy:canSlash(self.player, nil, true, 1) then
				after_num = after_num + 1
			end
		end
		if before_num > after_num and (self:isWeak() or self:getCardsNum("Jink") == 0) then return DefHorse end
	end

	if wooden_ox then
		local zhanghe = self.room:findPlayerBySkillName("qiaobian")
		local wuguotai = self.room:findPlayerBySkillName("ganlu")
		if not (zhanghe and self:isEnemy(zhanghe)) and not (wuguotai and self:isEnemy(wuguotai)) then return wooden_ox end
	end

	if analeptic then
		local slashs = self:getCards("Slash")
		for _, enemy in ipairs(self.enemies) do
			local hit_num = 0
			for _, slash in ipairs(slashs) do
				if self:slashIsEffective(slash, enemy) and self.player:canSlash(enemy, slash) and self:slashIsAvailable() then
					hit_num = hit_num + 1
					if getCardsNum("Jink", enemy) < 1
						or enemy:isKongcheng()
						or self:canLiegong(enemy, self.player)
						or self.player:hasSkills("tieji|wushuang|dahe|qianxi")
						or self.player:hasSkill("roulin") and enemy:isFemale()
						or (self.player:hasWeapon("Axe") or self:getCardsNum("Axe") > 0) and self.player:getCards("hes"):length() > 4
						then
						return analeptic
					end
				end
			end
			if (self.player:hasWeapon("Blade") or self:getCardsNum("Blade") > 0) and self:invokeTouhouJudge() then return analeptic end
			--if (self.player:hasWeapon("Blade") or self:getCardsNum("Blade") > 0) and getCardsNum("Jink", enemy) <= hit_num then return analeptic end
			if self:hasCrossbowEffect(self.player) and hit_num >= 2 then return analeptic end
		end
	end

	if weapon and (self:getCardsNum("Slash") > 0 and self:slashIsAvailable() or not SelfisCurrent) then
		local current_range = (self.player:getWeapon() and sgs.weapon_range[self.player:getWeapon():getClassName()]) or 1
		local nosuit_slash = sgs.cloneCard("slash", sgs.Card_NoSuit, 0)
		local slash = SelfisCurrent and self:getCard("Slash") or nosuit_slash

		self:sort(self.enemies, "defense")

		if crossbow then
			if #self:getCards("Slash") > 1 or self:hasSkills("kurou|keji")
				or (self:hasSkills("luoshen|yongsi|luoying|guzheng") and not SelfisCurrent and self.room:alivePlayerCount() >= 4) then
				return crossbow:getEffectiveId()
			end
			if self.player:hasSkill("guixin") and self.room:alivePlayerCount() >= 6 and (self.player:getHp() > 1 or self:getCardsNum("Peach") > 0) then
				return crossbow:getEffectiveId()
			end
			if self.player:hasSkill("rende") then
				for _, friend in ipairs(self.friends_noself) do
					if getCardsNum("Slash", friend) > 1 then
						return crossbow:getEffectiveId()
					end
				end
			end
			if self:isEnemy(NextPlayer) then
				local CanSave, huanggai, zhenji
				for _, enemy in ipairs(self.enemies) do
					if enemy:hasSkill("buyi") then CanSave = true end
					if enemy:hasSkill("jijiu") and getKnownCard(enemy, self.player, "red", nil, "hes") > 1 then CanSave = true end
					if enemy:hasSkill("chunlao") and enemy:getPile("wine"):length() > 1 then CanSave = true end
					if enemy:hasSkill("kurou") then huanggai = enemy end
					if enemy:hasSkill("keji") then return crossbow:getEffectiveId() end
					if self:hasSkills("luoshen|yongsi|guzheng", enemy) then return crossbow:getEffectiveId() end
					if enemy:hasSkill("luoying") and card:getSuit() ~= sgs.Card_Club then return crossbow:getEffectiveId() end
				end
				if huanggai then
					if huanggai:getHp() > 2 then return crossbow:getEffectiveId() end
					if CanSave then return crossbow:getEffectiveId() end
				end
				if getCardsNum("Slash", NextPlayer) >= 3 and NextPlayerisEnemy then return crossbow:getEffectiveId() end
			end
		end

		if halberd then
			if self.player:hasSkills("nosrende|rende") and self:findFriendsByType(sgs.Friend_Draw) then return halberd end
			if SelfisCurrent and self:getCardsNum("Slash") == 1 and self.player:getHandcardNum() == 1 then return halberd end
		end

		if gudingdao then
			local range_fix = current_range - 2
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, slash, true, range_fix) and enemy:isKongcheng() and not enemy:hasSkill("tianming") and
				(not SelfisCurrent or (self:getCardsNum("Dismantlement") > 0 or (self:getCardsNum("Snatch") > 0 and self.player:distanceTo(enemy) == 1))) then
					return gudingdao
				end
			end
		end

		if axe then
			local range_fix = current_range - 3
			local FFFslash = self:getCard("FireSlash")
			for _, enemy in ipairs(self.enemies) do
				if enemy:hasArmorEffect("Vine") and FFFslash and self:slashIsEffective(FFFslash, enemy) and
					self.player:getCardCount(true) >= 3 and self.player:canSlash(enemy, FFFslash, true, range_fix) then
					return axe
				elseif self:getCardsNum("Analeptic") > 0 and self.player:getCardCount(true) >= 4 and
					self:slashIsEffective(slash, enemy) and self.player:canSlash(enemy, slash, true, range_fix) then
					return axe
				end
			end
		end

		if double then
			local range_fix = current_range - 2
			for _, enemy in ipairs(self.enemies) do
				--if self.player:getGender() ~= enemy:getGender()
				local canDouble = self:touhouIsSameWithLordKingdom(self.player)~=self:touhouIsSameWithLordKingdom(enemy)
				if canDouble
				and self.player:canSlash(enemy, nil, true, range_fix) then
					return double
				end
			end
		end

		if qinggang then
			local range_fix = current_range - 2
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, slash, true, range_fix) and self:slashIsEffective(slash, enemy, self.player, true) then
					return qinggang
				end
			end
		end

	end

	local snatch, dismantlement, indulgence, doom_night, phoenix_flame, supplyshortage, collateral, duel, aoe, godsalvation, fireattack, lightning
	local new_enemies = {}
	if #self.enemies > 0 then new_enemies = self.enemies
	else
		for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if sgs.evaluatePlayerRole(aplayer) == "neutral" then
				table.insert(new_enemies, aplayer)
			end
		end
	end
	for _, card in ipairs(cards) do
		for _, enemy in ipairs(new_enemies) do
			if card:isKindOf("Snatch") and self:hasTrickEffective(card, enemy, self.player) and self.player:distanceTo(enemy) == 1 and not enemy:isNude() then
				snatch = card:getEffectiveId()
			elseif not enemy:isNude() and card:isKindOf("Dismantlement") and self:hasTrickEffective(card, enemy, self.player) then
				dismantlement = card:getEffectiveId()
			elseif card:isKindOf("Indulgence") and self:hasTrickEffective(card, enemy, self.player) and not enemy:containsTrick("indulgence") then
				indulgence = card:getEffectiveId()
			elseif card:isKindOf("DoomNight") and self:isWeak() and self.player:isWounded() and self:hasTrickEffective(card, enemy, self.player)
					and enemy:getCards("he"):length() <= 2 then
				doom_night = card:getEffectiveId()
			elseif card:isKindOf("SupplyShortage")  and self:hasTrickEffective(card, enemy, self.player) and not enemy:containsTrick("supply_shortage") then
				supplyshortage = card:getEffectiveId()
			elseif card:isKindOf("PhoenixFlame") and self:hasTrickEffective(card, enemy, self.player) and self:isWeak(enemy) and getKnownCard(enemy, self.player, "Slash", true) == 0
					and enemy:getHandcardNum() + enemy:getPile("wooden_ox"):length() <= 2 then
				phoenix_flame = card:getEffectiveId()
			elseif card:isKindOf("Collateral") and self:hasTrickEffective(card, enemy, self.player) and enemy:getWeapon()
					and table.contains(self:getKeyEquips(enemy), enemy:getWeapon()) then
				collateral = card:getEffectiveId()
			elseif card:isKindOf("Duel") and self:hasTrickEffective(card, enemy, self.player) and
					(self:getCardsNum("Slash") >= getCardsNum("Slash", enemy) or self.player:getHandcardNum() > 4) then
				duel = card:getEffectiveId()
			elseif card:isKindOf("AOE") then
				local dummy_use = {isDummy = true}
				self:useTrickCard(card, dummy_use)
				if dummy_use.card then
					aoe = card:getEffectiveId()
				end
			elseif card:isKindOf("FireAttack") and self:hasTrickEffective(card, enemy, self.player) then
				local FFF
				if enemy:getHp() == 1 or enemy:hasArmorEffect("Vine") then FFF = true end
				if FFF then
					local suits= {}
					local suitnum = 0
					for _, hcard in sgs.qlist(self.player:getHandcards()) do
						if hcard:getSuit() == sgs.Card_Spade then
							suits.spade = true
						elseif hcard:getSuit() == sgs.Card_Heart then
							suits.heart = true
						elseif hcard:getSuit() == sgs.Card_Club then
							suits.club = true
						elseif hcard:getSuit() == sgs.Card_Diamond then
							suits.diamond = true
						end
					end
					for k, hassuit in pairs(suits) do
						if hassuit then suitnum = suitnum + 1 end
					end
					if suitnum >= 3 or (suitnum >= 2 and enemy:getHandcardNum() == 1 ) then
						fireattack = card:getEffectiveId()
					end
				end
			elseif card:isKindOf("GodSalvation") and self:willUseGodSalvation(card) then
				godsalvation = card:getEffectiveId()
			elseif card:isKindOf("Lightning") and self:getFinalRetrial() == 1 then
				lightning = card:getEffectiveId()
			end
		end

		for _, friend in ipairs(self.friends_noself) do
			if (self:hasTrickEffective(card, friend) and (self:willSkipPlayPhase(friend, true) or self:willSkipDrawPhase(friend, true))) or
				self:needToThrowArmor(friend) then
				if isCard("Snatch", card, self.player) and self.player:distanceTo(friend) == 1 then
					snatch = card:getEffectiveId()
				elseif isCard("Dismantlement", card, self.player) then
					dismantlement = card:getEffectiveId()
				end
			end
		end
	end

	if snatch or dismantlement or indulgence or doom_night or supplyshortage or phoenix_flame or collateral or duel or aoe or godsalvation or fireattack or lightning then
		if not self:willSkipPlayPhase() or not NextPlayerCanUse then
			return snatch or dismantlement or indulgence or doom_night or supplyshortage or phoenix_flame or collateral or duel or aoe or godsalvation or fireattack or lightning
		end
		if #trickcard > nextfriend_num + 1 and NextPlayerCanUse then
			return lightning or fireattack or godsalvation or aoe or duel or collateral or phoenix_flame or supplyshortage or doom_night or indulgence or dismantlement or snatch
		end
	end

	if weapon and not self.player:getWeapon() and self:getCardsNum("Slash") > 0 and (self:slashIsAvailable() or not SelfisCurrent) then
		local inAttackRange
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) then
				inAttackRange = true
				break
			end
		end
		if not inAttackRange then return weapon end
	end

	if eightdiagram or silverlion or vine or renwang or ironarmor then
		return renwang or eightdiagram or ironarmor or silverlion or vine
	end

	self:sortByCardNeed(cards, true)
	for _, card in ipairs(cards) do
		if not card:isKindOf("TrickCard") and not card:isKindOf("Peach") then
			return card:getEffectiveId()
		end
	end

	return cards[1]:getEffectiveId()
end



local wooden_ox_skill = {}
wooden_ox_skill.name = "wooden_ox"
table.insert(sgs.ai_skills, wooden_ox_skill)
wooden_ox_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("WoodenOxCard") or self.player:isKongcheng() or not self.player:hasTreasure("wooden_ox")
	 then return end  --or self.player:getMark("@tianyi_Treasure") >0
	self.wooden_ox_assist = nil
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)--usevalueºÃ²»ºÃ£¿£¿ÄáÂêÁôÏÂÒ»¶ÑºìÌÒ¸øÈËÃþ
	local card, friend = self:getCardNeedPlayer(cards)
	if card and friend and friend:objectName() ~= self.player:objectName() and (self:getOverflow() > 0 or self:isWeak(friend)) then
		if  self:cautionRenegade(self.player,friend)  then
			--local target_role = sgs.evaluatePlayerRole(friend)
			--if sgs.explicit_renegade and target_role == "renegade"  then
				friend = nil
			--end
			--if not friend then
				local lord = self.room:getLord()
				if lord and self.player:objectName() ~= lord:objectName() and self:isFriend(lord) then friend = lord end
			--end
		end
		self.wooden_ox_assist = friend
		if self.wooden_ox_assist then
			return sgs.Card_Parse("@WoodenOxCard=" .. card:getEffectiveId())
		end
	end
	if self:getOverflow() > 0 or (self:needKongcheng() and #cards == 1) then
		return sgs.Card_Parse("@WoodenOxCard=" .. cards[1]:getEffectiveId())
	end
end

sgs.ai_skill_use_func.WoodenOxCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_playerchosen.wooden_ox = function(self, targets)
	if self.wooden_ox_assist then return self.wooden_ox_assist end
	if self.player:hasSkill("yongsi") then
		local kingdoms = {}
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			local kingdom = p:getKingdom()
			if not table.contains(kingdoms, kingdom) then table.insert(kingdoms, kingdom) end
		end
		if self.player:getCardCount(true) <= #kingdoms then
			self:sort(self.friends_noself)
			for _, friend in ipairs(self.friends_noself) do
				if not friend:getTreasure() then return friend end
			end
		end
	end
end

sgs.ai_playerchosen_intention.wooden_ox = -60
sgs.ai_no_playerchosen_intention.wooden_ox =function(self, from)

	local lord =self.room:getLord()
	if lord  then
		if sgs.current_mode_players["rebel"] == 0 then
			sgs.updateIntention(from, lord, 10)
		elseif not self:isWeak(from) and self:isWeak(lord)  then
			sgs.updateIntention(from, lord, 10)
		end
	end
end


--BreastPlate
sgs.ai_skill_invoke.BreastPlate = true
function sgs.ai_armor_value.BreastPlate(player, self)
	if player:hasSkills("huanmeng|bumie") then
		return 0
	end
	if player:getHp() >= 3 then
		return 2
	else
		return 5.5
	end
end

sgs.ai_use_priority.BreastPlate = 0.9

sgs.weapon_range.Triblade = 3
sgs.ai_skill_use["@@Triblade"] = function(self, prompt)
	local targets = sgs.SPlayerList()
	for _, p in sgs.qlist(self.room:getAllPlayers()) do
		if p:hasFlag("Global_TribladeFailed") then targets:append(p) end
	end

	if targets:isEmpty() then return "." end
	local id
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if not self.player:isCardLimited(c, sgs.Card_MethodDiscard) and not self:isValuableCard(c) then id = c:getEffectiveId() break end
	end
	if not id then return "." end
	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) and self:damageIsEffective(target, nil, self.player) and not self:getDamagedEffects(target, self.player)
			and not self:needToLoseHp(target, self.player) then
			--return "@TribladeSkillCard=" .. id .. "&tribladeskill->" .. target:objectName()
			return "@TribladeCard=".. id .."->" .. target:objectName()
		end
	end
	--¶ÔÓÑ¾ü²¿·ÖÔÝÊ±²»×ö
	--[[for _, target in sgs.qlist(targets) do
		if self:isFriend(target) and self:damageIsEffective(target, nil, self.player)
			and (self:getDamagedEffects(target, self.player) or self:needToLoseHp(target, self.player, nil, true)) then
			return "@TribladeSkillCard=" .. id .. "&tribladeskill->" .. target:objectName()
		end
	end]]
	return "."
end
function sgs.ai_slash_weaponfilter.Triblade(self, to, player)
	if player:distanceTo(to) > math.max(sgs.weapon_range.Triblade, player:getAttackRange()) then return end
	return sgs.card_lack[to:objectName()]["Jink"] == 1 or getCardsNum("Jink", to, self.player) == 0
end
function sgs.ai_weapon_value.Triblade(self, enemy, player)
	if not enemy then return 1 end
	if enemy and player:getHandcardNum() > 2 then return math.min(3.8, player:getHandcardNum() - 1) end
end

sgs.ai_use_priority.Triblade = 2.673

function getSidieVictim(self, targets,sidie)
	--local sidie =self.player:getTag("sidie_target"):toPlayer()
	local slash = sgs.cloneCard("slash")
	local targetlist = sgs.QList2Table(targets)
	local arrBestHp, canAvoidSlash, forbidden = {}, {}, {}
	self:sort(targetlist, "defenseSlash")

	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) and not self:slashProhibit(slash,target,sidie) and sgs.isGoodTarget(target, targetlist, self) then
			if self:slashIsEffective(slash, target,sidie) then
				if self:getDamagedEffects(target, sidie, true)  then
					--or self:needLeiji(target, self.player)
					table.insert(forbidden, target)
				elseif self:needToLoseHp(target, sidie, true, true) then
					table.insert(arrBestHp, target)
				else
					return target, "good"
				end
			else
				table.insert(canAvoidSlash, target)
			end
		end
	end
	--[[for i=#targetlist, 1, -1 do
		local target = targetlist[i]
		if not self:slashProhibit(slash, target,sidie) then
			if self:slashIsEffective(slash, target,sidie) then
				if self:isFriend(target) and (self:needToLoseHp(target, sidie, true, true)
					or self:getDamagedEffects(target, sidie, true) ) then --or self:needLeiji(target, self.player)
						--return target,"goodfriend"
				end
			else
				table.insert(canAvoidSlash, target)
			end
		end
	end]]

	if #canAvoidSlash > 0 then return canAvoidSlash[1], "canAvoid" end
	if #arrBestHp > 0 then return arrBestHp[1], "bestHp" end

	self:sort(targetlist, "defenseSlash")
	targetlist = sgs.reverse(targetlist)
	for _, target in ipairs(targetlist) do
		if target:objectName() ~= sidie:objectName()  and not self:isFriend(target) and not table.contains(forbidden, target) then
			return target, "other"
		end
	end
	return nil
end

function SmartAI:sidieEffect(from)
	if from:hasSkill("sidie") and not from:hasFlag("sidie_used") then
		return true
	end
	return false
end