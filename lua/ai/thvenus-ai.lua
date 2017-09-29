sgs.ai_skill_use["@@fanni"] = function(self, prompt)
	if #self.friends_noself + #self.enemies == 0 then
		return "."
	end

	local n = self.player:getHandcardNum()
	if self:isWeak() and n == 2 then n = 1 end

	local targets = {}

	self:sort(self.friends_noself, "hp")
	for _, p in ipairs(self.friends_noself) do
		if #targets < n and p:getHandcardNum() > self.player:getHandcardNum() and p:isWounded() and self:hasLion(p) then
			sgs.updateIntention(self.player, p, -80)
			table.insert(targets, p:objectName())
		end
	end

	self:sort(self.enemies, "chaofeng")
	for _, p in ipairs(self.enemies) do
		if #targets < n and p:getHandcardNum() > self.player:getHandcardNum() then
			sgs.updateIntention(self.player, p, 80)
			table.insert(targets, p:objectName())
		end
	end

	if #targets < n then
		for i = 1, math.min(n - #targets, #enemies), 1 do
			if not table.contains(targets, enemies[i]) then
				sgs.updateIntention(self.player, enemies[i], 80)
				table.insert(targets, enemies[i]:objectName())
			end
		end
	end

	if #targets == 0 then return "." end
	return "@FanniCard=.->" .. table.concat(targets, "+")
end

sgs.ai_skill_use["@@xiaoan"] = function(self, prompt)
	local move = self.player:getTag("XiaoanTag"):toMoveOneTime()
	local doom_night = sgs.Sanguosha:cloneCard("DoomNight", sgs.Card_SuitToBeDecided, 0)
	for _, id in sgs.qlist(move.card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isBlack() then
			doom_night:addSubcard(c)
		end
	end
	local targets = self:getDoomNightTargets(doom_night, self.player:getMark("xiaoan_targets"))
	if #targets > 0 then
		local target_names = {}
		for _, p in ipairs(targets) do
			table.insert(target_names, p:objectName())
		end
		return "@XiaoanCard=.->" .. table.concat(target_names, "+")
	end
	return "."
end

sgs.ai_skill_invoke.yueshi = function(self, data)
	local damage = data:toDamage()
	local from = damage.from

	--木牛底下的牌
	if self.player:hasTreasure("WoodenOx") then
		if damage.damage == 1 and self.player:getPile("wooden_ox"):length() >= 3 then return false end
		local n = 0
		local card_ids = self.player:getPile("wooden_ox")
		for _, id in sgs.qlist(card_ids) do
			local c = sgs.Sanguosha:getCard(id)
			if c:isKindOf("Peach") or c:isKindOf("Analeptic") then
				n = n + 1
			end
		end
		if damage.damage <= n then return false end
	end

	if damage.damage > 1 or damage.damage >= self.player:getHp() then return true end

	if self:getKeyEquipNum() > 0 then return true end

	self:sort(self.friends_noself, "hp")
	for _, p in ipairs(self.friends_noself) do
		if self:isWeakerThan(p, self.player) and ((getKnownCard(self.player, from, "Peach", true) > 0
				and getKnownCard(self.player, from, "Peach", true) >= self.player:getHandcardNum() - 1) or (self:getCardsNum("Peach") > 0
				and self:getCardsNum("Peach") == self.player:getHandcardNum())) then
			return false
		end
	end

	for _, c in ipairs(self:getVisibleCards(from)) do
		if c:isKindOf("Slash") and c:IsAvailable(from, c, true) and (self:hasHeavySlashDamage(from, c, self.player) or getKnownCard(from, self.player, "Analeptic", true))
				and self:getCardsNum("Jink") == 0 and c:targetFilter(sgs.PlayerList(), self.player, from) and not room:isProhibited(from, self.player, c) then
			return false
		end
	end

	return true
end

sgs.ai_skill_invoke.tannang = true

sgs.ai_skill_askforag.tannang = function(self, card_ids)
	local types = {}
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if not table.contains(types, c:getType()) then
			table.insert(types, c:getType())
		end
	end

	if #types == 1 then
		return card_ids[1]
	end

	local cards = {}
	local basics = {}
	local tricks = {}
	local equips = {}
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		table.insert(cards, c)
		if c:isKindOf("BasicCard") then
			table.insert(basics, c)
		elseif c:isKindOf("TrickCard") then
			table.insert(tricks, c)
		elseif c:isKindOf("EquipCard") then
			table.insert(equips, c)
		end
	end
	self:sortByKeepValue(cards)
	self:sortByKeepValue(basics)
	self:sortByKeepValue(tricks)
	self:sortByKeepValue(equips)

	return cards[1]:getEffectiveId()
end

sgs.ai_skill_use["@@lingbai"] = function(self, prompt)
	local n = self.player:getMark("lingbai")
	self:sort(self.friends, "defense")
	local targets = {}
	for _, p in ipairs(self.friends) do
		if self:isPriorTarget(p) and #targets < n then
			sgs.updateIntention(self.player, p, -80)
			table.insert(targets, p:objectName())
		end
	end
	if #targets < n and not table.contains(targets, self.player:objectName()) then
		sgs.updateIntention(self.player, p, -80)
		table.insert(targets, self.player:objectName())
	end
	for _, p in ipairs(self.friends) do
		if #targets < n and not table.contains(targets, p:objectName()) then
			sgs.updateIntention(self.player, p, -80)
			table.insert(targets, p:objectName())
		end
	end
	if #targets > 0 then
		return "@LingbaiCard=.->" .. table.concat(targets, "+")
	end
	return "."
end

sgs.ai_skill_use["@@jinghun"] = function(self, prompt)
	local use = self.player:getTag("JinghunUse"):toCardUse()
	local card = use.card
	local targets = {}
	if ("amazing_graze|god_salvation"):match(use.card:objectName()) then
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if self:isEnemy(p) and p:hasFlag("ValidJinghunTarget") and (p:getMark("@purified") == 3 or not self:isVeryWeak(p)) then
				table.insert(targets, p:objectName())
			end
		end
	else
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if p:hasFlag("ValidJinghunTarget") then
				if self:isEnemy(p) then
					if p:getMark("@purified") == 3 and p:getCards("he"):length() <= 1 then continue end
					if sgs.dynamic_value.damage_card[card:getClassName()] then
						if card:isKindOf("Slash") and self:hasHeavySlashDamage(self.player, card, p) then
							continue
						end
						if p:hasArmorEffect("Vine") and (card:isKindOf("FireSlash") or card:isKindOf("FireAttack") or card:isKindOf("PhoenixFlame"))
								and not self:notMeantToCauseEffect(card, self.player, p) then
							continue
						end
						if p:getHp() > 1 and (p:getMark("@purified") == 3 or not self:isVeryWeak(p)) then
							table.insert(targets, p:objectName())
						end
					elseif ("dismantlement|snatch|haze"):match(card:objectName()) then
						if not self:hasKeyEquip(p) and not (getKnownCard(p, self.player, "Peach", true) + getKnownCard(p, self.player, "Analeptic", true) >= p:getHandcardNum() - 1
								and not p:isKongcheng() and (p:isWounded() or self:hasWeakEnemy(false))) and (p:getMark("@purified") == 3 or not self:isVeryWeak(p)) then
							table.insert(targets, p:objectName())
						end
					elseif card:isKindOf("Collateral") then
						if p:getWeapon() and not table.contains(self:getKeyEquips(p), p:getWeapon()) and (p:getMark("@purified") == 3 or not self:isVeryWeak(p)) then
							table.insert(targets, p:objectName())
						end
					elseif (p:getMark("@purified") == 3 or not self:isVeryWeak(p)) then
						table.insert(targets, p:objectName())
					end
				elseif self:isFriend(p) then
					if p:getMark("@purified") == 3 and p:getCards("he"):length() <= 1 then
						table.insert(targets, p:objectName())
						continue
					end
					if sgs.dynamic_value.damage_card[card:getClassName()] then
						if p:getHp() == 1 then
							table.insert(targets, p:objectName())
						elseif p:getMark("@purified") < 3 or (p:isWounded() and p:getHandcardNum() <= 2 and not self:hasKeyEquip(p)
								and getKnownCard(p, self.player, "Peach", true) + getKnownCard(p, self.player, "Jink", true) <= 1) then
							table.insert(targets, p:objectName())
						end
					elseif ("dismantlement|snatch|haze"):match(card:objectName()) then
						if not ((self:hasLion(p) and p:isWounded()) or self:hasIndul(p) or self:hasShortage(p)) then
							if p:getMark("@purified") < 3 or (p:isWounded() and p:getHandcardNum() <= 2 and not self:hasKeyEquip(p)
									and getKnownCard(p, self.player, "Peach", true) + getKnownCard(p, self.player, "Jink", true) <= 1) then
								table.insert(targets, p:objectName())
							end
						end
					end
				elseif (p:getMark("@purified") == 3 or not self:isVeryWeak(p)) then
					table.insert(targets, p:objectName())
				end
			end
		end
	end

	if #targets > 0 then
		return "@JinghunCard=.->" .. table.concat(targets, "+")
	end
	return "."
end

sgs.ai_skill_invoke.jinghun = function(self, data)
	local player = self.player:getTag("JinghunRecoverTarget"):toPlayer()
	return self:isFriend(player)
end

sgs.ai_skill_use["@@caiyuan"] = function(self, prompt)
	self:sort(self.friends, "handcard", true)
	for _, p in ipairs(self.friends) do
		if p:getCards("he"):length() >= p:getMark("@purified") - 1 and p:getMark("@purified") > 0 and not (getKnownCard(p, self.player, "Peach", true)
				+ getKnownCard(p, self.player, "Analeptic", true) >= p:getHandcardNum() - 1 and p:getHandcardNum() > 2) then
			sgs.updateIntention(self.player, p, -80)
			return "@CaiyuanCard=.->" .. p:objectName()
		end
	end
	for _, p in ipairs(self.friends) do
		if not p:isNude() and p:getMark("@purified") > 0 then
			sgs.updateIntention(self.player, p, -80)
			return "@CaiyuanCard=.->" .. p:objectName()
		end
	end
	return "."
end

sgs.ai_skill_playerchosen.guangjie = function(self, targets)
	local targetslist = targets
	targets = sgs.QList2Table(targetslist)
	local use = self.player:getTag("GuangjieUse"):toCardUse()
	local card = use.card
	local from = use.from
	if not self.player:isNude() and ((sgs.getDefense(self.player) <= 5.25 and self:getDefenseCardsNum() <= 1) or (self:isVeryWeak() and self:hasKeyEquip())
			or (self:hasWeakFriend(true, true) and self:getCardsNum("Peach") > 0 and self.player:getHandcardNum() <= 3)) and not sgs.dynamic_value.benefit[card:getClassName()]
			and not (("duel|phoenix"):match(card:objectName()) and self:isEnemy(from)) then
		return false
	end
	if card:isKindOf("DoomNight") then
		if self:isFriend(from) then
			if from:isWounded() then
				self:sort(targets, "handcard")
			else
				self:sort(targets, "defense")
			end
			for _, p in ipairs(targets) do
				if self:isEnemy(p) and not p:hasArmorEffect("SilverLion") then
					return p
				end
			end
		else
			if from:getLostHp() >= 2 or (from:getLostHp() == 1 and use.to:at(0):getCards("he"):length() >= 4) then
				return nil
			end
			self:sort(targets, "defense")
			for _, p in ipairs(targets) do
				if self:isEnemy(p) and not p:hasArmorEffect("SilverLion") and sgs.getDefense(p) < 7 then
					return p
				end
			end
		end
	elseif card:isKindOf("FireAttack") and self:isEnemy(from) then
		self:sort(targets, "handcard")
		for _, p in ipairs(targets) do
			if self:isEnemy(p) and p:getHandcardNum() <= 2 and sgs.getDefense(p) < 7 then
				return p
			end
		end
	elseif card:isKindOf("PhoenixFlame") then
		if self:isFriend(from) then
			self:sort(targets, "handcard")
			for _, p in ipairs(targets) do
				if self:isEnemy(p) and getKnownCard(p, self.player, "Slash", true) == 0 and self:getVisibleCards(p):length() >= p:getHandcardNum() - 1 then
					return p
				end
			end
		elseif self:isEnemy(from) then
			self:sort(targets, "defense")
			for _, p in ipairs(targets) do
				if self:isEnemy(p) then
					return p
				end
			end
			self:sort(targets, "handcard", true)
			for _, p in ipairs(targets) do
				if self:isFriend(p) and getKnownCard(p, self.player, "Slash", true) > 0 then
					return p
				end
			end
		end
	elseif card:isKindOf("Duel") then
		if self:isFriend(from) then
			self:sort(targets, "handcard")
			for _, p in ipairs(targets) do
				if self:isEnemy(p) and getKnownCard(p, self.player, "Slash", true) == 0 and self:getVisibleCards(p):length() == p:getHandcardNum() then
					return p
				end
			end
		elseif self:isEnemy(from) then
			self:sort(targets, "defense")
			for _, p in ipairs(targets) do
				if self:isEnemy(p) then
					return p
				end
			end
		end
	elseif card:isKindOf("Slash") then
		self:sort(targets, "defense")
		for _, p in ipairs(targets) do
			if self:isEnemy(p) and p:getHp() <= getBestHp(p) and self:slashIsEffective(card, p, use.from) then
				return p
			end
		end
	elseif sgs.dynamic_value.damage_card[card:getClassName()] then
		self:sort(targets, "defense")
		for _, p in ipairs(targets) do
			if self:isEnemy(p) then
				return p
			end
		end
	elseif ("dismantlement|snatch|haze"):match(card:objectName()) then
		if self:isFriend(from) then
			local isDiscard = card:isKindOf("Dismantlement")
			local name = card:objectName()
			for _, p in ipairs(targets) do
				if self:isFriend(p) and p:isWounded() and p:hasArmorEffect("SilverLion") then
					return p
				end
			end
			for _, friend in ipairs(targets) do
				if not self:isFriend(friend) then continue end
				if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) and self:hasTrickEffective(card, friend) then
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
						return friend
					end
				end
			end
		elseif card:isKindOf("Dismantlement") then
			self:sort(targets, "defense")
			for _, p in ipairs(targets) do
				if self:isEnemy(p) and not (p:isWounded() and p:hasArmorEffect("SilverLion")) and not (p:containsTrick("indulgence") or p:containsTrick("supply_shortage")
						or p:containsTrick("lightning")) then
					return p
				end
			end
		end
	elseif card:isKindOf("IronChain") then
		self:sort(targets, "defense")
		for _, p in ipairs(targets) do
			if self:isFriend(p) and p:isChained() and p:hasArmorEffect("Vine") then
				return p
			end
		end
		for _, p in ipairs(targets) do
			if self:isFriend(p) and p:isChained() then
				return p
			end
		end
		for _, p in ipairs(targets) do
			if self:isEnemy(p) and not p:isChained() and p:hasArmorEffect("Vine") then
				return p
			end
		end
		for _, p in ipairs(targets) do
			if self:isEnemy(p) and not p:isChained() then
				return p
			end
		end
	elseif card:isKindOf("MindReading") then
		if self:isFriend(from) then
			self:sort(targets, "handcard", true)
			for _, p in ipairs(targets) do
				if self:isEnemy(p) then
					return p
				end
			end
		end
	elseif card:isKindOf("IcyFog") then
		if self:isFriend(from) then
			self:sort(targets, "hp")
			for _, p in ipairs(targets) do
				if self:isFriend(p) and p:isWounded() and p:hasArmorEffect("SilverLion") then
					return p
				end
			end
			for _, p in ipairs(targets) do
				if self:isEnemy(p) and not (p:isWounded() and p:hasArmorEffect("SilverLion") and p:getEquips():length() == 1) and self:hasKeyEquip(p) then
					return p
				end
			end
			for _, p in ipairs(targets) do
				if self:isEnemy(p) and not (p:isWounded() and p:hasArmorEffect("SilverLion") and p:getEquips():length() == 1) then
					return p
				end
			end
		end
	elseif card:isKindOf("ExNihilo") then
		return self:touhouFindPlayerToDraw(true, 2, targetslist)
	elseif card:isKindOf("Peach") then
		self:sort(targets, "hp")
		for _, p in ipairs(targets) do
			if self:isFriend(p) and p:getHp() < getBestHp(p) then
				return p
			end
		end
	end
	return nil
end

sgs.ai_skill_discard.guangjie1 = function(self, discard_num, optional, include_equip)
	if self.player:isNude() then
		return {}
	end
	local momiji = self.room:findPlayerBySkillName("guangjie")
	if not self:isFriend(momiji) then
		return {}
	end
	local use = self.player:getTag("GuangjieUse"):toCardUse()
	if use.from and use.from:isAlive() and self:isFriend(use.from, momiji) and self:needToThrowArmor(momiji) then
		return {}
	end
	if not self:isWeakerThan(momiji) or (sgs.isLordInDanger() and isLord(momiji)) or (self:isVeryWeak(momiji) and self:getDefenseCardsNum() < self:getCards("he"):length()) then
		return {self:chooseCardToThrow(true):getEffectiveId()}
	end
	return {}
end

sgs.ai_skill_discard.guangjie2 = function(self, discard_num, optional, include_equip)
	return {self:chooseCardToThrow(true):getEffectiveId()}
end

sgs.ai_skill_discard.guangjie3 = sgs.ai_skill_discard.guangjie2

sgs.ai_skill_invoke.langyan = function(self, data)
	self:sort(self.friends_noself, "handcard", true)
	local n = 0
	for _, p in ipairs(self.friends_noself) do
		if not self:isWeakerThan(p, self.player) then
			local cards = self:getVisibleCards(p)
			if self.player:getHandcardNum() - #cards >= 4 and not self:isVeryWeak() then
				n = n + 1
			else
				for _, c in ipairs(cards) do
					if c:isRed() and c:isKindOf("BasicCard") then
						n = n + 1
					end
				end
			end
		end
	end
	return n > 0
end

sgs.ai_skill_cardask["@langyan-give"] = function(self, pattern)
	local momiji = self.room:findPlayerBySkillName("langyan")
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if self:isFriend(momiji) then
		if self:isWeakerThan(momiji) then
			for _, c in ipairs(cards) do
				if c:isKindOf("BasicCard") and c:isRed() and (c:isKindOf("Slash") or self:getCardsNum(c:getClassName()) > 1 or c:isKindOf("Peach")) then
					return c:getEffectiveId()
				end
			end
		else
			for _, c in ipairs(cards) do
				if c:isKindOf("BasicCard") and c:isRed() then
					return c:getEffectiveId()
				end
			end
		end

		if momiji:hasWeapon("CrossBow") then
			for _, c in ipairs(cards) do
				if c:isKindOf("Slash") then
					return c:getEffectiveId()
				end
			end
		end

		if momiji:hasWeapon("Axe") then
			if getKnownCard(momiji, self.player, "Slash", true) == 0 then
				for _, c in ipairs(cards) do
					if c:isKindOf("Slash") then
						return c:getEffectiveId()
					end
				end
			elseif getKnownCard(momiji, self.player, "Analeptic", true) == 0 and not self:isVeryWeak() then
				for _, c in ipairs(cards) do
					if c:isKindOf("Analeptic") then
						return c:getEffectiveId()
					end
				end
			elseif momiji:getCards("he") <= 4 then
				for _, c in ipairs(cards) do
					if not (c:isKindOf("Analeptic") and self:isVeryWeak()) or self:getCardsNum(c:getClassName()) > 1 then
						return c:getEffectiveId()
					end
				end
			end
		end

		if momiji:hasArmorEffect("SilverLion") and momiji:isWounded() then
			if getKnownCard(momiji, self.player, "Armor", true) == 0 then
				for _, c in ipairs(cards) do
					if c:isKindOf("Armor") then
						return c:getEffectiveId()
					end
				end
			end
		end
	elseif self:isWeak() then
		local n = self.player:getTag("LangyanAlreadyNum"):toInt()
		if n == 0 then return "." end
		for _, c in ipairs(cards) do
			if c:isKindOf("Slash") and c:isRed() and not momiji:hasWeapon("CrossBow") then
				return c:getEffectiveId()
			end
		end
	end
	return "."
end

sgs.ai_skill_invoke.leixuan = function(self, data)
	if self.player:getHandcardNum() == self.player:getMaxCards() then
		if self:isWeak() and self:getDefenseCardsNum() == self.player:getHandcardNum() then
			return false
		end
		self:sort(self.enemies, "hp")
		for _, p in ipairs(self.enemies) do
			if sgs.getDefense(p) <= 6.5 or p:getHp() <= self.player:getHp() or (p:getHp() <= getBestHp(p) and p:getHandcardNum() <= math.max(0, 1 + p:getLostHp())) then
				return true
			end
		end
	elseif self.player:getHandcardNum() == self.player:getMaxCards() + 1 then
		local cards = self:getRuleDiscards(2, 2)
		local suits = {}
		local diff = true
		for _, id in ipairs(cards) do
			local c = sgs.Sanguosha:getCard(id)
			if table.contains(suits, c:getSuitString()) then
				diff = false
				break
			end
			table.insert(suits, c:getSuitString())
		end
		if diff then
			self:sort(self.enemies, "hp")
			for _, p in ipairs(self.enemies) do
				if sgs.getDefense(p) <= 6.5 or p:getHp() <= self.player:getHp() or (p:getHp() <= getBestHp(p) and p:getHandcardNum() <= math.max(0, 1 + p:getLostHp())) then
					return true
				end
			end
		end
		if self:getCardsNum("Peach") > 0 and self.player:getHp() >= 3 then
			return true
		end
	elseif self.player:getHandcardNum() > self.player:getMaxCards() + 1 then
		return true
	end
	return false
end

sgs.ai_skill_choice.leixuan = function(self, choices)
	if self.player:getHandcardNum() == self.player:getMaxCards() then
		return "LeixuanDown"
	elseif self.player:getHandcardNum() == self.player:getMaxCards() + 1 then
		local cards = self:getRuleDiscards(2, 2)
		local suits = {}
		local diff = true
		for _, id in ipairs(cards) do
			local c = sgs.Sanguosha:getCard(id)
			if table.contains(suits, c:getSuitString()) then
				diff = false
				break
			end
			table.insert(suits, c:getSuitString())
		end
		if diff then
			self:sort(self.enemies, "hp")
			for _, p in ipairs(self.enemies) do
				if sgs.getDefense(p) <= 6.5 or p:getHp() <= self.player:getHp() or (p:getHp() <= getBestHp(p) and p:getHandcardNum() <= math.max(0, 1 + p:getLostHp())) then
					return "LeixuanDown"
				end
			end
		end
		if self:getCardsNum("Peach") > 0 and self.player:getHp() >= 3 then
			return "LeixuanUp"
		end
	elseif self.player:getHandcardNum() > self.player:getMaxCards() + 1 then
		local n = self.player:getHandcardNum() - self.player:getMaxCards() + 1
		local jink = 0
		local peach = self:getCardsNum("Peach")
		if self:isVeryWeak() then peach = peach + self:getCardsNum("Analeptic") end
		if self:getCardsNum("Jink") > 0 then jink = 1 end
		if peach + jink >= self.player:getMaxCards() and self:hasWeakFriend(true, true) then
			return "LeixuanUp"
		end
		local null = 0
		if self:getCardsNum("Nullification") > 0 then null = 1 end
		if self:getCardsNum("Jink") + null >= self.player:getMaxCards() and self.player:getHp() == 1 then
			return "LeixuanUp"
		end
		local cards = self:getRuleDiscards(n, n)
		local suits = {}
		local diff = true
		for _, id in ipairs(cards) do
			local c = sgs.Sanguosha:getCard(id)
			if table.contains(suits, c:getSuitString()) then
				diff = false
				break
			end
			table.insert(suits, c:getSuitString())
		end
		if diff then
			self:sort(self.enemies, "hp")
			for _, p in ipairs(self.enemies) do
				if sgs.getDefense(p) <= 6.5 or p:getHp() <= self.player:getHp() or (p:getHp() <= getBestHp(p) and p:getHandcardNum() <= math.max(0, 1 + p:getLostHp())) then
					return "LeixuanDown"
				end
			end
		end
	end
	return "LeixuanUp"
end

sgs.ai_skill_playerchosen.leixuan = function(self, targets)
	if #self.enemies > 0 then
		sgs.updateIntention(self.player, self.enemies[1], 80)
		return self.enemies[1]
	end
	return nil
end

sgs.ai_skill_use["@@jingge"] = function(self, prompt)
	local suits = {}
	local to_discard = {}
	local cards = sgs.QList2Table(self.player:getHandcards())
	local tmp = {}
	self:sortByKeepValue(cards)
	local friendslist = sgs.SPlayerList()
	for _, p in ipairs(self.friends) do
		friendslist:append(p)
	end
	for _, c in ipairs(cards) do
		table.insert(tmp, c)
		if not table.contains(suits, "heart") and c:getSuitString() == "heart" then
			local lord = self.room:getLord()
			if (self:isFriend(lord) and sgs.isLordInDanger()) or (not c:isKindOf("ExNihilo")
						and not (c:isKindOf("Peach") and countCard(cards, "Peach") - countCard(to_discard, "Peach") <= 1 and self:isVeryWeak())
						and not (c:isKindOf("Jink") and countCard(cards, "Jink") - countCard(to_discard, "Jink") <= 1 and self:isWeak())) then
				self:sort(self.friends_noself, "hp")
				for _, p in ipairs(self.friends_noself) do
					if (p:isWounded() and p:getHp() < getBestHp(p)) or (p:objectName() == lord:objectName() and self:isFriend(lord) and sgs.isLordInDanger()) then
						table.insert(suits, "heart")
						table.insert(to_discard, c)
						break
					end
				end
			end
		elseif not table.contains(suits, "diamond") and c:getSuitString() == "diamond" then
			local lord = self.room:getLord()
			if (self:isFriend(lord) and sgs.isLordInDanger()) or (not c:isKindOf("ExNihilo") and not (c:isKindOf("Peach")
					and countCard(cards, "Peach") - countCard(to_discard, "Peach") <= 1)
					and not (c:isKindOf("Jink") and countCard(cards, "Jink") <= 1)) then
				if self:touhouFindPlayerToDraw(true, 1, friendslist) then
					table.insert(suits, "diamond")
					table.insert(to_discard, c)
				end
			end
		elseif not table.contains(suits, "club") and c:getSuitString() == "club" then
			self:sort(self.enemies, "hp")
			local dmg = 0
			for _, cc in ipairs(cards) do
				if table.contains(tmp, cc) then continue end
				if not table.contains(to_discard, cc) and (cc:isKindOf("Slash") or cc:isKindOf("Snatch") or cc:isKindOf("SupplyShortage")) then
					for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
						if self:isFriend(p) and cc:isKindOf("Snatch") and self.player:distanceTo(p) > 1 then
							if p:isWounded() and p:hasArmorEffect("SilverLion") and self:hasTrickEffective(cc, p) then
								dmg = dmg + 1
								break
							end
							if (p:containsTrick("indulgence") or p:containsTrick("supply_shortage")) and self:hasTrickEffective(cc, p) then
								local cardchosen
								tricks = p:getJudgingArea()
								for _, trick in sgs.qlist(tricks) do
									if trick:isKindOf("Indulgence") and (not isDiscard or self.player:canDiscard(p, trick:getId())) then
										if p:getHp() <= p:getHandcardNum() or p:isLord() or name == "snatch" then
											cardchosen = trick:getEffectiveId()
											break
										end
									end
									if trick:isKindOf("SupplyShortage") and (not isDiscard or self.player:canDiscard(p, trick:getId())) then
										cardchosen = trick:getEffectiveId()
										break
									end
									if trick:isKindOf("Indulgence") and (not isDiscard or self.player:canDiscard(p, trick:getId())) then
										cardchosen = trick:getEffectiveId()
										break
									end
								end
								if cardchosen then
									dmg = dmg + 1
									break
								end
							end
						elseif self:isEnemy(p) and not self.room:isProhibited(self.player, p, cc) then
							if cc:isKindOf("SupplyShortage") or cc:isKindOf("Snatch") and self.player:distanceTo(p) > 1 then
								dmg = dmg + 1
								break
							end
							if cc:isKindOf("Slash") and not self.player:inMyAttackRange(p) then
								dmg = dmg + 1
								break
							end
						end
					end
				end
			end
			if dmg > 0 then
				table.insert(suits, "club")
				table.insert(to_discard, c)
			end
		elseif not table.contains(suits, "spade") and c:getSuitString() == "spade" then
			self:sort(self.enemies, "defense")
			if c:isKindOf("Analeptic") and self:isVeryWeak() then continue end
			for _, p in ipairs(self.enemies) do
				if not p:isNude() and not (p:isKongcheng() and not self:hasKeyEquip(p)) then
					table.insert(suits, "club")
					table.insert(to_discard, c)
					break
				end
			end
		end
		tmp = {}
	end

	local discard_ids = {}
	for _, c in ipairs(to_discard) do
		table.insert(discard_ids, c:getEffectiveId())
	end
	return "@JinggeCard=" .. table.concat(discard_ids, "+")
end

sgs.ai_skill_playerchosen.jinggerecover = function(self, targets)
	targets = sgs.QList2Table(targets)
	local lord = self.room:getLord()
	if self:isFriend(lord) and sgs.isLordInDanger() and self.role ~= "lord" and table.contains(targets, lord) then
		sgs.updateIntention(self.player, lord, -100)
		return lord
	end
	self:sort(self.friends_noself, "hp")
	for _, p in ipairs(self.friends_noself) do
		if p:isWounded() and p:getHp() < getBestHp(p) and table.contains(targets, p) then
			sgs.updateIntention(self.player, lord, -90)
			return p
		end
	end
	return nil
end

sgs.ai_skill_playerchosen.jinggedraw = function(self, targets)
	local targetslist = targets
	targets = sgs.QList2Table(targetslist)
	local p = self:touhouFindPlayerToDraw(true, 1, targetslist)
	if p then
		sgs.updateIntention(self.player, p, -80)
	end
	return p
end

sgs.ai_skill_playerchosen.jinggelimit = function(self, targets)
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() then
			local visible_cards = self:getVisibleCards(p)
			local has_slash = false
			local has_jink = false
			local has_peach = false
			local has_null = false
			for _, cc in ipairs(visible_cards) do
				if cc:getSuitString() == "heart" then
					if cc:isKindOf("Slash") then
						has_slash = true
						break
					end
					if cc:isKindOf("Jink") then
						has_jink = true
						break
					end
					if cc:isKindOf("Peach") then
						has_peach = true
						break
					end
					if cc:isKindOf("Nullification") then
						has_null = true
						break
					end
				end
			end
			if ((has_slash or self:getCardsNum("Duel") + self:getCardsNum("PhoenixFlame") + self:getCardsNum("SavageAssault") == 0)
					and (has_jink or self:getCardsNum("Slash") + self:getCardsNum("ArcheryAttack") == 0)
					and (has_null or self:getCardsNum("TrickCard^DelayedTrick") - self:getCardsNum("Nullification") == 0)) or has_peach then
				continue
			end
			sgs.updateIntention(self.player, p, 80)
			return p
		end
	end
	return nil
end

sgs.ai_skill_invoke.sujuan = true

sgs.ai_skill_choice.sujuan = function(self, choices)
	if self:getCardsNum("ExNihilo") > 0 or (self:getCardsNum("Peach") > 0 and self.player:isWounded()) then
		return "red"
	end
	if self:getCardsNum("IronChain") > 0 and self.player:isChained() then
		return "black"
	end
	local n = math.random(2)
	if n == 1 then
		return "red"
	else
		return "black"
	end
end

local feiman_skill = {}
feiman_skill.name = "feiman"
table.insert(sgs.ai_skills, feiman_skill)
feiman_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("FeimanCard") then return nil end

	return sgs.Card_Parse("@FeimanCard=.")
end

sgs.ai_skill_use_func.FeimanCard = function(card, use, self)
	if #self.enemies == 0 then return end
	if not self:hasDamageCard() then return end
	self:sort(self.enemies, "defense")
	local may_use = false
	for _, p in ipairs(self.enemies) do
		if self.player:distanceTo(p) > 1 then
			may_use = true
			break
		end
	end
	if not may_use then return end
	for _, p in ipairs(self.enemies) do
		if p:getHandcardNum() == 1 then
			for _, c in sgs.qlist(self.player:getHandcards()) do
				if c:isKindOf("Slash") then
					if ((not p:hasArmorEffect("EightDiagram") or self.player:hasWeapon("QinggangSword")) or getKnownNum(p) > getKnownCard(p, self.player, "Jink", true))
							and self:slashIsEffective(c, p, self.player, self.player:hasWeapon("QinggangSword")) then
						use.card = card
						if use.to then use.to:append(p) end
						return
					end
				end
				if c:isKindOf("TrickCard") and sgs.dynamic_value.damage_card[c:getClassName()] and not c:isKindOf("FireAttack") then
					if self:hasTrickEffective(c, p, self.player) then
						use.card = card
						if use.to then use.to:append(p) end
						return
					end
				end
			end
		elseif p:getHandcardNum() == 2 then
			for _, c in sgs.qlist(self.player:getHandcards()) do
				if c:isKindOf("Slash") or c:isKindOf("ArcheryAttack") then
					if ((not p:hasArmorEffect("EightDiagram") or (self.player:hasWeapon("QinggangSword") and c:isKindOf("Slash")))
							or (getKnownNum(p) > 0 and getKnownCard(p, self.player, "Jink", true) == 0))
							and self:slashIsEffective(c, p, self.player, (self.player:hasWeapon("QinggangSword") and c:isKindOf("Slash"))) then
						use.card = card
						if use.to then use.to:append(p) end
						return
					end
				elseif ("duel|savage_assault|phoenix_flame"):match(c:objectName()) then
					if getKnownNum(p) > 0 and getKnownCard(p, self.player, "Slash", true) == 0 and self:hasTrickEffective(c, p, self.player) then
						use.card = card
						if use.to then use.to:append(p) end
						return
					end
				elseif c:isKindOf("NecroMusic") then
					if getKnownNum(p) > 0 and getKnownCard(p, self.player, "^BasicCard", true, "he") == 0 and self:hasTrickEffective(c, p, self.player) then
						use.card = card
						if use.to then use.to:append(p) end
						return
					end
				end
			end
		elseif p:isKongcheng() then
			if self:hasKeyEquip(p) and not (self:getKeyEquipNum(p) == 1 and self:getKeyEquips(p)[1]:isKindOf("SilverLion") and p:isWounded()) then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		elseif sgs.getDefense(p) <= 7.5 then
			if self.player:getHp() + self:getCardsNum("Peach") <= 2 and self:getDefenseCardsNum() <= 2 then
				return
			end
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
	end
end

sgs.ai_card_intention.FeimanCard = 80
sgs.ai_use_priority.FeimanCard = 7.6
sgs.ai_use_value.FeimanCard = 5.3
sgs.dynamic_value.control_card.FeimanCard = true

sgs.ai_skill_use["@@yuni"] = function(self, prompt)
	if self:getCardsNum("Peach") > 1 then return "." end
	local lord = self.room:getLord()
	if self:isFriend(lord) and not isLord(self.player) and sgs.isLordInDanger() then
		sgs.updateIntention(self.player, lord, -100)
		return "@YuniCard=.->" .. lord:objectName()
	end
	local n = math.min(self:getCardsNum("Jink"), self.player:getMaxCards() + self:getCardsNum("Peach")) * 0.7 + self:getCardsNum("Peach")
	if self:getCardsNum("Nullification") > 0 then n = n + 0.3 end
	if n < 2 then
		self:sort(self.friends, "hp")
		for _, p in ipairs(self.friends) do
			if p:isWounded() then
				sgs.updateIntention(self.player, p, -90)
				return "@YuniCard=.->" .. p:objectName()
			end
		end
	end
	return "."
end

sgs.ai_skill_playerchosen.daorang = function(self, targets)
	targets = sgs.QList2Table(targets)
	local judge = self.player:getTag("DaorangJudge"):toJudge()
	if not self:needRetrial(judge) then
		self:sort(targets, "handcard")
		for _, p in ipairs(targets) do
			if p:getHandcardNum() <= self.player:getHandcardNum() and self:isEnemy(p) and not self.player:isKongcheng() then
				sgs.updateIntention(self.player, p, 80)
				return p
			end
		end
	else
		self:sort(targets, "handcard", true)
		for _, p in ipairs(targets) do
			if (p:getCards("he"):length() >= self.player:getHandcardNum() + 3 or (p:getCards("he"):length() > self.player:getHandcardNum() and not self:isWeak(p))
					or self.player:isKongcheng()) and self:isFriend(p) then
				return p
			end
		end
		self:sort(targets, "handcard")
		for _, p in ipairs(targets) do
			if p:getHandcardNum() <= self.player:getHandcardNum() and self:isEnemy(p) and not self.player:isKongcheng() then
				return p
			end
		end
	end
	return nil
end

sgs.ai_skill_invoke.caizui = true

sgs.ai_skill_choice.caizui = function(self, choices)
	local shikieiki = self.room:findPlayerBySkillName("caizui")
	if self:isFriend(shikieiki) then
		return "CaizuiGain"
	end
	if self.player:getHp() > getBestHp(self.player) then
		return "CaizuiLose"
	end
	if self:willSkipPlayPhase(shikieiki) then
		return "CaizuiGain"
	end
	if self:getCardsNum("Jink") > 0 and self:getCardsNum("Peach") > 0 and self:getCardsNum("Jink") + self:getCardsNum("Peach") >= 3
			and not self.player:containsTrick("indulgence") and self.player:getHp() > 1 then
		return "CaizuiLose"
	end
	if self:getCardsNum("Jink") + self:getCardsNum("Peach") >= 2 and not self.player:containsTrick("indulgence") and self.player:getHp() >= 3 then
		return "CaizuiLose"
	end
	return "CaizuiGain"
end

local jiaomian_skill = {name = "jiaomian"}
table.insert(sgs.ai_skills, jiaomian_skill)
jiaomian_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("JiaomianProhibited") then return nil end

	return sgs.Card_Parse("@JiaomianCard=.")
end

sgs.ai_skill_use_func.JiaomianCard = function(card, use, self)
	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	for _, p in ipairs(players) do
		if p:hasFlag("JiaomianTargeted") or p:getMark("@felony") == 0 then
			continue
		end
		if self:isFriend(p) then
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
	end

	self:sort(players, "equip", true)
	for _, p in ipairs(players) do
		if p:hasFlag("JiaomianTargeted") or p:getMark("@felony") == 0 then
			continue
		end
		if self:isEnemy(p) and self:hasKeyEquip(p) then
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
	end

	self:sort(players, "defense")
	for _, p in ipairs(players) do
		if p:hasFlag("JiaomianTargeted") or p:getMark("@felony") == 0 then
			continue
		end
		if self:isEnemy(p) and not self.player:isKongcheng() then
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
	end
end

sgs.ai_use_priority.JiaomianCard = 10
sgs.ai_use_value.JiaomianCard = 8.8
sgs.dynamic_value.control_card.JiaomianCard = true

sgs.ai_skill_choice.jiaomian = function(self, choices)
	local n = 0
	local shikieiki = self.room:findPlayerBySkillName("jiaomian")
	if self:isFriend(shikieiki) then
		return "JiaomianDraw"
	end
	for _, p in ipairs(self.friends) do
		if p:getMark("@felony") > 0 and not p:hasFlag("JiaomianTargeted") then
			n = n + 1
		end
	end
	if n == 0 then
		return "JiaomianDraw"
	end
	if self:getDefenseCardsNum() > 0 then
		if self.player:getHp() <= 2 and self.player:getHp() <= getBestHp(self.player) then
			return "JiaomianDraw"
		end
		if (self.player:getHp() > getBestHp(self.player) or self.player:getHp() >= 4) and not self:hasOffensiveKeyEquip() then
			return "JiaomianProhibit"
		end
		for _, p in ipairs(self.friends) do
			if p:getMark("@felony") > 0 and not p:hasFlag("JiaomianTargeted") and self:isWeakerThan(p, self.player) then
				if (self:isVeryWeak(p) or (isLord(p) and sgs.isLordInDanger())) and self:getCardsNum("Peach") > 0 then
					return "JiaomianDraw"
				end
				if self.player:getHp() >= 3 or (self.player:getHp() >= 2 and shikieiki:getHandcardNum() <= 3) then
					return "JiaomianProhibit"
				end
			end
		end
	else
		if not self:hasKeyEquip() or (not self:hasOffensiveKeyEquip() and (self.player:getHp() > getBestHp(self.player) or self.player:getHp() >= 3)) then
			return "JiaomianProhibit"
		end
	end
	return "JiaomianDraw"
end

sgs.ai_skill_invoke.zhenling = true

sgs.ai_skill_choice.zhenling = function(self, choice)
	if self.player:getHp() < self.player:getMaxHp() - 1 then return "recover" end
	if self.player:isWounded() and (self.player:getHandcardNum() > self.player:getHp() or (self.player:getHandcardNum() >= self.player:getHp() - 1
			and self.player:containsTrick("indulgence"))) then
		return "recover"
	end
	return "draw"
end

local yongye_skill = {name = "yongye"}
table.insert(sgs.ai_skills, yongye_skill)
yongye_skill.getTurnUseCard = function(self)
	if self.player:getMark("@evernight") == 0 then return nil end
	if self.player:isKongcheng() then return nil end
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if not c:isBlack() then
			return nil
		end
	end
	if not self.player:isWounded() and not self:hasWeakEnemy() then return nil end
	return sgs.Card_Parse("@YongyeCard=.")
end

sgs.ai_skill_use_func.YongyeCard = function(card, use, self)
	local targets = {}
	local target_num = self.player:getHandcardNum()

	self:sort(self.friends_noself, "defense")
	for _, p in ipairs(self.friends_noself) do
		if p:isWounded() and self:hasLion(p) and #targets < target_num then
			sgs.updateIntention(self.player, p, -80)
			table.insert(targets, p)
		end
		if #targets >= target_num then
			use.card = card
			if use.to then
				for _, p in ipairs(targets) do
					use.to:append(p)
				end
			end
			return
		end
	end

	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if self:isWeak(p) and not p:isNude() and #targets < target_num then
			sgs.updateIntention(self.player, p, -80)
			table.insert(targets, p)
		end
		if #targets >= target_num then
			use.card = card
			if use.to then
				for _, p in ipairs(targets) do
					use.to:append(p)
				end
			end
			return
		end
	end

	self:sort(self.enemies, "handcard")
	for _, p in ipairs(self.enemies) do
		if p:hasSkills(sgs.cardneed_skill) and not p:isNude() and #targets < target_num then
			sgs.updateIntention(self.player, p, -80)
			table.insert(targets, p)
		end
		if #targets >= target_num then
			use.card = card
			if use.to then
				for _, p in ipairs(targets) do
					use.to:append(p)
				end
			end
			return
		end
	end

	self:sort(self.enemies, "equip")
	for _, p in ipairs(self.enemies) do
		if (self:hasDefensiveKeyEquip(p) or (p:hasSkills(sgs.need_equip_skill) and not p:getEquips():isEmpty())) and not p:isNude() and #targets < target_num then
			sgs.updateIntention(self.player, p, -80)
			table.insert(targets, p)
		end
		if #targets >= target_num then
			use.card = card
			if use.to then
				for _, p in ipairs(targets) do
					use.to:append(p)
				end
			end
			return
		end
	end

	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if not p:isNude() and #targets < target_num then
			sgs.updateIntention(self.player, p, -80)
			table.insert(targets, p)
		end
		if #targets >= target_num then
			use.card = card
			if use.to then
				for _, p in ipairs(targets) do
					use.to:append(p)
				end
			end
			return
		end
	end
end

sgs.ai_use_priority.YongyeCard = 6.5
sgs.ai_use_value.YongyeCard = 8.2
sgs.dynamic_value.control_card.YongyeCard = true

sgs.ai_skill_cardask.yiyue = function(self, data)
	if self.player:isKongcheng() then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if self:allCardsSameColor() then
		return cards[1]:getEffectiveId()
	end

	local black = 0
	for _, e in ipairs(self:getKeyEquips()) do
		if e:isBlack() then
			black = black + 0.34
		end
	end
	black = black - self.player:getLostHp() * 0.27
	black = black + self.player:getCards("he"):length() * 0.11 - 0.63
	black = black + self:getBlackCards(true) * 0.09
	if black > 0 then
		for _, c in ipairs(cards) do
			if c:isBlack() then
				return c:getEffectiveId()
			end
		end
	else
		for _, c in ipairs(cards) do
			if c:isRed() then
				return c:getEffectiveId()
			end
		end
	end
	return "."
end

local suozhen_skill = {}
suozhen_skill.name = "suozhen"
table.insert(sgs.ai_skills, suozhen_skill)
suozhen_skill.getTurnUseCard = function(self)
	local room = self.room
	if #self.enemies == 0 then return nil end
	self:sort(self.enemies, "defense")
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf("SilverLion") and self.player:isWounded() then
			local card_str = ("@SuozhenCard=%d"):format(c:getEffectiveId())
			return sgs.Card_Parse(card_str)
		end
	end
	local x = 0
	for _, p in sgs.qlist(room:getAlivePlayers()) do
		x = x + p:getMark("@shuttle")
	end
	for _, c in ipairs(cards) do
		if ((self:getUseValue(c) <= 5.5) or (self.player:getMark("queji") == 0 and x < 3 and self:getUseValue(c) <= 8.3))
				and not c:isKindOf("BasicCard") then
			local card_str = ("@SuozhenCard=%d"):format(c:getEffectiveId())
			return sgs.Card_Parse(card_str)
		end
	end
	return nil
end

sgs.ai_skill_use_func.SuozhenCard = function(card, use, self)
	local room = self.room
	self:sort(self.enemies, "chaofeng")
	for _, p in ipairs(self.enemies) do
		if p:getMark("@shuttle") == 0 then
			use.card = card
			if use.to then
				use.to:append(p)
			end
			return
		end
	end
	for _, p in ipairs(self.enemies) do
		if self:isWeak(p) then
			use.card = card
			if use.to then
				use.to:append(p)
			end
			return
		end
	end
	for _, p in ipairs(self.enemies) do
		if p:getHandcardNum() >= 4 then
			use.card = card
			if use.to then
				use.to:append(p)
			end
			return
		end
	end
	use.card = card
	if use.to then
		use.to:append(self.enemies[1])
	end
	return
end

sgs.ai_use_priority.SuozhenCard = 5.7
sgs.ai_use_value.SuozhenCard = 8.2
sgs.ai_card_intention.SuozhenCard = 80
sgs.dynamic_value.control_card.SuozhenCard = true

sgs.ai_skill_invoke.suozhen = function(self, data)
	local damage = data:toDamage()
	if not self:isEnemy(damage.from) then
		return false
	end
	if self.player:getMark("queji") == 0 then
		local n = 0
		for _, p in sgs.qlist(self.player:getAlivePlayers()) do
			n = n + p:getMark("@shuttle")
		end
		if n <= 3 and not damage.from:getHp() == 1 then
			return false
		end
	end
	return true
end

sgs.ai_skill_invoke.gelong = true

sgs.ai_skill_invoke.gelong_mark = function(self, data)
	local current = self.room:getCurrent()
	if self:isEnemy(current) then
		sgs.updateIntention(self.player, current, 80)
		return true
	end
	return false
end

sgs.ai_chaofeng.seijya = 4
sgs.ai_chaofeng.rumia = -2
sgs.ai_chaofeng.nazrin = 4
sgs.ai_chaofeng.shyou = 3
sgs.ai_chaofeng.momiji = 7
sgs.ai_chaofeng.tokiko = 3
sgs.ai_chaofeng.sekibanki = 2
sgs.ai_chaofeng.shikieiki = 4
sgs.ai_chaofeng.ex_kaguya = 5
