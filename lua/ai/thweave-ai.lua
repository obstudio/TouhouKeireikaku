sgs.ai_skill_use["@@jiejie"] = function(self, prompt)
	local room = self.room
	local current = room:getCurrent()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if self:isEnemy(current) then
		if current:getAttackRange() <= 1 and current:getHandcardNum() <= 4 then return false end
		local give_card
		for _, c in ipairs(cards) do
			if c:isKindOf("Peach") then
				give_card = c
			end
		end
		for _, c in ipairs(cards) do
			if c:isKindOf("Analeptic") and self:isWeak() then
				give_card = c
			end
		end
		for _, c in ipairs(cards) do
			if c:isKindOf("Jink") then
				give_card = c
			end
		end
		for _, c in ipairs(cards) do
			if c:isKindOf("Analeptic") and not self:isWeak() then
				give_card = c
			end
		end
		for _, c in ipairs(cards) do
			if c:isKindOf("Slash") then
				give_card = c
			end
		end
		if not give_card then return "." end
		if give_card:isKindOf("Peach") or (give_card:isKindOf("Analeptic") and (self:isWeak() or self:isWeak(current))) then
			return "."
		end
		if give_card:isKindOf("Jink") and (self:getCardsNum("Jink") <= 1 or (current:getHandcardNum() <= 2 and current:isWeak())) then
			return "."
		end
		if give_card:isKindOf("Slash") and getKnownCard(current, self.player, "SingleTargetTrick", true)
				- getKnownCard(current, self.player, "ExNihilo", true) + getKnownCard(current, self.player, "PhoenixFlame", true) >= 1 then
			local id = give_card:getEffectiveId()
			return ("@JiejieCard=%d->%s"):format(id, current:objectName())
		end
		if self.player:getHandcardNum() >= 3 and self:hasWeakFriend(true, false) then
			local id = give_card:getEffectiveId()
			return ("@JiejieCard=%d->%s"):format(id, current:objectName())
		end
		return "."
	elseif self:isFriend(current) then
		if self:isWeak(p) and (self:getCardsNum("Peach") + self:getCardsNum("Analeptic") + self:getCardsNum("Jink") > 0 and not self:isWeak()) then
			for _, c in ipairs(cards) do
				if c:isKindOf("Peach") or c:isKindOf("Analeptic") or c:isKindOf("Jink") then
					return ("@JiejieCard=%d->%s"):format(c:getEffectiveId(), current:objectName())
				end
			end
		elseif current:isWounded() and ((self:getCardsNum("Peach") > 0 and not self:isWeak()) or (self:getCardsNum("Peach") > 1)) then
			for _, c in ipairs(cards) do
				if c:isKindOf("Peach") then
					return ("@JiejieCard=%d->%s"):format(c:getEffectiveId(), current:objectName())
				end
			end
		elseif current:getHandcardNum() <= 3 and getKnownCard(current, self.player, "Jink", true) == 0 and self:getCardsNum("Jink") > 1
				and not self:isWeak() then
			for _, c in ipairs(cards) do
				if c:isKindOf("Jink") then
					return ("@JiejieCard=%d->%s"):format(c:getEffectiveId(), current:objectName())
				end
			end
		elseif getKnownCard(current, self.player, "Slash", true) > 1 and not self:isWeak() and self.player:getHandcardNum() >= 3 then
			for _, c in ipairs(cards) do
				if c:isKindOf("Analeptic") then
					return ("@JiejieCard=%d->%s"):format(c:getEffectiveId(), current:objectName())
				end
			end
		elseif current:getAttackRange() <= self:getNearestEnemyDistance(current) or (getKnownCard(current, self.player, "Slash", true) <= 1
				and current:getHandcardNum() <= 3) then
			for _, c in ipairs(cards) do
				if c:isKindOf("Slash") then
					return ("@JiejieCard=%d->%s"):format(c:getEffectiveId(), current:objectName())
				end
			end
		end
		return "."
	end
	return "."
end

sgs.ai_skill_choice.jiejie = function(self, choices)
	local player = self.room:getCurrent()
	if self:isFriend(player) then
		return "JJBuff"
	else
		return "JJDebuff"
	end
end

sgs.ai_skill_cardask["@xianhu-discard"] = function(self, data)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf("BasicCard") then
			return c:getEffectiveId()
		end
	end
	return "."
end

