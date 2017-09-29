sgs.ai_skill_use["Slash"] = function(self, prompt)
	if prompt == "@zhouneng-use" then
		if self:getCardsNum("Slash") <= 0 then return "." end
		if #self.enemies <= 0 then return "." end
		self:sort(self.enemies, "hp")
		local targets = {}
		for _, p in ipairs(self.enemies) do
			if self.player:inMyAttackRange(p) then
				table.insert(targets, p)
			end
		end
		if #targets <= 0 then return "." end
		self:sort(targets, "hp")
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, p in ipairs(targets) do
			if self:hasVine(p) then
				for _, c in ipairs(cards) do
					if c:isKindOf("FireSlash") then
						return ("fire_slash:zhouneng[%s:%s]=%d"):format(c:getSuitString(), c:getNumberString(), c:getEffectiveId()) .. "->" .. p:objectName()
					end
					if c:isKindOf("ThunderSlash") then
						return ("thunder_slash:zhouneng[%s:%s]=%d"):format(c:getSuitString(), c:getNumberString(), c:getEffectiveId()) .. "->" .. p:objectName()
					end
				end
			elseif self:hasRenwang(p) then
				for _, c in ipairs(cards) do
					if c:isKindOf("Slash") and not c:isBlack() then
						return ("slash:zhouneng[%s:%s]=%d"):format(c:getSuitString(), c:getNumberString(), c:getEffectiveId()) .. "->" .. p:objectName()
					end
				end
			else
				for _, c in ipairs(cards) do
					if c:isKindOf("Slash") then
						return ("slash:zhouneng[%s:%s]=%d"):format(c:getSuitString(), c:getNumberString(), c:getEffectiveId()) .. "->" .. p:objectName()
					end
				end
			end
		end
	end
	return "."
end

sgs.ai_skill_invoke.shuzui = function(self, data)
	local use = data:toCardUse()
	for _, p in sgs.qlist(use.to) do
		if self:isEnemy(p) and self:isVeryWeak(p) then
			return false
		end
		if not p:isWounded() or self:isFriend(p) then
			return true
		end
	end
	if self.player:getHandcardNum() + self.player:getHp() <= 3 then
		return true
	end
	if #self.enemies >= #self.friends_noself and self:getCardsNum("Jink") <= 0 then
		return true
	end
	return false
end

sgs.ai_skill_invoke.shengnian = function(self, data)
	local byakuren = self.room:findPlayerBySkillName("shengnian")
	return self:isFriend(byakuren)
end

sgs.ai_skill_playerchosen.shengnian = function(self, targets)
	local non_friends = {}
	local byakuren = self.room:findPlayerBySkillName("shengnian")
	for _, p in sgs.qlist(self.room:getOtherPlayers(byakuren)) do
		if not self:isFriend(p) then
			table.insert(non_friends, p)
		end
	end
	self:sort(non_friends, "chaofeng")
	if #non_friends > 0 then
		return non_friends[1]
	end
	return self.player
end

sgs.ai_skill_cardask["@shengnian-slash"] = function(self, data, pattern, target)
	if self:isFriend(target) then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf("Slash") then
			return c:toString()
		end
	end
	return "."
end

sgs.ai_skill_invoke.feidao = function(self, data)
	local room = self.player:getRoom()
	local use = data:toCardUse()
	local targets = sgs.SPlayerList()
	local slash = sgs.cloneCard("slash")
	for _, p in sgs.qlist(room:getOtherPlayers(self.player)) do
		if not use.to:contains(p) and self.player:inMyAttackRange(p) and self:isEnemy(p)
				and not self:slashProhibit(slash, p) and sgs.isGoodTarget(p, nil, self)
				and self:slashIsEffective(slash, p) then
			targets:append(p)
		end
	end
	if targets:isEmpty() then return false end
	return true
end

sgs.ai_skill_playerchosen.feidao = function(self, targets)
	local room = self.player:getRoom()
	local slash = room:getTag("FeidaoSlash"):toCard()
	local dests = sgs.QList2Table(targets)
	self:sort(dests, "defense")
	for _, p in ipairs(dests) do
		if self:isEnemy(p) and sgs.isGoodTarget(p, dests, self) then
			sgs.updateIntention(self.player, p, 80)
			return p
		end
	end
	return nil
end

