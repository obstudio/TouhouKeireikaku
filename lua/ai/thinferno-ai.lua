function SmartAI:findPhoenixFlameTargets(phoenix_flame, skill_name)
	skill_name = skill_name or "."
	local targets = {}
	local final_targets = {}
	if #self.enemies > 0 then
		self:sort(self.enemies, "handcard")
		local jinks = self:getCardsNum("Jink")
		for _, p in ipairs(self.enemies) do
			local phoenix_flame_to_flandre = true
			if p:hasSkill("wosui") and self.player:getHp() >= p:getHp() then
				if getKnownCard(p, self.player, "Slash", true) > 0 then phoenix_flame_to_flandre = false end
				if not p:isKongcheng() or p:getPile("wooden_ox"):length() > 0 then
					local all_num = p:getHandcardNum() + p:getPile("wooden_ox"):length()
					local visible_num = 0
					local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), p:objectName())
					for _, c in sgs.qlist(p:getHandcards()) do
						if (c:hasFlag("visible") or c:hasFlag(flag)) and not c:isKindOf("Slash") then
							visible_num = visible_num + 1
						end
					end
					for _, id in sgs.qlist(p:getPile("wooden_ox")) do
						local c = sgs.Sanguosha:getCard(id)
						if (c:hasFlag("visible") or c:hasFlag(flag)) and not c:isKindOf("Slash") then
							visible_num = visible_num + 1
						end
					end
					if visible_num < all_num then phoenix_flame_to_flandre = false end
				end
			end
			if (jinks - #targets > 0 or ((p:getHandcardNum() <= 2 or (p:getHandcardNum() <= 2 + self.player:getLostHp() and skill_name == "youwang")) and not self:isWeak()))
					and not (jinks - #targets == 1 and ((p:getHandcardNum() > 2 and skill_name ~= "youwang") or p:getHandcardNum() > 2 + self.player:getLostHp())
					and self.player:getHp() <= 3) and not self.room:isProhibited(self.player, p, phoenix_flame) and phoenix_flame:targetFilter(sgs.PlayerList(), p, self.player)
					and not self:needDeath(p) and not self:cantbeHurt(p, self.player) and phoenix_flame_to_flandre then
				table.insert(targets, p)
			end
		end
	end
	if #targets > 0 then
		local max_num = math.min(2 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, phoenix_flame), #targets)
		self:sort(targets, "hp")
		for _, p in ipairs(targets) do
			table.insert(final_targets, p)
			if #final_targets >= max_num then
				return final_targets
			end
		end
	end
	return final_targets
end

function SmartAI:useCardPhoenixFlame(phoenix_flame, use)
	if self.player:hasSkill("wuyan") and not self.player:hasSkill("jueqing") then return end
	if self.player:hasSkill("noswuyan") then return end

	local targets = self:findPhoenixFlameTargets(phoenix_flame, ".")
	if #targets > 0 then
		use.card = phoenix_flame
		if use.to then
			for _, p in ipairs(targets) do
				use.to:append(p)
			end
		end
		return
	end
end

sgs.ai_skill_cardask["phoenix_flame-slash"] = function(self, data, pattern, target2, target, prompt)
	for _, c in ipairs(self:getCards("Slash")) do
		if not self:slashProhibit(c, target, self.player) then
			return c:toString()
		end
	end
	return "."
end

sgs.ai_card_intention.PhoenixFlame = 80
sgs.ai_use_value.PhoenixFlame = 4.1
sgs.ai_keep_value.PhoenixFlame = 3.17
sgs.ai_use_priority.PhoenixFlame = 3.8

sgs.dynamic_value.damage_card.PhoenixFlame = true

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

sgs.weapon_range.AzraelScythe = 3

function sgs.ai_weapon_value.AzraelScythe(self, enemy)
	if not enemy then return end
	if enemy:getHandcardNum() > enemy:getHp() then return 4 end
end

sgs.ai_skill_invoke.AzraelScythe = function(self, data)
	local target = self.player:getTag("AzraelTarget"):toPlayer()
	if self:isEnemy(target) then
		return true
	end
	return false
end

sgs.ai_use_priority.AzraelScythe = 2.655

sgs.ai_skill_invoke.Yasakaninomagatama = function(self, data)
	if self.player:hasSkill("yuanqi") then
		local damage = data:toDamage()
		return (damage.damage >= self.player:getHp())
	end
	return true
end

sgs.ai_use_priority.Yasakaninomagatama = 2.75
sgs.ai_use_value.Yasakaninomagatama = 5.5