function SmartAI:hasCardToShishen()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	
	for _, c in ipairs(cards) do
		if c:isKindOf("TrickCard") and not (c:isKindOf("ExNihilo") and not self:isVeryWeak()) then
			return true
		end
	end
	
	for _, c in ipairs(cards) do
		if self:lackCards() and c:isKindOf("BasicCard") and not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Analeptic") and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 2
				and self:isWeak()) and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 2) then
			return true
		end
	end
	
	for _, c in ipairs(cards) do
		if c:isKindOf("EquipCard") then
			if #self.enemies == 0 then break end
			local has_victim = false
			for _, p in ipairs(self.enemies) do
				if not p:isNude() then
					has_victim = true
					break
				end
			end
			if not has_victim then break end
			return true
		end
	end
	
	for _, c in ipairs(cards) do
		if c:isKindOf("BasicCard") and not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Analeptic") and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 2
				and self:isWeak()) and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 2) then
			return true
		end
	end
	
	return false
end 

sgs.ai_skill_use["@@shishen"] = function(self, prompt)
	local targets = sgs.SPlayerList()
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if (not p:isKongcheng()) and p:isWounded() and self:isFriend(p)
				and not (p:objectName() == self.player:objectName() and not self:hasCardToShishen()) then
			targets:append(p)
		end
	end
	if targets:isEmpty() then return "." end
	targets = sgs.QList2Table(targets)
	self:sort(targets, "hp")
	sgs.updateIntention(self.player, targets[1], -80)
	return "@ShishenCard=.->" .. targets[1]:objectName()
end

sgs.ai_skill_cardask["@shishen-discard"] = function(self, targets)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	
	for _, c in ipairs(cards) do
		if c:isKindOf("TrickCard") and not (c:isKindOf("ExNihilo") and not self:isVeryWeak()) then
			return c:getEffectiveId()
		end
	end
	
	for _, c in ipairs(cards) do
		if self:lackCards() and c:isKindOf("BasicCard") and not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Analeptic") and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 2
				and self:isWeak()) and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 2) then
			return c:getEffectiveId()
		end
	end
	
	for _, c in ipairs(cards) do
		if c:isKindOf("EquipCard") then
			if #self.enemies == 0 then break end
			local has_victim = false
			for _, p in ipairs(self.enemies) do
				if not p:isNude() then
					has_victim = true
					break
				end
			end
			if not has_victim then break end
			return c:getEffectiveId()
		end
	end
	
	for _, c in ipairs(cards) do
		if c:isKindOf("BasicCard") and not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Analeptic") and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 2
				and self:isWeak()) and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 2) then
			return c:getEffectiveId()
		end
	end
	
	return "."
end

sgs.ai_skill_playerchosen.shishen = function(self, targets)
	self:sort(self.enemies, "chaofeng")
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() then
			return p
		end
	end
end

sgs.ai_skill_invoke.dunjia = function(self, data)
	local use = data:toCardUse()
	for _, p in sgs.qlist(use.to) do
		if (not (self:isEnemy(p) and p:getHp() >= 2)) and (self:getCardsNum("Peach") > 0
				or (self.player:getHp() >= 2 and self.player:getHandcardNum() >= 2)) then
			return true
		end
	end
	return false
end