local youdao_skill = {}
youdao_skill.name = "youdao"
table.insert(sgs.ai_skills, youdao_skill)
youdao_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("YoudaoCard") or self.player:isNude() then return end
	
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	
	local card
	if self:needToThrowArmor() then
		card = self.player:getArmor()
	else
		for _, c in ipairs(cards) do
			if not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
					and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 2)
					and not (c:isKindOf("Slash") and self:getCardsNum("Slash") < 2)
					and not (c:isKindOf("Analeptic") and self:isVeryWeak() and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 3)
					and not (c:isKindOf("ExNihilo"))
					and not (c:isKindOf("Haze")) then
				card = c
				break
			end
		end
	end
	if not card then return end
	local card_str = ("@YoudaoCard=%d"):format(card:getEffectiveId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.YoudaoCard = function(card, use, self)
	use.card = card
	return
end

sgs.ai_use_priority.YoudaoCard = 3.1
sgs.ai_use_value.YoudaoCard = 7.9
sgs.dynamic_value.benefit.YoudaoCard = true

sgs.ai_skill_invoke.thshiting = function(self, data)
	local room = self.player:getRoom()
	if self:isVeryWeak() and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") + self:getCardsNum("Jink") == 0 then
		return true
	end
	if #self.enemies == 0 then return false end
	self:sort(self.enemies, "hp")
	local has_weak = false
	for _, p in ipairs(self.enemies) do
		if self:isVeryWeak(p) and (self.player:inMyAttackRange(p) or self:getCardsNum("Duel") + self:getCardsNum("SavageAssault")
				+ self:getCardsNum("ArcheryAttack") > 0 + self:getCardsNum("Dismantlement")) then
			has_weak = true
			break
		end
	end
	if not has_weak then return false end
	local assistants = 0
	self:sort(self.enemies, "handcard", true)
	for _, p in ipairs(self.enemies) do
		if p:getHandcardNum() >= 3 and p:getHp() >= 2 then
			assistants = assistants + 1
		end
	end
	if assistants <= 1 then return true end
	return false
end

local shanchou_skill = {}
shanchou_skill.name = "shanchou"
table.insert(sgs.ai_skills, shanchou_skill)
shanchou_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ShanchouCard") or self.player:isKongcheng() then return end
	if #self.enemies == 0 then return end
	if self:getCardsNum("Peach") >= 2 then return end
	
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if (c:isKindOf("Peach") and self.player:isWounded()) or (self:getKeepValue(c) >= 8) then
			return
		end
	end
	local card_str = "@ShanchouCard=."
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.ShanchouCard = function(card, use, self)
	local room = self.room
	local handful = {}
	local handful_friend
	
	for _, p in ipairs(self.friends_noself) do
		if p:hasSkill("jisha") and getKnownCard(p, self.player, "Slash", false) > 0 then
			handful_friend = p
			break
		end
	end
	if not handful_friend then
		self:sort(self.friends_noself, "handcard", true)
		for _, p in ipairs(self.friends_noself) do
			if not self:isWeak(p) and p:getHandcardNum() >= 4 then
				handful_friend = p
				break
			end
		end
	end
	
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() and self:isEnemy(p) and (p:getHandcardNum() <= 3 or not handful_friend) then
			table.insert(handful, p)
		end
	end
	
	if #handful > 1 or (handful_friend and #handful > 0) then
		self:sort(handful, "handcard")
		use.card = card
		if use.to then
			if handful_friend and handful_friend:hasSkill("jisha") and getKnownCard(handful_friend, self.player, "Slash", false) > 0 then
				sgs.updateIntention(self.player, handful_friend, -40)
				sgs.updateIntention(self.player, handful[1], 80)
				use.to:append(handful[1])
				use.to:append(handful_friend)
			elseif #handful > 1 then
				sgs.updateIntention(self.player, handful[1], 80)
				sgs.updateIntention(self.player, handful[2], 80)
				use.to:append(handful[1])
				use.to:append(handful[2])
			else
				sgs.updateIntention(self.player, handful[1], 80)
				use.to:append(handful[1])
				use.to:append(handful_friend)
			end
		end
		return
	end
end

sgs.ai_skill_choice.shanchou = function(self, choices, data)
	local pindian = data:toPindian()
	local from = pindian.from
	local to = pindian.to
	local fcard = pindian.from_card
	local tcard = pindian.to_card
	
	if (fcard:getNumber() <= tcard:getNumber() and self:isFriend(from)) or (fcard:getNumber() > tcard:getNumber() and self:isFriend(to)) then
		return "SCGet"
	end
	
	local lose = {}
	if fcard:getNumber() <= tcard:getNumber() then
		table.insert(lose, fcard)
	end
	if tcard:getNumber() <= fcard:getNumber() then
		table.insert(lose, tcard)
	end
	
	for _, c in ipairs(lose) do
		if (c:isKindOf("Peach") and self:cardNeed(c) >= 10)
				or (c:isKindOf("Jink") and self:isVeryWeak())
				or (c:isKindOf("Analeptic") and self:isVeryWeak())
				or (c:isKindOf("ExNihilo"))
				or (c:isKindOf("Haze"))
				or (c:isKindOf("Snatch") and self:getNearestEnemyDistance() <= 1)
				or (c:isKindOf("Slash") and self.player:hasWeapon("CrossBow")) then
			return "SCGet"
		end
	end
	return "SCDamage"
end

sgs.ai_use_priority.ShanchouCard = 1.2
sgs.ai_use_value.ShanchouCard = 9.0
sgs.dynamic_value.control_card.ShanchouCard = true
sgs.dynamic_value.damage_card.ShanchouCard = true

function sgs.ai_skill_pindian.shanchou(minusecard, self, requestor, maxcard)
	local cards, maxcard = sgs.QList2Table(self.player:getHandcards())
	local function compare_func(a, b)
		return a:getNumber() > b:getNumber()
	end
	table.sort(cards, compare_func)
	if self.player:hasSkill("jisha") then
		for _, card in ipairs(cards) do
			if card:isKindOf("Slash") then
				return card
			end
		end
	end
	for _, card in ipairs(cards) do
		if self:getUseValue(card) < 6 then maxcard = card break end
	end
	return maxcard or cards[1]
end

sgs.ai_skill_invoke.zhouyuan = true

sgs.ai_skill_cardask["@zhouyuan-give"] = function(self, data)
	local room = self.room
	local parsee = room:findPlayerBySkillName("zhouyuan")
	if self:isFriend(parsee) then
		return "."
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf("Slash") then
			return c:getEffectiveId()
		end
		if c:isKindOf("Jink") and not (self:isWeak() and self:getCardsNum("Jink") <= 1) then
			return c:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_skill_use["@@jisha"] = function(self, prompt)
	if #self.enemies > 0 then
		self:sort(self.enemies, "hp")
		for _, p in ipairs(self.enemies) do
			if not self:needToLoseHp(p, self.player) and not self:cantbeHurt(p) and self:isWeak(p) then
				return "@JishaCard=.->" .. p:objectName()
			end
		end
	end
	if #self.friends_noself > 0 then
		self:sort(self.friends_noself, "hp")
		for _, p in ipairs(self.friends_noself) do
			if self:needToLoseHp(p, self.player) then
				return "@JishaCard=.->" .. p:objectName()
			end
		end
	end
	if #self.enemies > 0 then
		self:sort(self.enemies, "hp")
		for _, p in ipairs(self.enemies) do
			if not self:needToLoseHp(p, self.player) and not self:cantbeHurt(p) then
				return "@JishaCard=.->" .. p:objectName()
			end
		end
	end
	return "."
end

sgs.ai_skill_invoke.jiuyan = true

sgs.ai_skill_choice.jiuyan = function(self, choices, data)
	local recover = data:toRecover()
	if not self:isFriend(recover.to) then
		return "JYDraw"
	end
	if sgs.getDefense(self.player) <= sgs.getDefense(recover.to) then
		return "JYDraw"
	else
		return "JYLetDraw"
	end
end

sgs.ai_skill_invoke.huayuan = function(self, data)
	local room = self.room
	local use = data:toCardUse()
	if self:isEnemy(use.from) then
		sgs.updateIntention(self.player, use.from, 80)
		return true
	end
	return false
end

sgs.ai_skill_choice.Huayuan = function(self, choices, data)
	local room = self.room
	if self:isVeryWeak() or self.player:getHp() <= 1 then
		return "HYDraw"
	end
	local use = data:toCardUse()
	if use.card:isKindOf("Slash") or use.card:isKindOf("Duel") then
		for _, p in sgs.qlist(use.to) do
			if (p:getHp() <= 1 or self:isVeryWeak(p)) and self:isEnemy(p) then
				return "HYDamage"
			end
		end
	end
	if self:getOverFlow() > 1 then return "HYDraw" end
	local yuuka = room:findPlayerBySkillName("Huayuan")
	if yuuka then
		if self:needToLoseHp(self.player, yuuka) then
			return "HYDamage"
		end
	end
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if (self:getUseValue(c) >= 8 and not c:isKindOf("Collateral")) or (c:isKindOf("Peach") and self.player:isWounded()) then
			return "HYDraw"
		end
	end
	return "HYDraw"
end

function table.contains(_table, element)
	for _, i in ipairs(_table) do
		if element == i then
			return true
		end
	end
	return false
end

sgs.ai_skill_invoke.diwen = function(self, data)
	local room = self.room
	local use = data:toCardUse()
	local from = use.from
	local harmful_cards = { "dismantlement", "snatch", "duel", "iron_chain", "fire_attack", "savage_assault", "archery_attack", "collateral",
			"phoenix_flame", "haze", "mind_reading", "icy_fog" }
	local beneficial_cards = { "ex_nihilo", "god_salvation", "amazing_grace" }
	if self:isFriend(from) then
		if from:getWeapon() and from:getWeapon():isKindOf("CrossBow") then
			for _, p in sgs.qlist(room:getOtherPlayers(from)) do
				if from:distanceTo(p) <= 1 and from:canSlash(p) and self:isEnemy(from, p) then
					return true
				end
			end
		end
		if table.contains(beneficial_cards, use.card:objectName()) then
			if use.card:isKindOf("ExNihilo") then
				if not ((self:lackCards() or self:isWeak()) and from:getRole() ~= "lord")
						or (self:lackCards(from) or self:isWeak(from) and (not self:isWeak() or from:getRole() == "lord")) then
					sgs.updateIntention(self.player, from, -80)
					return true
				end
			elseif use.card:isKindOf("GodSalvation") then
				if self.player:getMark("@changed") > 0 then
					for _, p in sgs.qlist(use.to) do
						if self:isEnemy(p) and p:isWounded() then
							return true
						end
					end
					for _, p in sgs.qlist(use.to) do
						if not p:isWounded() then
							return true
						end
					end
				end
				if self.player:isWounded() and ((self:isWeak() and from:getRole() ~= "lord")
						or not (self:lackCards(from) or self:isWeak(from))) then
					return false
				end
			elseif use.card:isKindOf("AmazingGrace") then
				if self.player:getMark("@changed") > 0 then
					for _, p in sgs.qlist(use.to) do
						if self:isEnemy(p) then
							return true
						end
					end
				end
				if ((self:lackCards() or self:isWeak()) and from:getRole() ~= "lord")
						or not (self:lackCards(from) or self:isWeak(from)) then
					return false
				end
			end
		end
		sgs.updateIntention(self.player, from, -80)
		return true
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if use.card:getNumber() > 0 and c:getNumber() > use.card:getNumber()
				and not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Analeptic") and self:isVeryWeak())
				and not (c:isKindOf("Jink") and self:isWeak() and self:getCardsNum("Jink") <= 1)
				and not c:isKindOf("ExNihilo") and not (c:isKindOf("Dismantlement") and not self:isWeak()) then
			if table.contains(harmful_cards, use.card:objectName()) then
				return true
			elseif table.contains(beneficial_cards, use.card:objectName()) then
				if self.player:getMark("@changed") > 0 then
					if use.card:isKindOf("GodSalvation") then
						for _, p in sgs.qlist(use.to) do
							if self:isEnemy(p) and p:isWounded() and self:isVeryWeak(p) then
								return true
							end
						end
					elseif use.card:isKindOf("AmazingGrace") then
						for _, p in sgs.qlist(use.to) do
							if self:isEnemy(p) and p:objectName() ~= use.from:objectName() and self:isWeak(p) then
								return true
							end
						end
					end
				end
			end
		end
	end
	if not (use.from:getWeapon() and use.from:getWeapon():isKindOf("CrossBow")) and not (use.from:getHandcardNum() >= 4 and use.from:getHp() >= 3) then
		if table.contains(harmful_cards, use.card:objectName()) then
			if self:isWeak() then
				return true
			end
			if self.player:getMark("@changed") > 0 then
				for _, p in sgs.qlist(use.to) do
					if self:isFriend(p) and self:isWeak(p) then
						return true
					end
				end
			end
		elseif table.contains(beneficial_cards, use.card:objectName()) then
			if self.player:getMark("@changed") > 0 then
				if use.card:isKindOf("GodSalvation") then
					for _, p in sgs.qlist(use.to) do
						if self:isEnemy(p) and p:isWounded() and self:isVeryWeak(p) then
							return true
						end
					end
				elseif use.card:isKindOf("AmazingGrace") then
					for _, p in sgs.qlist(use.to) do
						if self:isEnemy(p) and p:objectName() ~= use.from:objectName() and self:isVeryWeak(p) then
							return true
						end
					end
				end
			end
		end
	end
	return false
end

sgs.ai_skill_cardask["@diwen-discard"] = function(self, data)
	local room = self.room
	local use = data:toCardUse()
	if use.from and self:isFriend(use.from) then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if use.card and use.card:getNumber() > 0 and c:getNumber() > use.card:getNumber()
				and not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Analeptic") and self:isVeryWeak())
				and not (c:isKindOf("Jink") and self:isWeak() and self:getCardsNum("Jink") <= 1)
				and not c:isKindOf("ExNihilo") and not (c:isKindOf("Dismantlement") and not self:isWeak()) then
			return c:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_skill_use["@@diwen"] = function(self, prompt)
	local room = self.room
	local use = self.player:getTag("diwenCardUse"):toCardUse()
	local harmful_cards = { "dismantlement", "snatch", "duel", "iron_chain", "fire_attack", "savage_assault", "archery_attack", "collateral",
			"phoenix_flame", "haze", "mind_reading", "icy_fog" }
	local beneficial_cards = { "ex_nihilo", "god_salvation", "amazing_grace" }
	local maximum = math.min(self.player:getLostHp() + 1, use.to:length())
	local targets = {}
	if table.contains(harmful_cards, use.card:objectName()) then
		self:sort(self.friends, "hp")
		for _, p in ipairs(self.friends) do
			if use.to:contains(p) and #targets < maximum then
				sgs.updateIntention(self.player, p, -80)
				table.insert(targets, p:objectName())
				if #targets >= maximum then
					break
				end
			end
		end
	elseif table.contains(beneficial_cards, use.card:objectName()) then
		if use.card:isKindOf("ExNihilo") then
			table.insert(targets, self.player:objectName())
		else
			self:sort(self.enemies, "hp")
			if use.card:isKindOf("GodSalvation") then
				for _, p in ipairs(self.enemies) do
					if use.to:contains(p) and p:isWounded() and #targets < maximum then
						sgs.updateIntention(self.player, p, 80)
						table.insert(targets, p:objectName())
						if #targets >= maximum then
							break
						end
					end
				end
				if #targets == 0 then
					for _, p in sgs.qlist(use.to) do
						if not p:isWounded() then
							table.insert(targets, p:objectName())
							break
						end
					end
				end
			elseif use.card:isKindOf("AmazingGrace") then
				for _, p in ipairs(self.enemies) do
					if use.to:contains(p) and #targets < maximum then
						sgs.updateIntention(self.player, p, 80)
						table.insert(targets, p:objectName())
						if #targets >= maximum then
							break
						end
					end
				end
			end
		end
	end
	--room:writeToConsole("Diwen Identified: " .. table.concat(targets, ","))
	return "@DiwenCard=.->" .. table.concat(targets, "+")
end

sgs.ai_skill_invoke.chengzhao = true

sgs.ai_skill_choice.chengzhao = function(self, choices, data)
	local room = self.room
	local damage = data:toDamage()
	if (self:getCardsNum("Peach") + self:getCardsNum("Analeptic") >= 2 and self.player:getHandcardNum() <= 3)
			or self:getCardsNum("Peach") + self:getCardsNum("Analeptic") + self:getCardsNum("Jink") >= 3 then
		return "CZModify"
	end
	if self:getCardsNum("Peach") + self:getCardsNum("Analeptic") + self:getCardsNum("Jink") >= 2 and self.player:getLostHp() >= 2 then
		return "CZModify"
	end
	if self:isWeak() then return "CZDraw" end
	if not damage.from or self:isFriend(damage.from) then
		return "CZDraw"
	elseif self:isEnemy(damage.from) then
		if self:lackCards() then
			if damage.from:getHandcardNum() > 2 then
				return "CZDraw"
			else
				return "CZModify"
			end
		elseif damage.from:getHandcardNum() <= 2 then
			if #self.enemies >= #self.friends_noself then
				return "CZDraw"
			else
				return "CZModify"
			end
		end
	end
	return "CZDraw"
end

sgs.ai_skill_cardask["@lingmiao-discard"] = function(self, data)
	local room = self.room
	local miko = room:findPlayerBySkillName("lingmiao")
	local use = data:toCardUse()
	local harmful_cards = { "dismantlement", "snatch", "duel", "fire_attack", "savage_assault", "archery_attack", "collateral",
			"phoenix_flame", "haze", "mind_reading", "icy_fog" }
	local beneficial_cards = { "ex_nihilo", "god_salvation", "amazing_grace" }
	if self.player:isChained() then
		table.insert(beneficial_cards, "iron_chain")
	else
		table.insert(harmful_cards, "iron_chain")
	end
	local id = "."
	if self:hasLion() and self.player:isWounded() then
		id = self.player:getArmor():getEffectiveId()
	else
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, c in ipairs(cards) do
			if not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
					and not (c:isKindOf("Analeptic") and self:isVeryWeak())
					and not (c:isKindOf("Jink") and self:isWeak() and self:getCardsNum("Jink") <= 1)
					and not (c:isKindOf("TrickCard") and self:getUseValue(c) >= 8) and c:isRed() then 
				id = c:getEffectiveId()
				break
			end
		end
	end
	if id == "." then return "." end
	if self:isFriend(miko) then
		if table.contains(harmful_cards, use.card:objectName()) then
			if not miko:hasFlag("DiwenUsed") and self:isFriend(miko, self.player)
					and self:getCardsNum("ExNihilo") + self:getCardsNum("ArcheryAttack") + self:getCardsNum("AmazingGrace") == 0 then
				return id
			end
		elseif table.contains(beneficial_cards, use.card:objectName()) then
			sgs.updateIntention(self.player, miko, -100)
			return id
		end
	elseif self:isEnemy(miko) then
		if table.contains(harmful_cards, use.card:objectName()) then
			if miko:hasFlag("DiwenUsed") or (miko:getMark("@changed") == 0)
					or (miko:getMark("@changed") > 0 and self:isWeak(miko) and self:lackCards(miko)) then
				return id
			end
		elseif table.contains(beneficial_cards, use.card:objectName()) then
			return "."
		end
	end
	return "."
end

local shehun_skill = {}
shehun_skill.name = "shehun"
table.insert(sgs.ai_skills, shehun_skill)
shehun_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ShehunCard") then return nil end
	if self.player:isKongcheng() then return nil end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	local discards = {}
	for _, c in ipairs(cards) do
		if not c:isKindOf("ExNihilo") and not (c:isKindOf("Peach") and self:getCardsNum("Peach") >= 3) then
			table.insert(discards, c)
		end
	end
	if #discards == 0 then return nil end
	local targets = {}
	local friends = {}
	for _, p in ipairs(self.enemies) do
		if not p:isNude() then
			table.insert(targets, p)
		end
	end
	for _, p in ipairs(self.friends_noself) do
		if self:needToThrowArmor(p) then
			table.insert(friends, p)
		end
	end
	if #targets == 0 and #friends == 0 then return end
	
	self:sortByKeepValue(discards)
	local card_str = ("@ShehunCard=%d"):format(discards[1]:getEffectiveId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.ShehunCard = function(card, use, self)
	local targets = {}
	local friends = {}
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() then
			table.insert(targets, p)
		end
	end
	for _, p in ipairs(self.friends_noself) do
		if self:needToThrowArmor(p) then
			table.insert(friends, p)
		end
	end
	self:sort(targets, "defense")
	self:sort(friends, "defense")
	local maximum
	if self.player:getMark("@rotten") > 0 then
		maximum = 1
	else
		maximum = math.min(#targets + #friends, self.player:getLostHp() + 1)
	end
	if use.to then
		for i = 1, math.min(maximum, #friends), 1 do
			sgs.updateIntention(self.player, friends[i], -80)
			use.to:append(friends[i])
		end
		if use.to:length() < maximum then
			for i = 1, math.min(maximum - use.to:length(), #targets), 1 do
				sgs.updateIntention(self.player, targets[i], 80)
				use.to:append(targets[i])
			end
		end
	end
	use.card = card
	return
end

sgs.ai_use_priority.ShehunCard = 8.5
sgs.ai_use_value.ShehunCard = 9.7
sgs.dynamic_value.control_card.ShehunCard = true

sgs.ai_skill_invoke.fuliao = true

local yuanshe_skill = {}
yuanshe_skill.name = "yuanshe"
table.insert(sgs.ai_skills, yuanshe_skill)
yuanshe_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("YuansheCard") then return nil end
	if self.player:getCards("he"):length() < 2 then return nil end
	if #self.enemies == 0 then return nil end
	local has_target = false
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() then
			has_target = true
			break
		end
	end
	if not has_target then return nil end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	local basics = {}
	local tricks = {}
	local equips = {}
	for _, c in ipairs(cards) do
		if c:isKindOf("BasicCard") then
			table.insert(basics, c)
		elseif c:isKindOf("TrickCard") then
			table.insert(tricks, c)
		elseif c:isKindOf("EquipCard") then
			table.insert(equips, c)
		end
	end
	local card1, card2
	if self:hasLion() and self.player:isWounded() and #equips >= 2 then
		card1 = self:getArmor()
	end
	for _, c in ipairs(cards) do
		if not card1 then
			if not (c:isKindOf("Peach") and self:getCardsNum("Peach") >= 3)
					and not (c:isKindOf("Analeptic") and self:isWeak())
					and not (c:isKindOf("Jink") and (self:getCardsNum("Jink") == 1 or self:isWeak()))
					and not (c:isKindOf("ExNihilo"))
					and not (c:isKindOf("Snatch"))
					and not (c:isKindOf("Indulgence")) then
				if (c:isKindOf("BasicCard") and #basics >= 2)
						or (c:isKindOf("TrickCard") and #tricks >= 2)
						or (c:isKindOf("EquipCard") and #equips >= 2) then
					card1 = c
				end
			end
		elseif not card2 then
			if not (c:isKindOf("Peach") and self:getCardsNum("Peach") >= 3)
					and not (c:isKindOf("Analeptic") and self:isWeak())
					and not (c:isKindOf("Jink") and (self:getCardsNum("Jink") == 1 or self:isWeak()))
					and not (c:isKindOf("ExNihilo"))
					and not (c:isKindOf("Snatch"))
					and not (c:isKindOf("Indulgence")) then
				if c:getTypeId() == card1:getTypeId() then
					card2 = c
				end
			end
		end
	end
	if not card1 or not card2 then return nil end
	local card_str = ("@YuansheCard=%d+%d"):format(card1:getEffectiveId(), card2:getEffectiveId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.YuansheCard = function(card, use, self)
	local targets = {}
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() then
			table.insert(targets, p)
		end
	end
	if #targets == 0 then return nil end
	self:sort(targets, "defense")
	use.card = card
	if use.to then use.to:append(targets[1]) end
	self:speak("yuanshe", self.player:isFemale())
	return
end

sgs.ai_card_intention.YuansheCard = 80
sgs.ai_use_priority.YuansheCard = 8.9
sgs.ai_use_value.YuansheCard = 8.6
sgs.dynamic_value.control_card.YuansheCard = true

sgs.ai_skill_invoke.nianxie = true

sgs.ai_skill_askforag.nianxie = function(self, card_ids)
	local max_keep_value = -10
	local choice = -1
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if self:getKeepValue(card) > max_keep_value and card:isKindOf("BasicCard") then
			max_keep_value = self:getKeepValue(card)
			choice = id
		end
	end
	return choice
end

sgs.ai_skill_invoke.huoyun = function(self, data)
	local room = self.room
	if room:getOtherPlayers(self.player):length() <= 1 then return false end
	if #self.friends <= 1 and #self.enemies <= 1 then return false end
	local has_slash_target = false
	for _, p in ipairs(self.enemies) do
		if self.player:inMyAttackRange(p) then
			for _, c in ipairs(self:getCards("Slash")) do
				if self:slashIsEffective(c, p, self.player) then
					has_slash_target = true
					break
				end
			end
		end
		if has_slash_target then break end
	end
	if self:getCardsNum("SingleTargetTrick") == 0
			and (self:getCardsNum("Slash") == 0 or not has_slash_target)
			and (self:getCardsNum("Peach") == 0 or not self.player:isWounded()) then
		return false
	end
	return true
end

sgs.ai_skill_use["@@huoyun"] = function(self, prompt)
	local room = self.room
	local use = self.player:getTag("HuoyunUse"):toCardUse()
	local targets = {}
	for _, p in sgs.qlist(room:getAlivePlayers()) do
		if not p:hasFlag("HuoyunTargeted") and self:isFriend(p) then
			if use.card:isKindOf("Peach") then
				if p:isWounded() then
					table.insert(targets, p)
				end
			else
				table.insert(targets, p)
			end
		end
	end
	self:sort(targets, "hp")
	for _, p in ipairs(targets) do
		if self:isFriend(p) then
			return "@HuoyunCard=.->" .. p:objectName()
		else
			return "."
		end
	end
	return "."
end

sgs.ai_skill_use["@@shimian"] = function(self, prompt)
	local cards = self.player:getCards("he")
	local suits = { {}, {}, {}, {} }	--heart, diamond, spade, club
	for _, c in sgs.qlist(cards) do
		if c:getSuit() == sgs.Card_Heart then
			table.insert(suits[1], c)
		elseif c:getSuit() == sgs.Card_Diamond then
			table.insert(suits[2], c)
		elseif c:getSuit() == sgs.Card_Spade then
			table.insert(suits[3], c)
		elseif c:getSuit() == sgs.Card_Club then
			table.insert(suits[4], c)
		end
	end
	local has_weak_friend = false
	for _, p in ipairs(self.friends) do
		if p:getHp() == 1 or self:isVeryWeak(p) then
			has_weak_friend = true
		end
	end
	local peaches = self:getCardsNum("Peach")
	local jinks = self:getCardsNum("Jink")
	local analeptics = self:getCardsNum("Analeptic")
	local peach_get = 0
	local jink_get = 0
	local analeptic_get = 0
	local use_cards = {}
	local masks = { {}, {}, {}, {} }	--heart, diamond, spade, club
	for _, id in sgs.qlist(self.player:getPile("mask")) do
		local c = sgs.Sanguosha:getCard(id)
		if c:getSuit() == sgs.Card_Heart then
			table.insert(masks[1], c)
		elseif c:getSuit() == sgs.Card_Diamond then
			table.insert(masks[2], c)
		elseif c:getSuit() == sgs.Card_Spade then
			table.insert(masks[3], c)
		elseif c:getSuit() == sgs.Card_Club then
			table.insert(masks[4], c)
		end
	end
	for i = 1, 4, 1 do
		if #masks[i] == 0 then 
			self:sortByKeepValue(suits[i])
			for _, c in ipairs(suits[i]) do
				if not ((c:isKindOf("Peach") and (peach_get >= peaches - 1 and has_weak_friend))
						or (c:isKindOf("Analeptic") and (analeptic_get >= analeptics - 1 and self:isVeryWeak()))
						or (c:isKindOf("Jink") and (jink_get >= jinks - 1 and self:lackCards() and not self:hasRedCard()))) then
					table.insert(use_cards, c:getEffectiveId())
					if c:isKindOf("Peach") then
						peach_get = peach_get + 1
					elseif c:isKindOf("Analeptic") then
						analeptic_get = analeptic_get + 1
					elseif c:isKindOf("Jink") then
						jink_get = jink_get + 1
					end
					break
				end
			end
		end
	end
	if #use_cards == 0 then return "." end
	return "@ShimianCard=" .. table.concat(use_cards, "+")
end

sgs.ai_skill_invoke.shimian = function(self, data)
	local room = self.room
	local damage = data:toDamage()
	if not self:isFriend(damage.to) then
		return false
	end
	local masks = {}
	for _, id in sgs.qlist(self.player:getPile("mask")) do
		local c = sgs.Sanguosha:getCard(id)
		table.insert(masks, c:objectName())
	end
	if self:isVeryWeak(damage.to) and (table.contains(masks, "peach") or table.contains(masks, "analeptic")
			or (table.contains(masks, "jink") and (damage.card:isKindOf("Slash") or damage.card:isKindOf("ArcheryAttack") or damage.to:isKongcheng()))
			or (table.contains(masks, "slash") and damage.to:hasWeapon("CrossBow"))) then
		sgs.updateIntention(self.player, damage.to, -80)
		return true
	end
	if (self:getCardsNum("Jink") < 1 and self:getCardsNum("Peach") < 1) or self:isVeryWeak() then
		return true
	end
	return false
end

sgs.ai_skill_askforag.shimian = function(self, card_ids)
	local room = self.room
	local cards = {}
	local damage = self.player:getTag("ShimianDamageTag"):toDamage()
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		table.insert(cards, c)
	end
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf("Peach") or c:isKindOf("Analeptic") then
			return c:getEffectiveId()
		end
	end
	for _, c in ipairs(cards) do
		if c:isKindOf("Jink") and damage.card and (damage.card:isKindOf("Slash") or damage.card:isKindOf("ArcheryAttack") or damage.to:isKongcheng()) then
			return c:getEffectiveId()
		end
	end
	for _, c in ipairs(cards) do
		if c:isKindOf("Slash") and damage.to:hasWeapon("CrossBow") then
			return c:getEffectiveId()
		end
	end
	return cards[1]:getEffectiveId()
end

sgs.ai_skill_use["@@jiwu"] = function(self, prompt)
	local room = self.room
	self:sort(self.enemies, "defense")
	local targets = {}
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() then
			table.insert(targets, p)
		end
	end
	local cards = {}
	for _, id in sgs.qlist(self.player:getPile("mask")) do
		local c = sgs.Sanguosha:getCard(id)
		if not c:isKindOf("Peach") and not (c:isKindOf("Jink") and self:getCardsNum("Jink") == 0) then
			table.insert(cards, sgs.Sanguosha:getCard(id))
		end
	end
	self:sortByKeepValue(cards)
	local x = math.min(#targets, #cards)
	if x == 0 then return "." end
	local names = {}
	local ids = {}
	for i = 1, x, 1 do
		table.insert(names, targets[i]:objectName())
		table.insert(ids, cards[i]:getEffectiveId())
	end
	return "@JiwuCard=" .. table.concat(ids, "+") .. "->" .. table.concat(names, "+")
end

sgs.ai_skill_use["@@wangjie"] = function(self, prompt)
	if #self.enemies == 0 and #self.friends_noself == 0 and not self.player:isWounded() then return "." end
	local room = self.room
	self:sort(self.friends, "hp")
	self:sort(self.enemies, "hp")
	for _, p in ipairs(self.enemies) do
		if p:getHp() == 1 and (p:getRole() == "lord" or #self.friends > #self.enemies or #self.enemies == 1) then
			return "@WangjieCard=.->" .. p:objectName()
		end
	end
	for _, p in ipairs(self.friends) do
		if self:isWeak(p) and p:isWounded() then
			return "@WangjieCard=.->" .. p:objectName()
		end
	end
	if #self.enemies > 0 then return "@WangjieCard=.->" .. self.enemies[1]:objectName() end
	for _, p in ipairs(self.friends) do
		if p:isWounded() then
			return "@WangjieCard=.->" .. p:objectName()
		end
	end
	return "."
end

sgs.ai_skill_choice.wangjie = function(self, choices, data)
	local target = self.player:getTag("WangjieTarget"):toPlayer()
	if self:isEnemy(target) then
		sgs.updateIntention(self.player, target, 80)
		return "WJDamage"
	elseif self:isFriend(target) then
		sgs.updateIntention(self.player, target, -80)
		return "WJRecover"
	end
end

local shixi_skill = {}
shixi_skill.name = "shixi"
table.insert(sgs.ai_skills, shixi_skill)
shixi_skill.getTurnUseCard = function(self)
	local room = self.room
	if self.player:hasUsed("ShixiCard") then return end
	self:sort(self.friends_noself, "hp")
	for _, p in ipairs(self.friends_noself) do
		if p:getArmor() and p:getArmor():isKindOf("SilverLion") and p:isWounded() then
			if p:getArmor():getSuit() == sgs.Card_Club and self:isWeak() then
				local card_str = "@ShixiCard=."
				return sgs.Card_Parse(card_str)
			end
		end
	end
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if p:getArmor() and (p:getArmor():getSuit() ~= sgs.Card_Club)
				and not (p:getArmor():isKindOf("SilverLion") and p:isWounded())then
			return sgs.Card_Parse("@ShixiCard=.")
		elseif p:getDefensiveHorse() and (p:getDefensiveHorse():getSuit() ~= sgs.Card_Club) then
			return sgs.Card_Parse("@ShixiCard=.")
		elseif p:getWeapon() and (p:getWeapon():getSuit() ~= sgs.Card_Club) then
			return sgs.Card_Parse("@ShixiCard=.")
		elseif p:getTreasure() and (p:getTreasure():getSuit() ~= sgs.Card_Club) then
			return sgs.Card_Parse("@ShixiCard=.")
		elseif p:getOffensiveHorse() and (p:getOffensiveHorse():getSuit() ~= sgs.Card_Club) then
			return sgs.Card_Parse("@ShixiCard=.")
		end
	end
	if not self.player:getEquips():isEmpty() then
		for _, c in sgs.qlist(self.player:getEquips()) do
			if c:getSuitString() ~= "club" or c:isKindOf("SilverLion") then
				local card_str = "@ShixiCard=."
				return sgs.Card_Parse(card_str)
			end
		end
	end
	for _, p in ipairs(self.enemies) do
		if p:getArmor() and (p:getArmor():getSuit() ~= sgs.Card_Club or not self:isWeak())
				and not (p:getArmor():isKindOf("SilverLion") and p:isWounded())then
			return sgs.Card_Parse("@ShixiCard=.")
		elseif p:getDefensiveHorse() and (p:getDefensiveHorse():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			return sgs.Card_Parse("@ShixiCard=.")
		elseif p:getWeapon() and (p:getWeapon():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			return sgs.Card_Parse("@ShixiCard=.")
		elseif p:getTreasure() and (p:getTreasure():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			return sgs.Card_Parse("@ShixiCard=.")
		elseif p:getOffensiveHorse() and (p:getOffensiveHorse():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			return sgs.Card_Parse("@ShixiCard=.")
		end
	end
	return
end

sgs.ai_skill_use_func.ShixiCard = function(card, use, self)
	self:sort(self.friends_noself, "hp")
	for _, p in ipairs(self.friends_noself) do
		if p:getArmor() and p:getArmor():isKindOf("SilverLion") and p:isWounded() then
			if p:getArmor():getSuit() ~= sgs.Card_Club or not self:isWeak() then
				sgs.updateIntention(self.player, p, -80)
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
	end
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if p:getArmor() and (p:getArmor():getSuit() ~= sgs.Card_Club)
				and not (p:getArmor():isKindOf("SilverLion") and p:isWounded())then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		elseif p:getDefensiveHorse() and (p:getDefensiveHorse():getSuit() ~= sgs.Card_Club) then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		elseif p:getWeapon() and (p:getWeapon():getSuit() ~= sgs.Card_Club) then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		elseif p:getTreasure() and (p:getTreasure():getSuit() ~= sgs.Card_Club) then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		elseif p:getOffensiveHorse() and (p:getOffensiveHorse():getSuit() ~= sgs.Card_Club) then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
	end
	if not self.player:getEquips():isEmpty() then
		for _, c in sgs.qlist(self.player:getEquips()) do
			if not c:getSuitString() == "club" or c:isKindOf("SilverLion") then
				use.card = card
				if use.to then use.to:append(self.player) end
				return
			end
		end
	end
	for _, p in ipairs(self.enemies) do
		if p:getArmor() and (p:getArmor():getSuit() ~= sgs.Card_Club or not self:isWeak())
				and not (p:getArmor():isKindOf("SilverLion") and p:isWounded())then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		elseif p:getDefensiveHorse() and (p:getDefensiveHorse():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		elseif p:getWeapon() and (p:getWeapon():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		elseif p:getTreasure() and (p:getTreasure():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		elseif p:getOffensiveHorse() and (p:getOffensiveHorse():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			sgs.updateIntention(self.player, p, 80)
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
	end
end

sgs.ai_skill_cardchosen.shixi = function(self, who)
	if who:objectName() == self.player:objectName() then
		if who:hasArmorEffect("SilverLion") then
			return who:getArmor()
		end
		local cards = sgs.QList2Table(self.player:getEquips())
		self:sortByKeepValue(cards)
		return cards[1]
	end
	if self:isFriend(who) then
		return who:getArmor()
	elseif self:isEnemy(who) then
		if who:getArmor() and (who:getArmor():getSuit() ~= sgs.Card_Club or not self:isWeak())
				and not (who:getArmor():isKindOf("SilverLion") and who:isWounded())then
			return who:getArmor()
		elseif who:getDefensiveHorse() and (who:getDefensiveHorse():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			return who:getDefensiveHorse()
		elseif who:getWeapon() and (who:getWeapon():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			return who:getWeapon()
		elseif who:getTreasure() and (who:getTreasure():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			return who:getTreasure()
		elseif who:getOffensiveHorse() and (who:getOffensiveHorse():getSuit() ~= sgs.Card_Club or not self:isWeak()) then
			return who:getOffensiveHorse()
		end
	end
end

sgs.ai_use_priority.ShixiCard = 8.2
sgs.ai_use_value.ShixiCard = 7.3
sgs.dynamic_value.control_card.ShixiCard = true

sgs.ai_skill_invoke.huaying = true

sgs.ai_chaofeng.byakuren = 2
sgs.ai_chaofeng.sakuya = 2
sgs.ai_chaofeng.parsee = 5
sgs.ai_chaofeng.yuuka = 3
sgs.ai_chaofeng.hatate = 4
sgs.ai_chaofeng.nue = 2
sgs.ai_chaofeng.kokoro = 3
sgs.ai_chaofeng.ex_yukari = 3
sgs.ai_chaofeng.ex_lily = 5
