function SmartAI:getDoomNightTargets(card, target_num, skill_name)
	skill_name = skill_name or "."
	local friends = {}
	local enemies = {}

	for _, p in ipairs(self.friends_noself) do
		if not self.room:isProhibited(self.player, p, card) and card:targetFilter(sgs.PlayerList(), p, self.player) then
			table.insert(friends, p)
		end
	end
	for _, p in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, p, card) and card:targetFilter(sgs.PlayerList(), p, self.player) then
			table.insert(enemies, p)
		end
	end

	if #friends + #enemies == 0 then return {} end

	self:sort(friends, "hp")
	self:sort(enemies, "handcard")
	
	targets = {}
	
	for _, p in ipairs(friends) do
		if self:needToThrowArmor(p) and #targets < target_num and skill_name ~= "youwang" then
			table.insert(targets, p)
			if #targets >= target_num then
				return targets
			end
		end
	end
	
	if self.player:isWounded() then
		for _, p in ipairs(enemies) do
			if (p:getCards("he"):length() == 1 or (p:getCards("he"):length() <= self.player:getLostHp() + 1)) and #targets < target_num then
				table.insert(targets, p)
				if #targets >= target_num then
					return targets
				end
			end
		end
		
		self:sort(friends, "handcard", true)
		if self:isWeak() then
			for _, p in ipairs(friends) do
				if not p:isNude() and getKnownCard(p, self.player, "Peach", true, "he") ~= p:getCards("he"):length() and not self:isWeak(p) and #targets < target_num
						and skill_name ~= "youwang" then
					table.insert(targets, p)
					if #targets >= target_num then
						return targets
					end
				end
			end
		end
		
		for _, p in ipairs(enemies) do
			if not p:isNude() and getKnownCard(p, self.player, "Peach", true, "he") >= p:getCards("he"):length() - 1 and #targets < target_num then
				table.insert(targets, p)
				if #targets >= target_num then
					return targets
				end
			end
		end
		
		self:sort(enemies, "defense")
		for _, p in ipairs(enemies) do
			if not p:isNude() and self:isWeak(p) and not self:needToThrowArmor(p) and #targets < target_num then
				table.insert(targets, p)
				if #targets >= target_num then
					return targets
				end
			end
		end
		
		self:sort(enemies, "chaofeng")
		for _, p in ipairs(enemies) do
			if not p:isNude() and #targets < target_num then
				table.insert(targets, p)
				if #targets >= target_num then
					return targets
				end
			end
		end
	end
	
	for _, p in ipairs(enemies) do
		if not p:isNude() and getKnownCard(p, self.player, "Peach", true, "he") >= p:getCards("he"):length() - 1 and #targets < target_num then
			table.insert(targets, p)
			if #targets >= target_num then
				return targets
			end
		end
	end
	
	self:sort(enemies, "defense")
	for _, p in ipairs(enemies) do
		if not p:isNude() and self:isWeak(p) and not self:needToThrowArmor(p) and #targets < target_num then
			table.insert(targets, p)
			if #targets >= target_num then
				return targets
			end
		end
	end

	return targets
end

function SmartAI:useCardDoomNight(card, use)
	local friends = self.friends_noself
	local enemies = self.enemies
	self:sort(friends, "hp")
	self:sort(enemies, "handcard")
	
	local targets_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card)
	local targets = self:getDoomNightTargets(card, targets_num)

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

sgs.ai_skill_discard.DoomNight = function(self, discard_num, optional, include_equip)
	discards = {}
	if self:needToThrowArmor() then
		table.insert(discards, self.player:getArmor():getEffectiveId())
		return discards
	end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	table.insert(discards, cards[1]:getEffectiveId())
	return discards
end

sgs.ai_skill_discard.DoomNightExtra = function(self, discard_num, optional, include_equip)
	discards = {}
	if self:needToThrowArmor() then
		table.insert(discards, self.player:getArmor():getEffectiveId())
		return discards
	end
	from = self.player:getTag("DoomNightSource"):toPlayer()
	if not from or not from:isAlive() or not from:isWounded() or self:isFriend(from) then
		return {}
	end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if (not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 2 and self:isWeak())
				and not (c:isKindOf("Analeptic") and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 3 and self:isVeryWeak()))
				or (not self:isWeak() and from and from:isAlive() and self:isVeryWeak(from))
				then
			table.insert(discards, c:getEffectiveId())
			return discards
		end
	end
	return {}
end

sgs.ai_card_intention.DoomNight = 0

sgs.dynamic_value.control_card.DoomNight = true
sgs.ai_use_value.DoomNight = 4.2
sgs.ai_keep_value.DoomNight = 5.7
sgs.ai_use_priority.DoomNight = 7

function SmartAI:useCardOracle(card, use)
	if #self.friends_noself == 0 then return end

	local lord = self.room:getLord()
	if self:isFriend(lord) and self.player:objectName() ~= lord:objectName() and sgs.isLordInDanger() then
		use.card = card
		if use.to then use.to:append(lord) end
		return
	end

	for _, p in ipairs(self.friends_noself) do
		self.room:setPlayerMark(p, "oracle_value", 1000)
		local n = 0
		local np = self.player
		while (np:objectName() ~= p:objectName()) do
			np = np:getNextAlive()
			n = n + 1
		end
		local marks = (3 - (p:getHp() + p:getHandcardNum() - p:getMaxHp() - 1)) * 16 - n * 11
		if marks > 0 then
			self.room:addPlayerMark(p, "oracle_value", marks)
		elseif marks < 0 then
			self.room:removePlayerMark(p, "oracle_value", -marks)
		end
	end

	local maxv = 0
	local player
	for _, p in ipairs(self.friends_noself) do
		if p:getMark("oracle_value") > maxv then
			maxv = p:getMark("oracle_value")
			player = p
		end
	end
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		self.room:setPlayerMark(p, "oracle_value", 0)
	end
	if player then
		use.card = card
		if use.to then use.to:append(player) end
		return
	end
end

sgs.ai_card_intention.Oracle = -100
sgs.ai_use_priority.Oracle = 0.5
sgs.ai_use_value.Oracle = 8.0
sgs.dynamic_value.benefit.Oracle = true
sgs.ai_keep_value.Oracle = 3.6
sgs.ai_judge_model.oracle = function(self, who)
	local judge = sgs.JudgeStruct()
	judge.who = who
	judge.pattern = ".|diamond"
	judge.good = true
	judge.reason = "oracle"
	return judge
end