local yuanqi_skill = {}
yuanqi_skill.name = "yuanqi"
table.insert(sgs.ai_skills, yuanqi_skill)
yuanqi_skill.getTurnUseCard = function(self)
	local room = self.room
	if self.player:hasUsed("YuanqiCard") then return nil end
	local cards = self.player:getCards("e")
	if cards:isEmpty() then return nil end
	cards = sgs.QList2Table(cards)

	for _, card in ipairs(cards) do
		if card:isKindOf("SilverLion") and self.player:isWounded() then
			self:sort(self.friends_noself, "hp")
			self:sort(self.enemies, "hp", true)
			for _, p in ipairs(self.friends_noself) do
				if not p:getArmor() and p:isWounded() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
			for _, p in ipairs(self.enemies) do
				if not p:getArmor() and not p:isWounded() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
		end
	end
	
	for _, card in ipairs(cards) do
		if card:isKindOf("Yasakaninomagatama") and self.player:getMaxHp() > 1 then
			self:sort(self.friends_noself, "hp")
			for _, p in ipairs(self.friends_noself) do
				if not p:getTreasure() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
		end
	end
	
	for _, card in ipairs(cards) do
		if card:isKindOf("Treasure") then
			self:sort(self.friends_noself, "defense")
			for _, p in ipairs(self.friends_noself) do
				if not p:getTreasure() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
		end
	end
	
	for _, card in ipairs(cards) do
		if card:isKindOf("OffensiveHorse") then
			self:sort(self.friends_noself, "handcard", true)
			self:sort(self.enemies, "handcard")
			for _, p in ipairs(self.friends_noself) do
				if not p:getOffensiveHorse() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
			for _, p in ipairs(self.enemies) do
				if not p:getOffensiveHorse() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
		end
	end
	
	for _, card in ipairs(cards) do
		if card:isKindOf("Weapon") then
			self:sort(self.friends_noself, "handcard", true)
			self:sort(self.enemies, "handcard")
			for _, p in ipairs(self.friends_noself) do
				if not p:getWeapon() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
			for _, p in ipairs(self.enemies) do
				if not p:getWeapon() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
		end
	end
	
	for _, card in ipairs(cards) do
		if card:isKindOf("DefensiveHorse") then
			self:sort(self.friends_noself, "defense")
			self:sort(self.enemies, "defense", true)
			for _, p in ipairs(self.friends_noself) do
				if not p:getDefensiveHorse() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
			for _, p in ipairs(self.enemies) do
				if not p:getDefensiveHorse() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
		end
	end
	
	for _, card in ipairs(cards) do
		if card:isKindOf("Armor") then
			self:sort(self.friends_noself, "defense")
			self:sort(self.enemies, "defense", true)
			for _, p in ipairs(self.friends_noself) do
				if not p:getArmor() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
			for _, p in ipairs(self.enemies) do
				if not p:getArmor() and not p:isWounded() then
					local card_str = ("@YuanqiCard=%d"):format(card:getEffectiveId())
					return sgs.Card_Parse(card_str)
				end
			end
		end
	end
end

sgs.ai_skill_use_func.YuanqiCard = function(card, use, self)
	local c = sgs.Sanguosha:getCard(card:getSubcards():at(0))
	if c:isKindOf("SilverLion") then
		self:sort(self.friends_noself, "hp")
		self:sort(self.enemies, "hp", true)
		for _, p in ipairs(self.friends_noself) do
			if not p:getArmor() and p:isWounded() then
				use.card = card
				if use.to then
					sgs.updateIntention(self.player, p, -80)
					use.to:append(p)
				end
				return 
			end
		end
		for _, p in ipairs(self.enemies) do
			if not p:getArmor() and not p:isWounded() then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
	end
	
	if c:isKindOf("Yasakaninomagatama") then
		self:sort(self.friends_noself, "hp")
		for _, p in ipairs(self.friends_noself) do
			if not p:getTreasure() then
				use.card = card
				if use.to then
					sgs.updateIntention(self.player, p, -80)
					use.to:append(p)
				end
				return 
			end
		end
	end
	
	if c:isKindOf("Treasure") then
		self:sort(self.friends_noself, "defense")
		for _, p in ipairs(self.friends_noself) do
			if not p:getTreasure() then
				use.card = card
				if use.to then
					sgs.updateIntention(self.player, p, -40)
					use.to:append(p)
				end
				return 
			end
		end
	end
	
	if c:isKindOf("OffensiveHorse") then
		self:sort(self.friends_noself, "handcard", true)
		self:sort(self.enemies, "handcard")
		for _, p in ipairs(self.friends_noself) do
			if not p:getOffensiveHorse() then
				use.card = card
				if use.to then use.to:append(p) end
				return 
			end
		end
		for _, p in ipairs(self.enemies) do
			if not p:getOffensiveHorse() then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
	end
	
	if c:isKindOf("Weapon") then
		self:sort(self.friends_noself, "handcard", true)
		self:sort(self.enemies, "handcard")
		for _, p in ipairs(self.friends_noself) do
			if not p:getWeapon() then
				use.card = card
				if use.to then use.to:append(p) end
				return 
			end
		end
		for _, p in ipairs(self.enemies) do
			if not p:getWeapon() then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
	end
	
	if c:isKindOf("DefensiveHorse") then
		self:sort(self.friends_noself, "defense")
		self:sort(self.enemies, "defense", true)
		for _, p in ipairs(self.friends_noself) do
			if not p:getDefensiveHorse() then
				use.card = card
				if use.to then use.to:append(p) end
				return 
			end
		end
		for _, p in ipairs(self.enemies) do
			if not p:getDefensiveHorse() then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
	end
	
	if c:isKindOf("Armor") then
		self:sort(self.friends_noself, "defense")
		self:sort(self.enemies, "defense", true)
		for _, p in ipairs(self.friends_noself) do
			if not p:getArmor() then
				use.card = card
				if use.to then use.to:append(p) end
				return 
			end
		end
		for _, p in ipairs(self.enemies) do
			if not p:getArmor() and not p:isWounded() then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
	end
end

sgs.ai_use_value.YuanqiCard = 7.1
sgs.ai_use_priority.YuanqiCard = 0.6
sgs.dynamic_value.control_card.YuanqiCard = true

sgs.ai_skill_askforag.yuanqi = function(self, card_ids)
	local room = self.room
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("SilverLion") and (self:isFriend(room:getCardOwner(id)) and room:getCardOwner(id):isWounded()) then
			return id
		end
	end
	
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("Yasakaninomagatama") and self.player:isWounded() then
			return id
		end
	end
	
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("Armor") and not (c:isKindOf("SilverLion") and not self:isFriend(room:getCardOwner(id))) then
			return id
		end
	end
	
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("Yasakaninomagatama") then
			return id
		end
	end
	
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("WoodenOx") then
			local p = room:getCardOwner(id)
			if p:getPile("wooden_ox"):length() > 0 and not (self:isFriend(p) and self:isWeak(p)) then
				return id
			end
		end
	end
	
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("DefensiveHorse") then
			return id
		end
	end
	
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("Weapon") then
			return id
		end
	end
	
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("WoodenOx") then
			local p = room:getCardOwner(id)
			if not (self:isFriend(p) and self:isWeak(p)) then
				return id
			end
		end
	end
	
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("OffensiveHorse") then
			return id
		end
	end
	
	return card_ids[1]
end

sgs.ai_skill_use["@@mishi"] = function(self, prompt)
	local room = self.room
	if self.player:hasUsed("YuanqiCard") and self.player:getPhase() ~= sgs.Player_Finish
			and self.player:getPhase() ~= sgs.Player_NotActive then
		return "."
	end
	self:sort(self.friends_noself, "defense")
	local targets = {}
	for _, p in ipairs(self.friends) do
		if p:hasFlag("MishiTarget") then
			table.insert(targets, p:objectName())
		end
	end
	if #targets == 0 then return "." end
	return "@MishiCard=.->" .. table.concat(targets, "+")
end

sgs.ai_skill_use["@@miezui"] = function(self, prompt)
	if self.player:getHandcardNum() < 2 then
		return "."
	end
	
	local damage = self.room:getTag("miezuiDamage"):toDamage()
	if damage.damage > 1 then
		return "."
	end
	
	if self.player:getArmor() and self.player:getArmor():isKindOf("SilverLion") and self.player:isWounded() then
		return "."
	end
	
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for i = 1, #cards - 1, 1 do
		local c1 = cards[i]
		for j = i + 1, #cards, 1 do
			local c2 = cards[j]
			if c1:getSuit() == c2:getSuit() and not c1:isKindOf("Peach") and not c2:isKindOf("Peach")
					and not c1:isKindOf("ExNihilo") and not c2:isKindOf("ExNihilo")
					and not (c1:isKindOf("Analeptic") and c2:isKindOf("Analeptic") and self:isWeak()) then
				local id1 = c1:getEffectiveId()
				local id2 = c2:getEffectiveId()
				return ("@MiezuiCard=%d+%d"):format(id1, id2)
			end
		end
	end
	
	return "."
end

sgs.ai_skill_askforyiji.kuaiqing = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end

	if self.player:getHandcardNum() <= 2 then
		return nil, -1
	end

	local new_friends = {}
	local CanKeep
	for _, friend in ipairs(self.friends) do
		if not self:needKongcheng(friend, true) then
			if friend:objectName() == self.player:objectName() then CanKeep = true
			else
				table.insert(new_friends, friend)
			end
		end
	end

	if #new_friends > 0 then
		local card, target = self:getCardNeedPlayer(cards)
		if card and target then
			for _, friend in ipairs(new_friends) do
				if target:objectName() == friend:objectName() then
					return friend, card:getEffectiveId()
				end
			end
		end
		self:sort(new_friends, "defense")
		self:sortByKeepValue(cards, true)
		return new_friends[1], cards[1]:getEffectiveId()
	elseif CanKeep then
		return nil, -1
	else
		local other = {}
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			table.insert(other, player)
		end
		return other[math.random(1, #other)], card_ids[math.random(1, #card_ids)]
	end

end

sgs.ai_skill_use["@@kuaiqing"] = function(self, prompt)
	local room = self.room
	self:sort(self.friends, "defense")
	local n = self.player:getHandcardNum()
	local targets = {}
	for _, p in ipairs(self.friends) do
		if #targets < n then
			table.insert(targets, p:objectName())
			if #targets >= n then break end
		end
	end
	if #targets > 0 then
		return "@KuaiqingCard=.->" .. table.concat(targets ,"+")
	end
	return "."
end

sgs.ai_skill_choice.Kuaiqing = function(self, choices)
	if self.player:getEquips():isEmpty() then return "KuaiqingDamage" end
	if self:needToThrowArmor() then return "KuaiqingDisarm" end
	if self.player:getEquips():length() == 1 then return "KuaiqingDisarm" end
	if self:isVeryWeak() then return "KuaiqingDisarm" end
	if self:getCardsNum("Peach") + self:getCardsNum("Analeptic") + self:getCardsNum("ExNihilo") >= self.player:getHandcardNum() / 3 then
		return "KuaiqingDisarm"
	end
	for _, c in sgs.qlist(self.player:getEquips()) do
		if c:isKindOf("WoodenOx") then
			local woods = self.player:getPile("wooden_ox")
			if (woods:length() >= 2) then
				return "KuaiqingDamage"
			end
			for _, id in sgs.qlist(woods) do
				if sgs.Sanguosha:getCard(id):isKindOf("Peach") or sgs.Sanguosha:getCard(id):isKindOf("ExNihilo")
						or (sgs.Sanguosha:getCard(id):isKindOf("Analeptic") and self:isWeak()) then
					return "KuaiqingDamage"
				end
			end
		end
	end
	return "KuaiqingDisarm"
end

sgs.ai_skill_invoke.lingche = true

function SmartAI:canMaoyouSelf(player)
	player = player or self.player
	if not player:hasSkill("maoyou") then return false end
	local n = 0
	for _, p in sgs.qlist(self.room:getAllPlayers(true)) do
		if p:isDead() then
			n = n + 1
		end
	end
	if n > 3 and player:getMark("@spirit") == 0 then
		n = 3
	end
	if player:objectName() ~= self.player:objectName() then
		if (n <= 2 and player:getCards("he"):length() >= n) or player:getCards("he"):length() >= n + 2 then
			return true
		end
	else
		local t = 0
		local cards = sgs.QList2Table(self.player:getCards("he"))
		self:sortByUseValue(cards)
		for _, c in ipairs(cards) do
			if not c:isKindOf("Peach") and not (c:isKindOf("Jink") and self:getCards("Jink") == 1) and not c:isKindOf("Analeptic") then
				t = t + 1
			end
		end
		if t >= n then
			return true
		end
	end
	return false
end

sgs.ai_skill_invoke.maoyou = true

sgs.ai_skill_discard.maoyou = function(self, discard_num, optional, include_equip)
	if self.player:getCards("he"):length() < discard_num then
		return {}
	end
	local discards = {}
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards)
	local player = self.room:getCurrentDyingPlayer()
	for _, c in ipairs(cards) do
		if not c:isKindOf("Peach") and not c:isKindOf("ExNihilo") and not (discard_num > 2 and not self:isFriend(player)
				and ((c:isKindOf("Jink") and self:getCards("Jink") == 1) or (c:isKindOf("Analeptic") and self:isWeak())))
				and #discards < discard_num then
			table.insert(discards, c:getEffectiveId())
			if #discards >= discard_num then break end
		end
	end
	return discards
end

sgs.ai_skill_choice.maoyou = function(self, choices)
	local room = self.room
	if #choices == 1 then return choices[1] end
	local player = room:getCurrentDyingPlayer()
	if player:objectName() == self.player:objectName() and self.player:getHp() == 0 then
		return "MaoyouDraw"
	elseif self:isFriend(player) then
		local peaches = 0;
		for _, p in ipairs(self.friends) do
			if p:objectName() == player:objectName() then
				peaches = peaches + getKnownCard(p, self.player, "Peach", true) + getKnownCard(p, self.player, "Analeptic", true)
			else
				peaches = peaches + getKnownCard(p, self.player, "Peach", true)
			end
		end
		if peaches < 1 - player:getHp() then
			return "MaoyouDraw"
		elseif #self.enemies > 0 then
			return "MaoyouDiscard"
		elseif self:touhouFindPlayerToDraw(true, 2, self:getFriendsList()) then
			return "MaoyouDraw"
		elseif table.contains(string.split(choices, "+"), "MaoyouDiscard") then
			return "MaoyouDiscard"
		else
			return "MaoyouDraw"
		end
	elseif self:isEnemy(player) then
		local peaches = 0;
		for _, p in ipairs(self.enemies) do
			if p:objectName() == player:objectName() then
				peaches = peaches + getKnownCard(p, self.player, "Peach", true) + getKnownCard(p, self.player, "Analeptic", true)
			else
				peaches = peaches + getKnownCard(p, self.player, "Peach", true)
			end
		end
		if peaches > 0 then
			return "MaoyouDiscard"
		else
			for _, p in ipairs(self.enemies) do
				if p:getHandcardNum() >= 4 then
					return "MaoyouDiscard"
				end
			end
			if self:touhouFindPlayerToDraw(true, 2, self:getFriendsList()) then
				return "MaoyouDraw"
			elseif table.contains(string.split(choices, "+"), "MaoyouDiscard") then
				return "MaoyouDiscard"
			else
				return "MaoyouDraw"
			end
		end
	else
		return "MaoyouDraw"
	end
end

sgs.ai_skill_playerchosen.MaoyouDrawTarget = function(self, targets)
	self:sort(self.friends, "hp")
	return self.friends[1]
end

sgs.ai_skill_playerchosen.MaoyouDiscardTarget = function(self, targets)
	self:sort(self.enemies, "defense", true)
	for _, p in ipairs(self.enemies) do
		if not p:isNude() then
			return p
		end
	end
	local max_player = self.player
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if max_player:objectName() == self.player:objectName() or p:getHandcardNum() > max_player:getHandcardNum() then
			max_player = p
		end
	end
	return max_player
end

sgs.ai_skill_invoke.shihun = function(self, data)
	local room = self.room
	local player = data:toDying().who
	if not self:isFriend(player) then
		return false
	end
	if self.role == "loyalist" and player:getRole() == "lord" then
		return true
	end
	if self.role == "regenade" and player:getRole() == "lord" and room:getAlivePlayers():length() > 1 then
		return true
	end
	if self.role == "lord" then
		if self.player:getHp() == 1 then
			local peaches = self:getCardsNum("Peach") + self:getCardsNum("Analeptic")
			for _, p in ipairs(self.friends_noself) do
				peaches = peaches + getKnownCard(p, self.player, "Peach", true)
			end
			if peaches > 0 then
				return true
			end
		end
		return false
	elseif self.role == "regenade" then
		if self.player:getHp() == 1 then
			local peaches = self:getCardsNum("Peach") + self:getCardsNum("Analeptic")
			for _, p in ipairs(self.friends_noself) do
				peaches = peaches + getKnownCard(p, self.player, "Peach", true)
			end
			if peaches > 0 then
				return true
			end
		end
		return false
	else
		if self.player:getHp() <= 1 and self.player:getHandcardNum() <= 2 then
			return true
		end
		if self.player:getHp() >= 3 or self.player:getHandcardNum() >= 5 then
			return false
		end
		local peaches = self:getCardsNum("Peach") + self:getCardsNum("Analeptic")
		for _, p in ipairs(self.friends_noself) do
			peaches = peaches + getKnownCard(p, self.player, "Peach", true)
		end
		if peaches > 0 then
			return true
		end
	end
	return false
end

local herong_skill = {}
herong_skill.name = "herong"
table.insert(sgs.ai_skills, herong_skill)
herong_skill.getTurnUseCard = function(self)
	local room = self.room
	if self.player:hasUsed("HerongCard") then return nil end
	if self.player:getHandcardNum() < 2 then return nil end
	if #self.enemies == 0 then return nil end
	local has_target = false
	for _, p in ipairs(self.enemies) do
		if self.player:inMyAttackRange(p) then
			has_target = true
			break
		end
	end
	if not has_target then return nil end
	
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	local card1, card2, maxvalue = 1000
	for i = 1, #cards - 1, 1 do
		local c1 = cards[i]
		if not c1:isKindOf("Peach") and not (c1:isKindOf("Jink") and self:getCardsNum("Jink") <= 3)
				and not (c1:isKindOf("Analeptic") and self:isWeak()) and not c1:isKindOf("ExNihilo")
				and not (c1:isKindOf("IronChain")) then
			for j = i + 1, #cards, 1 do
				local c2 = cards[j]
				if not c2:isKindOf("Peach") and not (c2:isKindOf("Jink") and self:getCardsNum("Jink") <= 3)
						and not (c2:isKindOf("Analeptic") and self:isWeak()) and not c2:isKindOf("ExNihilo")
						and not (c2:isKindOf("IronChain")) and c1:getNumber() + c2:getNumber() > 4 * (math.min(self.player:getMark("@nuclear") + 1, 4)) then
					card1 = c1
					card2 = c2
					maxvalue = self:getKeepValue(c1) + self:getKeepValue(c2)
				end
			end
		end
	end
	if card1 and card2 then
		local card_str = ("@HerongCard=%d+%d"):format(card1:getEffectiveId(), card2:getEffectiveId())
		return sgs.Card_Parse(card_str)
	end
end

sgs.ai_skill_use_func.HerongCard = function(card, use, self)
	local room = self.room
	local n = math.min(self.player:getMark("@nuclear") + 1, 4)
	self:sort(self.enemies, "hp")
	local targets = {}
	for _, p in ipairs(self.enemies) do
		if self.player:inMyAttackRange(p) and #targets < n and self:hasVine(p) then
			table.insert(targets, p)
			if #targets >= n then break end
		end
	end
	if #targets >= n then
		use.card = card
		if use.to then
			for _, p in ipairs(targets) do
				use.to:append(p)
			end
		end
		return
	end
	for _, p in ipairs(self.enemies) do
		if self.player:inMyAttackRange(p) and #targets < n and not self:hasVine(p) then
			table.insert(targets, p)
			if #targets >= n then break end
		end
	end
	if #targets > 0 then
		use.card = card
		if use.to then
			for _, p in ipairs(targets) do
				use.to:append(p)
			end
		end
		return
	end
end

sgs.ai_card_intention.HerongCard = 80
sgs.ai_use_priority.HerongCard = 7.2
sgs.ai_use_value.HerongCard = 9.3
sgs.dynamic_value.damage_card.HerongCard = true

sgs.ai_skill_discard.molian = function(self, discard_num, optional, include_equip)
	local current = self.room:getCurrent()
	local komachi = self.player
	if not self:isEnemy(current) then
		return "."
	end
	
	local both = komachi:hasFlag("MolianBoth")
	if current:getArmor() and current:getArmor():isKindOf("Vine") and not both then
		return "."
	end
	
	local cards = sgs.QList2Table(komachi:getCards("he"))
	self:sortByKeepValue(cards)
	local discards = {}
	if self:needToThrowArmor() then
		table.insert(discards, komachi:getArmor():getEffectiveId())
		return discards
	end
	for _, c in ipairs(cards) do
		if not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 2)
				and not (c:isKindOf("ExNihilo"))
				and not (c:isKindOf("Analeptic") and self:isWeak() and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 3)
				and not (c:isKindOf("Slash") and self:getCardsNum("Slash") < 2 and not (self:isVeryWeak(current) or both)) then
			table.insert(discards, c:getEffectiveId())
			return discards
		end
	end
	
	return discards
end

sgs.ai_skill_invoke.sijian = function(self, data)
	if not self.player:faceUp() then
		return true
	end
	if self:isVeryWeak() or (self:isWeak() and self.player:getHandcardNum() <= 2) then
		return false
	end
	if self:getCardsNum("Jink") == 0 and self.player:isWounded() then
		return false
	end
	return true
end

sgs.ai_skill_invoke.qionghu = function(self, data)
	if not self.player:faceUp() then
		return true
	end
	if self.player:getPhase() == sgs.Player_NotActive and self:isVeryWeak() then
		return false
	end
	if self.player:getPhase() == sgs.Player_NotActive then
		if self:isWeak() and self:getCardsNum("Jink") == 0 then
			return false
		end
		return true
	else
		if self:getCardsNum("Peach") > 0 or (self:getCardsNum("Analeptic") > 0 and not self.player:hasUsed("Analeptic")) then
			return true
		end
		if not self:isVeryWeak() and self:getCardsNum("Jink") == 0 then
			return true
		end
		return false
	end
end

sgs.ai_skill_use["@@yuzhi"] = function(self, prompt)
	local targets = {}
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isFriend(p) and (p:getCards("j"):length() > 0 or (self:hasLion(p) and p:isWounded())) then
			sgs.updateIntention(self.player, p, -80)
			table.insert(targets, p:objectName())
			if #targets == self.player:getLostHp() then
				break
			end
		end
	end
	if #targets == self.player:getLostHp() then
		return ("@YuzhiCard=.->%s"):format(table.concat(targets, "+"))
	end
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isEnemy(p) and not p:isNude() then
			sgs.updateIntention(self.player, p, 80)
			table.insert(targets, p:objectName())
			if #targets == self.player:getLostHp() then
				break
			end
		end
	end
	if #targets == 0 then return "." end
	return ("@YuzhiCard=.->%s"):format(table.concat(targets, "+"))
end

sgs.ai_skill_invoke.hongmo = true

function SmartAI:hasBig()
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:getNumber() >= 10 and not (c:isKindOf("ExNihilo") or c:isKindOf("Snatch") or c:isKindOf("Peach")) then
			return true
		end
	end
	return false
end

function SmartAI:hasMedium()
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:getNumber() >= 7 and not (c:isKindOf("ExNihilo") or c:isKindOf("Snatch") or c:isKindOf("Peach")) then
			return true
		end
	end
	return false
end

function SmartAI:hasSmall()
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:getNumber() <= 4 and not (c:isKindOf("ExNihilo") or c:isKindOf("Snatch") or c:isKindOf("Peach")) then
			return true
		end
	end
	return false
end

function SmartAI:needToLongzuan()
	if not self.player:hasSkill("longzuan") then return nil end
	
	self:sort(self.enemies, "hp")
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() and (self:isWeak(p) and ((p:getHandcardNum() <= 2 and self:hasMedium()) or self:hasBig()))
				and not (p:hasSkill("Zhuqu") and self.player:getHp() <= 2 and self.role == "regenade") then
			return p
		end
	end
	
	self:sort(self.friends_noself, "hp")
	for _, p in ipairs(self.friends_noself) do
		if not p:isKongcheng() and self:hasSmall() and (not self:isVeryWeak() or p:getRole() == "lord")
				and self:isWeak(p) and p:getHandcardNum() >= 3 then
			return p
		end
	end
	
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() and self:hasBig() then
			local cards = sgs.QList2Table(self.player:getHandcards())
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if not self:cardNeed(c) then
					return p
				end
			end
		end
	end
	
	return nil
end

sgs.ai_skill_use["@@longzuan"] = function(self, data)
	local target = self:needToLongzuan()
	if target == nil then return "." end
	return "@LongzuanCard=.->" .. target:objectName()
end

function sgs.ai_skill_pindian.longzuan(minusecard, self, requestor)
	if self:isFriend(requestor) and self.player:hasSkill("longzuan") and self.player:objectName() ~= requestor:objectName() then
		return self:getMinCard()
	end
	return self:getMaxCard()
end

sgs.ai_skill_choice.longzuan = function(self, choices)
	local room = self.room
	local target = room:getTag("LongzuanTarget"):toPlayer()
	if self:isFriend(target) then
		sgs.updateIntention(self.player, target, -80)
		return "LongzuanDraw"
	end
	return "Cancel"
end
--[[
sgs.ai_skill_use["@@longzuan"] = function(self, prompt)
	self:sort(self.enemies, "hp")
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() and (self:isWeak(p) and ((p:getHandcardNum() <= 2 and self:hasMedium()) or self:hasBig()))
				and not (p:hasSkill("Zhuqu") and self.player:getHp() <= 2 and self.role == "regenade") then
			sgs.updateIntention(self.player, p, 80)
			return ("#longzuan:.:->%s"):format(p:objectName())
		end
	end
	
	self:sort(self.friends_noself, "hp")
	for _, p in ipairs(self.friends_noself) do
		if not p:isKongcheng() and self:hasSmall() and (not self:isVeryWeak() or p:getRole() == "lord")
				and self:isWeak(p) and p:getHandcardNum() >= 3 then
			return ("#longzuan:.:->%s"):format(p:objectName())
		end
	end
	
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() and self:hasBig() then
			local cards = sgs.QList2Table(self.player:getHandcards())
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if not self:cardNeed(c) then
					sgs.updateIntention(self.player, p, 80)
					return ("#longzuan:.:->%s"):format(p:objectName())
				end
			end
		end
	end
	
	return false
end
]]--

sgs.ai_chaofeng.yukari = 4
sgs.ai_chaofeng.chen = 3
sgs.ai_chaofeng.keine = 4
sgs.ai_chaofeng.mokou = 3
sgs.ai_chaofeng.utsuho = 2
sgs.ai_chaofeng.erin = 2
sgs.ai_chaofeng.kaguya = -3
sgs.ai_chaofeng.ex_scarlets = 4
sgs.ai_chaofeng.ex_iku = 4
