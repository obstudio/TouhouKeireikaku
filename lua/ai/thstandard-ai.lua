local fengmo_skill = {name = "fengmo"}
table.insert(sgs.ai_skills, fengmo_skill)
fengmo_skill.getTurnUseCard = function(self)
	local reimu = self.player
	if reimu:getHandcardNum() < 2 and reimu:getMark("@spell") <= 0 then return nil end
	if reimu:hasFlag("FengmoProhibited") then return nil end
	local all_used = true
	for _, p in sgs.qlist(self.room:getOtherPlayers(reimu)) do
		if not p:hasFlag("FengmoTargeted") and (p:getHandcardNum() >= 2 or p:getMark("@spell") > 0) then
			all_used = false
			break
		end
	end
	if all_used then return nil end
	if #self.enemies == 0 then return nil end

	local decide = "none"

	local cards = sgs.QList2Table(reimu:getHandcards())
	self:sortByKeepValue(cards)
	local card_str = "."
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if p:hasFlag("FengmoTargeted") or (p:getHandcardNum() < 2 and p:getMark("@spell") <= 0) then
			continue
		end
		if p:getHandcardNum() < 4 and p:getHp() < 4 then
			local slash
			for _, c in sgs.qlist(reimu:getHandcards()) do
				if c:isKindOf("Slash") then
					slash = c
					break
				end
			end
			if (slash and reimu:needNoSpell(slash)) or not reimu:inMyAttackRange(p)
				or slash == nil or reimu:getMark("@spell") > 1 then
				card_str = "@FengmoCard=."
				break
			end
		end
		if p:getMark("@spell") <= 0 then
			local card1 = -1
			local card2 = -1
			local peaches = self:getCardsNum("Peach")
			local jinks = self:getCardsNum("Jink")
			local anas = self:getCardsNum("Analeptic")
			for _, c in ipairs(cards) do
				if not (c:isKindOf("Peach") and ((peaches <= 3 and self:hasWeakFriend()) or peaches <= 2))
					and not (c:isKindOf("Jink") and jinks <= 1)
					and not (c:isKindOf("Analeptic") and self:isWeak() and peaches + anas <= 2)
					and not (c:objectName():match("ex_nihilo|mind_reading|iron_chain|dismantlement|snatch|haze")) then
					if (card1 < 0) then
						card1 = c:getEffectiveId()
						if c:isKindOf("Peach") then
							peaches = peaches - 1
						elseif c:isKindOf("Jink") then
							jinks = jinks - 1
						elseif c:isKindOf("Analeptic") then
							anas = anas - 1
						end
					elseif (card1 > 0) then
						card2 = c:getEffectiveId()
						break
					end
				end
			end
			if card1 > 0 and card2 > 0 then
				card_str = ("@FengmoCard=%d+%d"):format(card1, card2)
				break
			end
		end
		if self:isWeak(p) and p:getHandcardNum() >= 2 then
			if reimu:getMark("@spell") > 0 then
				card_str = "@FengmoCard=."
				break
			end
		end
		if reimu:getMark("@spell") > self:getCardsNum("Slash") or not self:canAttack(p, reimu) or not reimu:inMyAttackRange(p) then
			card_str = "@FengmoCard=."
			break
		end
	end
	if card_str ~= "." then
		return sgs.Card_Parse(card_str)
	end
	return nil
end

sgs.ai_skill_use_func.FengmoCard = function(card, use, self)
	local reimu = self.player
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if p:hasFlag("FengmoTargeted") or (p:getHandcardNum() < 2 and p:getMark("@spell") <= 0) then
			continue
		end
		if p:getHandcardNum() < 4 and p:getHp() < 4 then
			local slash
			for _, c in sgs.qlist(reimu:getHandcards()) do
				if c:isKindOf("Slash") then
					slash = c
					break
				end
			end
			if (slash and reimu:needNoSpell(slash)) or not reimu:inMyAttackRange(p)
				or slash == nil or reimu:getMark("@spell") > 1 then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
		if p:getMark("@spell") <= 0 and card:getSubcards():length() == 2 then
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
		if self:isWeak(p) and p:getHandcardNum() >= 2 then
			if reimu:getMark("@spell") > 0 then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
		if reimu:getMark("@spell") > self:getCardsNum("Slash") or not self:canAttack(p, reimu) or not reimu:inMyAttackRange(p) then
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
	end
end

sgs.ai_card_intention.FengmoCard = 80
sgs.ai_use_priority.FengmoCard = 4.2
sgs.ai_use_value.FengmoCard = 6.7
sgs.dynamic_value.control_card.FengmoCard = true

sgs.ai_skill_discard.fengmo2 = function(self, discard_num, optional, include_equip)
	if self.player:getHandcardNum() < 2 then return {} end
	local discards = {}
	local reimu_choice = self.player:getTag("FengmoReimuChoice"):toString()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if reimu_choice == "LoseSpell" then
		local peaches = self:getCardsNum("Peach")
		local jinks = self:getCardsNum("Jink")
		local anas = self:getCardsNum("Analeptic")
		for _, c in ipairs(cards) do
			if not (c:isKindOf("Peach") and ((peaches <= 3 and self:hasWeakFriend()) or peaches <= 2))
				and not (c:isKindOf("Jink") and jinks <= 1)
				and not (c:isKindOf("Analeptic") and self:isWeak() and peaches + anas <= 2) then
				table.insert(discards, c:getEffectiveId())
				if (#discards == 1) then
					if c:isKindOf("Peach") then
						peaches = peaches - 1
					elseif c:isKindOf("Jink") then
						jinks = jinks - 1
					elseif c:isKindOf("Analeptic") then
						anas = anas - 1
					end
				elseif (#discards == 2) then
					break
				end
			end
		end
		if #discards == 2 then return discards end
		return {}
	elseif reimu_choice == "Discard" then
		if self.player:getMark("@spell") > 1 then return {} end
		self:sort(self.friends_noself, "defense")
		for _, p in ipairs(self.friends_noself) do
			if self:isWeak(p) and not p:hasFlag("FengmoTargeted") then
				return {}
			end
		end
		if self:isWeak() then return {} end
		local peaches = self:getCardsNum("Peach")
		local jinks = self:getCardsNum("Jink")
		local anas = self:getCardsNum("Analeptic")
		for _, c in ipairs(cards) do
			if not (c:isKindOf("Peach") and ((peaches <= 3 and self:hasWeakFriend()) or peaches <= 2))
				and not (c:isKindOf("Jink") and jinks <= 1)
				and not (c:isKindOf("Analeptic") and self:isWeak() and peaches + anas <= 2)
				and not (c:isKindOf("ExNihilo"))
				and not (c:isKindOf("Haze"))
				and not (c:isKindOf("Snatch"))
				and not (c:isKindOf("Indulgence")) then
				table.insert(discards, c:getEffectiveId())
				if (#discards == 1) then
					if c:isKindOf("Peach") then
						peaches = peaches - 1
					elseif c:isKindOf("Jink") then
						jinks = jinks - 1
					elseif c:isKindOf("Analeptic") then
						anas = anas - 1
					end
				elseif (#discards == 2) then
					break
				end
			end
		end
		if #discards == 2 then return discards end
		return {}
	end
	return {}
end

sgs.ai_skill_invoke.guayu = function(self, data)
	local reimu = self.room:getLord()
	if self:isFriend(reimu) then return true end
	local slash
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isKindOf("Slash") then
			slash = c
			break
		end
	end
	if self.player:getMark("@spell") > 0 and (self:getCardsNum("Slash") <= 0 or (slash and self.player:needNoSpell(slash))
		or (slash and not self:hasSlashTarget(slash))) then
		return true
	end
	return false
end

sgs.ai_skill_choice.guayu = function(self, choices)
	if #self.enemies == 0 then return "GYGive" end
	local slash
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isKindOf("Slash") then
			slash = c
			break
		end
	end
	if self.player:getMark("@spell") > 0 and (self:getCardsNum("Slash") <= 0 or (slash and self.player:needNoSpell(slash))
		or (slash and not self:hasSlashTarget(slash))) then
		return "GYGive"
	end
	return "GYAdd"
end

sgs.ai_skill_invoke.xingchen = function(self, data)
	if #self:getEnemies() == 0 then return false end
	local damage = data:toDamage()
	for _, p in ipairs(self:getEnemies()) do
		if damage.to:distanceTo(p) <= damage.to:getAttackRange() and p:objectName() ~= damage.to:objectName()
				and not p:hasFlag("XingchenSlashed") then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.xingchen = function(self, targets)
	for _, p in sgs.qlist(targets) do
		if self:isEnemy(p) and not p:hasFlag("XingchenSlashed") then
			sgs.updateIntention(self.player, p, 80)
			return p
		end
	end
end

sgs.ai_view_as.sheyue = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_PlaceSpecial and card:getSuit() == sgs.Card_Spade and not card:hasFlag("using") then
		return ("slash:sheyue[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local sheyue_skill = {}
sheyue_skill.name = "sheyue"
table.insert(sgs.ai_skills, sheyue_skill)
sheyue_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)

	local spade_card
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Spade and not card:isKindOf("Slash")
			and (self:getUseValue(card) <= sgs.ai_use_value.Slash or inclusive) then
			spade_card = card
			break
		end
	end

	if spade_card then
		local suit = spade_card:getSuitString()
		local number = spade_card:getNumberString()
		local card_id = spade_card:getEffectiveId()
		local card_str = ("slash:sheyue[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)

		assert(slash)
		return slash
	end
end

function sgs.ai_cardneed.sheyue(to, card)
	return card:getSuit() == sgs.Card_Spade
end

sgs.ai_skill_use["@@shantou"] = function(self, prompt)
	if #self.enemies == 0 then return false end
	self:sort(self.enemies, "handcard")
	for _, p in ipairs(self.enemies) do
		if p:getHp() > self.player:getHp() and p:getHandcardNum() >= self.player:getHandcardNum() then
			return "@ShantouCard=.->" .. p:objectName()
		end
	end
	for _, p in ipairs(self.enemies) do
		if p:getHp() > self.player:getHp() then
			return "@ShantouCard=.->" .. p:objectName()
		end
	end
	return "."
end

local daoshe_skill={}
daoshe_skill.name = "daoshe"
table.insert(sgs.ai_skills,daoshe_skill)
daoshe_skill.getTurnUseCard = function(self)
	if self.player:getHandcardNum() < 2 then return nil end
	if self.player:hasUsed("DaosheCard") then return nil end

	local cards = self.player:getHandcards()
	local peaches = self:getCardsNum("Peach")
	local jinks = self:getCardsNum("Jink")
	local anas = self:getCardsNum("Analeptic")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	local id1 = -1
	local id2 = -1
	for _, card in ipairs(cards) do
		if not (card:isKindOf("Peach") and peaches < 3) and not (card:isKindOf("Jink") and jinks < 2)
				and not (card:isKindOf("Analeptic") and peaches + anas < 2 and self:isVeryWeak())
				and not card:isKindOf("ExNihilo") and not card:isKindOf("Haze") then
			if id1 < 0 then
				id1 = card:getEffectiveId()
				if card:isKindOf("Peach") then
					peaches = peaches - 1
				elseif card:isKindOf("Jink") then
					jinks = jinks - 1
				elseif card:isKindOf("Analeptic") then
					anas = anas - 1
				end
			elseif id2 < 0 and card:getType() == sgs.Sanguosha:getCard(id1):getType() then
				id2 = card:getEffectiveId()
				local card_str = ("@DaosheCard=%d+%d"):format(id1, id2)
				return sgs.Card_Parse(card_str)
			end
		end
	end
	
	return nil
end

sgs.ai_skill_use_func.DaosheCard = function(card, use, self)
	local room = self.room
	if self:isWeak() and #self.friends_noself > 0 then
		for _, p in ipairs(self.friends_noself) do
			if p:getHandcardNum() == p:getHp() + 1 and self:getKnownCard(p, self.player, "Peach", false) > 0 and (not p:isWounded()
					or (not self:isWeak(p) and p:getHp() >= 4) and self:getKnownCard(p, self.player, "Peach", true) < p:getHandcardNum()) then
				use.card = card
				if use.to then use.to:append(p) end
				self:speak("daoshe", self.player:isFemale())
				return
			end
		end
	end
	if #self.enemies > 0 then
		self:sort(self.enemies, "hp")
		for _, p in ipairs(self.enemies) do
			if p:getHandcardNum() == p:getHp() + 1 and getKnownCard(p, self.player, "Peach", true) then
				use.card = card
				if use.to then use.to:append(p) end
				self:speak("daoshe", self.player:isFemale())
				return
			end
		end
		for _, p in ipairs(self.enemies) do
			if not p:isKongcheng() and getKnownCard(p, self.player, "Peach", true) then
				use.card = card
				if use.to then use.to:append(p) end
				self:speak("daoshe", self.player:isFemale())
				return
			end
		end
		self:sort(self.enemies, "handcard")
		for _, p in ipairs(self.enemies) do
			if p:getHandcardNum() == p:getHp() + 1 then
				use.card = card
				if use.to then use.to:append(p) end
				self:speak("daoshe", self.player:isFemale())
				return
			end
		end
		self:sort(self.enemies, "defense")
		for _, p in ipairs(self.enemies) do
			if not p:isKongcheng() then
				use.card = card
				if use.to then use.to:append(p) end
				self:speak("daoshe", self.player:isFemale())
				return
			end
		end
	end
end

sgs.ai_card_intention.DaosheCard = 80
sgs.ai_use_priority.DaosheCard = 8.2
sgs.ai_use_value.DaosheCard = 8.4
sgs.dynamic_value.control_card.DaosheCard = true

sgs.ai_skill_discard.DaosheCard = function(self, discard_num, optional, include_equip)
	local to_discard = {}
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	table.insert(to_discard, cards[1]:getEffectiveId())
	return to_discard
end

sgs.ai_skill_cardask["@daoshe-discard-more"] = function(self)
	if self.player:isKongcheng() then return "." end
	local aya = self.room:findPlayerBySkillName("daoshe")
	if aya and not self:isFriend(aya) and self.player:getHandcardNum() == self.player:getHp() then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		return cards[1]:getEffectiveId()
	end
	return "."
end

sgs.ai_skill_askforag.DaosheCard = function(self, card_ids)
	local cards = {}
	for _, id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(id))
	end
	self:sortByKeepValue(cards, true)
	for _, c in ipairs(cards) do
		if c:isKindOf("Peach") or (c:isKindOf("Analeptic") and self:isWeak()) then
			return c:getEffectiveId()
		end
	end
	for _, c in ipairs(cards) do
		if c:isKindOf("Peach") or (c:isKindOf("Analeptic") and self:isWeak(self.room:getCardOwner(c:getEffectiveId()))) then
			return c:getEffectiveId()
		end
	end
	return cards[1]:getEffectiveId()
end

sgs.ai_skill_invoke.fengmi = function(self, data)
	local x = self.player:getTag("FengmiNum"):toInt()
	if x >= self.player:getMaxCards() then
		return true
	end
	if x == self.player:getMaxCards() - 1 and x > 0 and self:getCardsNum("Peach") + self:getCardsNum("Jink") == 0 then
		return true
	end
	return false
end

sgs.ai_skill_cardask["@bingpu"] = function(self)
	if self.player:getHandcardNum() <= 1 then return "." end
	if self.player:getHp() <= 1 and self.player:getHandcardNum() <= 2 then return "." end
	local current = self.room:getCurrent()
	if self.player:getHandcardNum() == self:getCardsNum("Peach") + self:getCardsNum("Jink") + self:getCardsNum("Analeptic")
			+ self:getCardsNum("Nullification") + self:getCardsNum("ExNihilo") + self:getCardsNum("Snatch")
			+ self:getCardsNum("Dismantlement") and self:isEnemy(current)
			then return "." end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	if self:isEnemy(current) then
		if current:isNude() and not current:isAllNude() then
			return "."
		end
		for _, c in sgs.qlist(current:getJudgingArea()) do
			if c:isKindOf("SupplyShortage") then
				return "."
			end
		end
		for _, c in ipairs(cards) do
			if self:getUseValue(c) < 5 and not c:isKindOf("SavageAssault") and not c:isKindOf("ArcheryAttack") then
				return "$" .. c:getEffectiveId()
			end
		end
		return "."
	elseif self:isFriend(current) then
		if not current:getJudgingArea():isEmpty() then
			for _, c2 in ipairs(cards) do
				if self:getUseValue(c2) < 8 and not c2:isKindOf("Peach") then
					return "$" .. c2:getEffectiveId()
				end
			end
		else
			for _, c2 in sgs.qlist(current:getCards("e")) do
				if c2:isKindOf("SilverLion") and current:isWounded() then
					return "$" .. c2:getEffectiveId()
				end
			end
		end
		return "."
	end
	return "."
end

sgs.ai_skill_choice.bingpu = function(self, choices)
	local cirno = self.room:findPlayerBySkillName("bingpu")
	if self:isEnemy(cirno) then
		if self.player:isAllNude() then
			return "ObtainWithSkip"
		end
		if self.player:isNude() and not self.player:isAllNude() then
			return "LetMeSnatch"
		end
		if self:isWeak() and self.player:isKongcheng() and not self.player:isNude() then
			return "LetMeSnatch"
		end
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		for _, c in ipairs(cards) do
			if self:getUseValue(c) > 5 and not c:isKindOf("Jink") then
				return "ObtainWithSkip"
			elseif c:isKindOf("Jink") and self:getCardsNum("Jink") == 1 then
				return "ObtainWithSkip"
			elseif c:isKindOf("SavageAssault") or c:isKindOf("ArcheryAttack") then
				return "ObtainWithSkip"
			end
		end
		if self:getCardsNum("Armor", "e") > 0 then
			return "ObtainWithSkip"
		end
		if self:getCardsNum("Weapon", "e") > 0 then
			return "ObtainWithSkip"
		end
		return "LetMeSnatch"
	elseif self:isFriend(cirno) then
		return "LetMeSnatch"
	end
	return "ObtainWithSkip"
end

sgs.ai_skill_cardchosen.bingpu = function(self, who)
	local card
	if self:isEnemy(who) then
		local equips = who:getCards("e")
		if who:isNude() and not who:isAllNude() then
			card = sgs.Sanguosha:getCard(self:getCardRandomly(who, "j"))
		elseif not who:isKongcheng() then
			card = sgs.Sanguosha:getCard(self:getCardRandomly(who, "h"))
		end
		if not equips:isEmpty() then
			for _, e in sgs.qlist(equips) do
				if e:isKindOf("OffensiveHorse") then card = e end
			end
			for _, e in sgs.qlist(equips) do
				if e:isKindOf("Weapon") then card = e end
			end
			for _, e in sgs.qlist(equips) do
				if e:isKindOf("DefensiveHorse") then card = e end
			end
			for _, e in sgs.qlist(equips) do
				if e:isKindOf("Armor") and not (e:isKindOf("SilverLion") and who:isWounded() and card) then card = e end
			end
		end
		sgs.updateIntention(self.player, who, 80)
	elseif self:isFriend(who) then
		local judges = who:getJudgingArea()
		for _, j in sgs.qlist(judges) do
			if j:isKindOf("Lightning") then card = j end
		end
		for _, j in sgs.qlist(judges) do
			if j:isKindOf("SupplyShortage") then card = j end
		end
		for _, j in sgs.qlist(judges) do
			if j:isKindOf("Indulgence") then card = j end
		end
		for _, c in sgs.qlist(who:getCards("e")) do
			if c:isKindOf("SilverLion") and who:isWounded() then
				card = c
				break
			end
		end
		sgs.updateIntention(self.player, who, -80)
	end
	return card
end

sgs.ai_skill_invoke.shiling = function(self, data)
	if self.player:isKongcheng() then return false end
	local use = data:toCardUse()
	local card = use.card
	if self:getCardsNum("Jink") + self:getCardsNum("Peach") - self.player:getLostHp() >= 2 and not self.player:isWounded() then
		return false
	end
	if self.player:getHp() == 1 and self:getCardsNum("Peach") == self.player:getHandcardNum() then
		return false
	end
	return true
end

sgs.ai_skill_cardask["@shiling-discard"] = function(self, data)
	local use = data:toCardUse()
	local card = use.card
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if #cards == 1 then
		local c = cards[1]
		if self.player:getHp() == 1 and c:objectName():match("peach|analeptic") and not (card:objectName():match("snatch|haze")
			and not self:hasKeyEquip() and not self:isFriend(use.from)) then
			return "."
		end
		if self.player:getHp() == 1 then
			if card:isKindOf("Slash") and not c:isKindOf("Jink") then
				return c:getEffectiveId()
			elseif card:isKindOf("Duel") and not c:isKindOf("Slash") and not c:isKindOf("Nullification") then
				return c:getEffectiveId()
			elseif card:isKindOf("SavageAssault") and not c:isKindOf("Slash") then
				return c:getEffectiveId()
			elseif card:isKindOf("ArcheryAttack") and not c:isKindOf("Jink") then
				return c:getEffectiveId()
			end
			return "."
		end
	end

	if card:isKindOf("ArcheryAttack") and self:getCardsNum("Jink") + self:getCardsNum("Nullification") == 0 then
		return cards[1]:getEffectiveId()
	elseif card:isKindOf("SavageAssault") and self:getCardsNum("Slash") + self:getCardsNum("Nullification") == 0 then
		return cards[1]:getEffectiveId()
	end

	if self:isFriend(use.from) then return "." end

	if card:isKindOf("Slash") and not use.from:hasWeapon("Axe") and not use.from:hasWeapon("Blade") and self:getCardsNum("Jink") > 0 then
		return "."
	elseif card:isKindOf("Duel") and self:getCardsNum("Slash") > use.from:getHandcardNum() / 2 then
		return "."
	elseif card:isKindOf("MindReading") then
		return "."
	elseif card:objectName():match("dismantlement|snatch|haze") and ((self:hasKeyEquip() and self:getCardsNum("Peach") > 0)
		or (not self:hasKeyEquip() and self:getCardsNum("Peach") == 0 and self:getCardsNum("Jink") ~= 1)) then
		return "."
	elseif card:isKindOf("IcyFog") then
		if not self:hasKeyEquip() then return "." end
		for _, c in sgs.qlist(self.player:getHandcards()) do
			if c:isKindOf("EquipCard") then
				return "."
			end
		end
	end
	return cards[1]:getEffectiveId()
end

sgs.ai_skill_invoke.wosui = function(self, data)
	local current = self.room:getCurrent()
	if self:isFriend(current) then return false end
	sgs.updateIntention(self.player, current, 80)
	return true
end

heiyan_skill = {}
heiyan_skill.name = "heiyan"
table.insert(sgs.ai_skills, heiyan_skill)
heiyan_skill.getTurnUseCard = function(self)
	if self.player:getMark("@darkflame") <= 0 then return end
	if #self:getEnemies() <= 0 then return end
	
	local blacknum = 0
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isBlack() and not (c:isKindOf("Analeptic") and self.player:getHp() <= 1) then
			blacknum = blacknum + 1
		end
	end
	if blacknum <= 0 then return end
	
	local n = 0
	for _, p in ipairs(self.enemies) do
		if self:isWeak(p) then
			if p:getHandcardNum() <= 2 then
				if n < 3 then
					n = n + 1
				end
			end
		else
			local has_vine = false
			for _, c in sgs.qlist(p:getCards("e")) do
				if c:isKindOf("Vine") then
					has_vine = true
					break
				end
			end
			if has_vine and p:getHandcardNum() <= 3 and p:getHp() <= 3 then
				if n < 3 then
					n = n + 1
				end
			end
		end
	end
	if n <= 0 then return end
	
	if blacknum == 1 and #self.enemies > 1 and self:isWeak() then return end
	return sgs.Card_Parse("@HeiyanCard=.")
end

sgs.ai_skill_use_func.HeiyanCard = function(card, use, self)
	local abandon_card = {}
	local subnum = 0
	
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if c:isBlack() and not (c:isKindOf("Analeptic") and self.player:getHp() <= 1) then
			table.insert(abandon_card, c:getEffectiveId())
			subnum = subnum + 1
			if subnum == math.min(#self:getEnemies(), 3) then break end
		end
	end
	
	use.card = sgs.Card_Parse("@HeiyanCard=" .. table.concat(abandon_card, "+"))
	
	self:sort(self:getEnemies(), "hp")
	for _, p in ipairs(self:getEnemies()) do
		if self:isWeak(p) then
			if p:getHandcardNum() <= 2 then
				if use.to then use.to:append(p) end
			end
		else
			local has_vine = false
			for _, c in sgs.qlist(p:getCards("e")) do
				if c:isKindOf("Vine") then
					has_vine = true
					break
				end
			end
			if has_vine and p:getHandcardNum() <= 3 and p:getHp() <= 3 then
				if use.to then use.to:append(p) end
			end
		end
		if use.to and use.to:length() >= subnum then break end
	end
	if use.to and use.to:length() < subnum then
		local suspects = {}
		for _, p in ipairs(self:getEnemies()) do
			if not use.to:contains(p) then
				table.insert(suspects, p)
			end
		end
		if #suspects > 0 then
			self:sort(suspects, "hp")
			for _, p in ipairs(suspects) do
				if use.to then use.to:append(p) end
				if use.to and use.to:length() >= subnum then break end
			end
		end
	end
	
	return
end

sgs.ai_use_priority.HeiyanCard = 9
sgs.ai_use_value.HeiyanCard = 8.1
sgs.ai_card_intention.HeiyanCard = 80
sgs.dynamic_value.damage_card.HeiyanCard = true

sgs.ai_skill_cardask["@huiyin-discard"] = function(self, data)
	local move = data:toMoveOneTime()
	if not self:isFriend(move.from) then return "." end
	local suit_list = {}
	for _, id in sgs.qlist(move.card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if not contains(suit_list, c:getSuitString()) then
			table.insert(suit_list, c:getSuitString())
		end
	end
	if self:hasLion() and self.player:isWounded() then
		return self.player:getArmor():getEffectiveId()
	end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards, true)
	for _, c in ipairs(cards) do
		if not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 2)
				and not (c:isKindOf("Analeptic") and self:getCardsNum("Analeptic") + self:getCardsNum("Peach") < 3 and self:isVeryWeak())
				and not (c:isKindOf("ExNihilo")) and contains(suit_list, c:getSuitString()) then
			return c:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_skill_invoke.linshang = function(self, data)
	local draw = data:toDrawNCards()
	local n = draw.n
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if not (self.player:canSlash(p) and self:slashIsAvailable()) then continue end
		if p:getHp() <= 2 and not self:hasUnknownCard(p) and getKnownCard(p, self.player, "Jink", true) == 0
			and not p:hasArmorEffect("EightDiagram") and not self:hasJiahuEffect(p) then
			return true
		end
		if p:getHp() <= 2 and self.player:getWeapon() and self.player:getWeapon():isKindOf("Axe")
			and self:getCardsNum("Slash") >= 1 and self.player:getCards("he"):length() >= 4 then
			return true
		end
		if p:getHp() == 1 and p:getHandcardNum() + p:getPile("wooden_ox"):length() <= 1
			and not p:hasArmorEffect("EightDiagram") and not self:hasJiahuEffect(p) then
			return true
		end
		if not self:cantbeHurt(p, self.player, 2) and p:hasArmorEffect("Vine") and self:canAttack(p, self.player, sgs.DamageStruct_Fire) then
			return true
		end
		if not self:cantbeHurt(p) and self:canAttack(p, self.player, sgs.DamageStruct_Fire) and self.player:hasWeapon("Fan") then
			return true
		end
	end

	if self.player:getHp() == 1 or (self:isVeryWeak() and self:getDefenseCardsNum() <= 1) then
		return true
	end

	if n >= 2 then
		self:sort(self.enemies, "equip", true)
		for _, p in ipairs(self.enemies) do
			if self:hasKeyEquip(p) then
				return true
			end
		end

		self:sort(self.friends_noself, "hp")
		for _, p in ipairs(self.friends_noself) do
			if p:getArmor() and p:getArmor():isKindOf("SilverLion") and (not p:hasSkills("jinlun+chiwa") or p:getHp() == 1)
				and p:isWounded() then
				return true
			end
		end
	end

	for _, p in ipairs(self.enemies) do
		if not (self.player:canSlash(p) and self:slashIsAvailable()) then continue end
		if self:isVeryWeak(p) and not self:cantbeHurt(p) and (self:canAttack(p, self.player)
			or self:canAttack(p, self.player, sgs.DamageStruct_Fire) or self:canAttack(p, self.player, sgs.DamageStruct_Thunder)) then
			return true
		end
	end

	if n >= 2 then
		self:sort(self.enemies, "handcard")
		for _, p in ipairs(self.enemies) do
			if p:hasSkills(sgs.cardneed_skill) and p:getHandcardNum() > 0 then
				return true
			end
		end
	end

	return false
end

sgs.ai_skill_choice.linshang = function(self, choices, data)
	local draw = data:toDrawNCards()
	local n = draw.n
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if not (self.player:canSlash(p) and self:slashIsAvailable()) then continue end
		if p:getHp() <= 2 and not self:hasUnknownCard(p) and getKnownCard(p, self.player, "Jink", true) == 0
			and not p:hasArmorEffect("EightDiagram") and not self:hasJiahuEffect(p) then
			return tostring(n)
		end
		if p:getHp() <= 2 and self.player:getWeapon() and self.player:getWeapon():isKindOf("Axe")
			and self:getCardsNum("Slash") >= 1 and self.player:getCards("he"):length() >= 4 then
			return tostring(n)
		end
		if p:getHp() == 1 and p:getHandcardNum() + p:getPile("wooden_ox"):length() <= 1
			and not p:hasArmorEffect("EightDiagram") and not self:hasJiahuEffect(p) then
			return tostring(n)
		end
		if not self:cantbeHurt(p, self.player, 2) and p:hasArmorEffect("Vine") and self:canAttack(p, self.player, sgs.DamageStruct_Fire) then
			return tostring(n)
		end
		if not self:cantbeHurt(p) and self:canAttack(p, self.player, sgs.DamageStruct_Fire) and self.player:hasWeapon("Fan") then
			return tostring(n)
		end
	end

	if self.player:getHp() == 1 or (self:isVeryWeak() and self:getDefenseCardsNum() <= 1) then
		return tostring(n)
	end

	if n >= 2 then
		self:sort(self.enemies, "equip", true)
		for _, p in ipairs(self.enemies) do
			if self:hasKeyEquip(p) then
				return tostring(n - 1)
			end
		end

		self:sort(self.friends_noself, "hp")
		for _, p in ipairs(self.friends_noself) do
			if p:getArmor() and p:getArmor():isKindOf("SilverLion") and (not p:hasSkills("jinlun+chiwa") or p:getHp() == 1)
				and p:isWounded() then
				return tostring(n - 1)
			end
		end
	end

	for _, p in ipairs(self.enemies) do
		if not (self.player:canSlash(p) and self:slashIsAvailable()) then continue end
		if self:isVeryWeak(p) and not self:cantbeHurt(p) and (self:canAttack(p, self.player)
			or self:canAttack(p, self.player, sgs.DamageStruct_Fire) or self:canAttack(p, self.player, sgs.DamageStruct_Thunder)) then
			return tostring(n)
		end
	end

	if n >= 2 then
		return tostring(n - 1)
	end
	return tostring(n)
end

sgs.ai_skill_playerchosen.linshang = function(self, targets)
	self:sort(self.enemies, "equip", true)
	for _, p in ipairs(self.enemies) do
		if self:hasKeyEquip(p) then
			return p
		end
	end

	self:sort(self.friends_noself, "hp")
	for _, p in ipairs(self.friends_noself) do
		if p:getArmor() and p:getArmor():isKindOf("SilverLion") and (not p:hasSkills("jinlun+chiwa") or p:getHp() == 1)
			and p:isWounded() then
			return p
		end
	end

	self:sort(self.enemies, "handcard")
	for _, p in ipairs(self.enemies) do
		if p:hasSkills(sgs.cardneed_skill) and p:getHandcardNum() > 0 then
			return p
		end
	end

	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if not p:isNude() then
			return p
		end
	end
end

sgs.ai_skill_choice.linshangbasic = function(self, choices, data)
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if not (self.player:canSlash(p) and self:slashIsAvailable()) then continue end
		if p:getHp() <= 2 and not self:hasUnknownCard(p) and getKnownCard(p, self.player, "Jink", true) == 0
			and not p:hasArmorEffect("EightDiagram") and not self:hasJiahuEffect(p) then
			if self:canAttack(p, self.player, sgs.DamageStruct_Fire) and p:hasArmorEffect("Vine") and table.contains(choices, "fire_slash") then
				return "fire_slash"
			end
			if self:isGoodChainTarget(p, self.player, sgs.DamageStruct_Fire) and table.contains(choices, "fire_slash") then
				return "fire_slash"
			elseif self:isGoodChainTarget(p, self.player, sgs.DamageStruct_Thunder) and table.contains(choices, "thunder_slash") then
				return "thunder_slash"
			end
			return "slash"
		end
		if p:getHp() <= 2 and self.player:getWeapon() and self.player:getWeapon():isKindOf("Axe")
			and self:getCardsNum("Slash") >= 1 and self.player:getCards("he"):length() >= 4 then
			return "analeptic"
		end
		if p:getHp() == 1 and p:getHandcardNum() + p:getPile("wooden_ox"):length() <= 1
			and not p:hasArmorEffect("EightDiagram") and not self:hasJiahuEffect(p) then
			if self:canAttack(p, self.player, sgs.DamageStruct_Fire) and p:hasArmorEffect("Vine") and table.contains(choices, "fire_slash") then
				return "fire_slash"
			end
			if self:isGoodChainTarget(p, self.player, sgs.DamageStruct_Fire) and table.contains(choices, "fire_slash") then
				return "fire_slash"
			elseif self:isGoodChainTarget(p, self.player, sgs.DamageStruct_Thunder) and table.contains(choices, "thunder_slash") then
				return "thunder_slash"
			end
			return "slash"
		end
		if not self:cantbeHurt(p, self.player, 2) and p:hasArmorEffect("Vine") and self:canAttack(p, self.player, sgs.DamageStruct_Fire) then
			if table.contains(choices, "fire_slash") then
				return "fire_slash"
			elseif table.contains(choices, "thunder_slash") then
				return "thunder_slash"
			end
		end
		if not self:cantbeHurt(p) and self:canAttack(p, self.player, sgs.DamageStruct_Fire) and self.player:hasWeapon("Fan") then
			if table.contains(choices, "fire_slash") then
				return "fire_slash"
			end
		end
	end

	if self.player:getHp() == 1 or (self:isVeryWeak() and self:getDefenseCardsNum() <= 1) then
		return "peach"
	end

	for _, p in ipairs(self.enemies) do
		if not (self.player:canSlash(p) and self:slashIsAvailable()) then continue end
		if self:isVeryWeak(p) and not self:cantbeHurt(p) then
			if self:canAttack(p, self.player, sgs.DamageStruct_Fire) and table.contains(choices, "fire_slash") then
				return "fire_slash"
			elseif self:canAttack(p, self.player, sgs.DamageStruct_Thunder) and table.contains(choices, "thunder_slash") then
				return "thunder_slash"
			elseif self:canAttack(p, self.player) and table.contains(choices, "slash") then
				return "slash"
			end
		end
	end

	return choices[1]
end

sgs.ai_skill_use["@@linshang"] = function(self, prompt)
	local cardname = prompt:split(":")[2]
	local card = sgs.Sanguosha:cloneCard(cardname, sgs.Card_NoSuit, 0)
	card:setSkillName("linshang")
	card:deleteLater()
	local dummy_use = {isDummy = true, to = sgs.SPlayerList()}
	self:useBasicCard(card, dummy_use)
	if not dummy_use.card then return "." end
	if dummy_use.to:isEmpty() then
		if dummy_use.card:isKindOf("Peach") or dummy_use.card:isKindOf("Analeptic") then return dummy_use.card:toString() end
	else
		self:sort(self.enemies, "defense")
		for _, p in ipairs(self.enemies) do
			if not (self.player:canSlash(p) and self:slashIsAvailable()) then continue end
			if p:getHp() <= 2 and not self:hasUnknownCard(p) and getKnownCard(p, self.player, "Jink", true) == 0
				and not p:hasArmorEffect("EightDiagram") and not self:hasJiahuEffect(p) then
				return dummy_use.card:toString() .. "->" .. p:objectName()
			end
			if p:getHp() == 1 and p:getHandcardNum() + p:getPile("wooden_ox"):length() <= 1
				and not p:hasArmorEffect("EightDiagram") and not self:hasJiahuEffect(p) then
				return dummy_use.card:toString() .. "->" .. p:objectName()
			end
			if not self:cantbeHurt(p, self.player, 2) and p:hasArmorEffect("Vine") and self:canAttack(p, self.player, sgs.DamageStruct_Fire) then
				return dummy_use.card:toString() .. "->" .. p:objectName()
			end
			if not self:cantbeHurt(p) and self:canAttack(p, self.player, sgs.DamageStruct_Fire) and self.player:hasWeapon("Fan") then
				return dummy_use.card:toString() .. "->" .. p:objectName()
			end
		end
		for _, p in ipairs(self.enemies) do
			if not (self.player:canSlash(p) and self:slashIsAvailable()) then continue end
			if self:isVeryWeak(p) and not self:cantbeHurt(p) and (self:canAttack(p, self.player)
				or self:canAttack(p, self.player, sgs.DamageStruct_Fire) or self:canAttack(p, self.player, sgs.DamageStruct_Thunder)) then
				return dummy_use.card:toString() .. "->" .. p:objectName()
			end
		end
	end
	
	return "."
end

sgs.ai_skill_invoke.jiexun = true

sgs.ai_skill_choice.shengyao = function(self, choices)
	
	function shuffle(t)
	    math.randomseed(tonumber(tostring(os.time()):reverse():sub(1, 7)))
	    for i = 1, #t - 1 do
	    	for j = i + 1, #t do
	    		if math.random(1, 10) < 5 then
	    			t[i], t[j] = t[j], t[i]
	    		end
	    	end
	    end
	end

	local choice_table = choices:split("+")
	shuffle(choice_table)
	if self:isWeak() and self.player:getMark("@moon") <= 1 and table.contains(choice_table, "@moon") then return "@moon" end
	self:sort(self.friends, "hp")
	for _, p in ipairs(self.friends) do
		if p:getLostHp() >= 2 and (p:getHandcardNum() > p:getHp() or p:getHp() == 1) and table.contains(choice_table, "@wood")
			and self.player:getMark("@wood") == 0 then
			return "@wood"
		end
	end
	for _, p in ipairs(self.friends) do
		if self:isVeryWeak(p) and table.contains(choice_table, "@earth") and self.player:getMark("@earth") == 0 then
			return "@earth"
		end
	end
	if table.contains(choice_table, "@sun") then return "@sun" end
	for _, mark in ipairs(choice_table) do
		if self.player:getMark(mark) == 0 then
			return mark
		end
	end
	math.randomseed(tostring(os.time()):reverse():sub(1, 7))
	return choice_table[math.random(1, #choice_table)]
end

sgs.ai_skill_use["@@ranhui"] = function(self, prompt)
	local target = self.room:getCurrent()
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	local discards = self:askForDiscard("ranhui", 3, 3, true, true)
	if #discards < 3 then return "." end

	local damage = sgs.DamageStruct()
	damage.damage = 1
	damage.nature = sgs.DamageStruct_Fire
	damage.from = self.player
	damage.to = target
	damage.reason = "ranhui"
	damage = self:touhouDamage(damage, self.player, target)

	local epeaches = 0
	for _, p in ipairs(self.enemies) do
		epeaches = epeaches + getCardsNum("Peach", p, self.player)
	end
	epeaches = epeaches + getCardsNum("Analeptic", target, self.player)
	if damage.damage >= target:getHp() + epeaches and self:isEnemy(target) then
		return "@RanhuiCard=" .. table.concat(discards, "+") .. "->" .. target:objectName()
	end

	local peaches = 0
	local all_peach = self:getCardsNum("Peach")
	if self:isVeryWeak() then
		all_peach = all_peach + self:getCardsNum("Analeptic")
	end
	for _, id in ipairs(discards) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isKindOf("Peach") or (self:isVeryWeak() and c:isKindOf("Analeptic")) then
			peaches = peaches + 1
		end
	end
	if peaches == all_peach or peaches >= 2 then return "." end

	if self:isGoodChainTarget(target) then
		return "@RanhuiCard=" .. table.concat(discards, "+") .. "->" .. target:objectName()
	end

	if not self:isEnemy(target) then return nil end

	if damage.damage >= 2 then
		return "@RanhuiCard=" .. table.concat(discards, "+") .. "->" .. target:objectName()
	end

	if self:getOverflow() >= 2 and damage.damage > 0 and not self:cantbeHurt(target)
		and (self.player:getMark("@fire") > 1 or self.player:getMark("@philosopher") == 0) then
		return "@RanhuiCard=" .. table.concat(discards, "+") .. "->" .. target:objectName()
	end
	
	return "."
end

sgs.ai_skill_invoke.huzang = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) and damage.to:isWounded() and damage.to:getArmor() and damage.to:getArmor():isKindOf("SilverLion") then
		return true
	end
	if self:isEnemy(damage.to) and damage.to:isWounded() and damage.to:getArmor() and damage.to:getArmor():isKindOf("SilverLion")
		and damage.to:getCards("he"):length() == 1 then
		return false
	end
	if self:hasKeyEquip(damage.to) then
		return true
	end
	if self.player:getMark("@water") == 1 and self.player:getMark("@philosopher") > 0 then return false end
	return true
end

sgs.ai_skill_invoke.jiaodi = function(self, data)
	local target = self.player:getTag("JiaodiTarget"):toPlayer()
	if not self:isFriend(target) then return false end
	if target:getHp() == 1 or self:isVeryWeak(target) then return true end
	if target:isLord() and sgs.isLordInDanger() then return true end
	if self.player:getMark("@wood") > 1 or self.player:getMark("@philosopher") == 0 then return true end
	return false
end

local dianjin_skill = {name = "dianjin"}
table.insert(sgs.ai_skills, dianjin_skill)
dianjin_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return nil end

	local card
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)

	if #cards == 1 or self:isWeak() then
		card = cards[1]
		if self:getUseValue(card) <= 6 and self:getKeepValue(card) <= 6 then
			return sgs.Card_Parse(("@DianjinCard=%d"):format(card:getEffectiveId()))
		end
	end

	if self.player:getMark("@gold") == 1 and self.player:getMark("@philosopher") > 0 then return nil end

	for _, acard in ipairs(cards) do
		local shouldUse = true
		if self:getUseValue(acard) > sgs.ai_use_value.IronChain and acard:getTypeId() == sgs.Card_TypeTrick then
			local dummy_use = { isDummy = true }
			self:useTrickCard(acard, dummy_use)
			if dummy_use.card then shouldUse = false end
		end
		if acard:getTypeId() == sgs.Card_TypeEquip then
			local dummy_use = { isDummy = true }
			self:useEquipCard(acard, dummy_use)
			if dummy_use.card then shouldUse = false end
		end
		if shouldUse then
			card = acard
			break
		end
	end
	if not card then return nil end
	return sgs.Card_Parse(("@DianjinCard=%d"):format(card:getEffectiveId()))
end

sgs.ai_skill_use_func.DianjinCard = function(card, use, self)
	use.card = card
	return
end

sgs.ai_use_value.DianjinCard = 6.1
sgs.ai_use_priority.DianjinCard = 8.7
sgs.dynamic_value.benefit.DianjinCard = true

sgs.ai_skill_cardask["@zhenlei-discard"] = function(self, data)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	local lord = self.room:getLord()
	if self:isFriend(lord) and sgs.isLordInDanger() then
		for _, c in ipairs(cards) do
			if not (c:isKindOf("Peach") and self:getAllPeachNum() <= 2)
				and not (lord:objectName() == self.player:objectName() and (c:isKindOf("Jink") or c:isKindOf("Analeptic"))) then
				self.room:writeToConsole("Zhenlei to " .. lord:getGeneralName())
				return c:getEffectiveId()
			end
		end
	end
	self:sort(self.friends, "hp")
	for _, p in ipairs(self.friends) do
		if self:isVeryWeak(p) and (p:getHp() <= 1 or p:isKongcheng()) then
			for _, c in ipairs(cards) do
				if not (c:isKindOf("Peach") and self:getAllPeachNum() <= 2)
					and not (p:objectName() == self.player:objectName() and (c:isKindOf("Jink") or c:isKindOf("Analeptic"))) then
					self.room:writeToConsole("Zhenlei to " .. p:getGeneralName())
					return c:getEffectiveId()
				end
			end
		end
	end
	if self.player:getMark("@earth") == 1 and self.player:getMark("@philosopher") > 0 then return "." end
	self.room:writeToConsole("Zhenlei mark > 1 or pachouli has xianshied.")
	if #self.friends == 0 then return "." end
	for _, c in ipairs(cards) do
		if not c:isKindOf("Peach") and not (self:isWeak() and (c:isKindOf("Jink") or c:isKindOf("Analeptic")))
			and not (c:isKindOf("Jink") and self:getCardsNum("Jink") == 1) then
			return c:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_skill_playerchosen.zhenlei = function(self, data)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	local lord = self.room:getLord()
	if self:isFriend(lord) and sgs.isLordInDanger() then
		for _, c in ipairs(cards) do
			if not (c:isKindOf("Peach") and self:getAllPeachNum() <= 2)
				and not (lord:objectName() == self.player:objectName() and (c:isKindOf("Jink") or c:isKindOf("Analeptic"))) then
				return lord
			end
		end
	end
	self:sort(self.friends, "hp")
	for _, p in ipairs(self.friends) do
		if self:isVeryWeak(p) and (p:getHp() <= 1 or p:isKongcheng()) then
			for _, c in ipairs(cards) do
				if not (c:isKindOf("Peach") and self:getAllPeachNum() <= 2)
					and not (p:objectName() == self.player:objectName() and (c:isKindOf("Jink") or c:isKindOf("Analeptic"))) then
					return p
				end
			end
		end
	end
	return self.friends[1]
end

sgs.ai_skill_choice.zhenlei = function(self, choices, data)
	local damage = data:toDamage()
	local player = damage.to
	if not self:isEnemy(player) then return "ZLPrevent" end
	if self:cantbeHurt(player, self.player, damage.damage) then return "ZLPrevent" end
	if damage.damage >= player:getHp() + self:getAllPeachNum(player) and self:endsGameByDeath(player) then return "ZLTurn" end
	if self:isWeakerThan(player, self.player) and not self:isWeak() then return "ZLTurn" end
	return "ZLPrevent"
end

sgs.ai_skill_use["@@huangyan"] = function(self, prompt)
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()
	local min_card = self:getMinCard()
	local min_point = min_card:getNumber()

	self:sort(self.enemies, "hp")
	for _, p in ipairs(self.enemies) do
		if p:isKongcheng() then continue end
		local damage = sgs.DamageStruct("huangyan", self.player, p, 1, sgs.DamageStruct_Fire)
		damage = self:touhouDamage(damage, self.player, p)
		if p:getHp() + self:getAllPeachNum(p) <= damage.damage and (max_point >= 11 or (max_point >= 7 and p:getHandcardNum() <= 2)) then
			return ("@HuangyanCard=%d->%s"):format(max_card:getEffectiveId(), p:objectName())
		end
	end

	self:sort(self.enemies, "handcard")
	for _, p in ipairs(self.enemies) do
		if p:isKongcheng() then continue end
		local damage = sgs.DamageStruct("huangyan", self.player, p, 1, sgs.DamageStruct_Fire)
		damage = self:touhouDamage(damage, self.player, p)
		if damage.damage < 1 then continue end
		if p:getHandcardNum() == 1 or (self:isWeak(p) and p:getHandcardNum() <= 2) or (self.player:getHp() >= 2 and max_point >= 7
			and p:getHandcardNum() <= 2) or (self:isWeakerThan(p, self.player) and max_point - p:getHandcardNum() >= 6) then
			return ("@HuangyanCard=%d->%s"):format(max_card:getEffectiveId(), p:objectName())
		end
	end

	self:sort(self.friends_noself, "handcard", true)
	for _, p in ipairs(self.friends_noself) do
		if p:isKongcheng() then continue end
		if self:getOverflow(p) >= 0 and not self:isVeryWeak(p) and not (p:isLord() and sgs.isLordInDanger())
			and min_point <= 5 and not min_card:isKindOf("Peach") and not (min_card:isKindOf("Jink") and self:getCardsNum("Jink") == 1)
			and not (min_card:isKindOf("ExNihilo")) and not (min_card:isKindOf("Haze"))
			and not (min_card:isKindOf("Analeptic") and self:getCardsNum("Analeptic") == 1 and self:isVeryWeak()) then
			return ("@HuangyanCard=%d->%s"):format(max_card:getEffectiveId(), p:objectName())
		end
	end

	return "."
end

sgs.ai_skill_use["@@jingyue"] = function(self, prompt)
	local use = self.player:getTag("JingyueUse"):toCardUse()
	local card = use.card
	local friends = {}
	local enemies = {}
	local targets = {}
	for _, p in sgs.qlist(use.to) do
		if p:hasFlag("JingyueTarget") then
			if self:isFriend(p) then
				table.insert(friends, p)
			elseif self:isEnemy(p) then
				table.insert(enemies, p)
			end
		end
	end

	if card:isKindOf("AmazingGrace") then
		if #enemies > 0 and #enemies >= #friends - 1 then
			for _, p in ipairs(enemies) do
				table.insert(targets, p:objectName())
			end
		end
	elseif card:isKindOf("GodSalvation") then
		if #enemies > 0 then
			for _, p in ipairs(enemies) do
				if p:isWounded() then
					table.insert(targets, p:objectName())
				end
			end
		end
	elseif card:objectName():match("savage_assault|archery_attack") then
		for _, p in ipairs(friends) do
			local damage = sgs.DamageStruct(card, use.from, p)
			damage = self:touhouDamage(damage, use.from, p)
			if not self:needToLoseHp(p) and damage.damage > 0 then
				table.insert(targets, p:objectName())
			elseif damage.damage > 1 then
				table.insert(targets, p:objectName())
			end
		end
	elseif card:objectName():match("fire_slash|thunder_slash|fire_attack") then
		local nature = sgs.DamageStruct_Fire
		if card:isKindOf("ThunderSlash") then
			nature = sgs.DamageStruct_Thunder
		end
		local damage = sgs.DamageStruct(card, use.from, p, 1, nature)
		damage = self:touhouDamage(damage, use.from, p)
		for _, p in ipairs(friends) do
			if (not self:needToLoseHp(p) or self:isGoodChainTarget(p, use.from, nature)) and damage.damage > 0 and self:isWeak(p) then
				table.insert(targets, p:objectName())
			elseif damage.damage > 1 then
				table.insert(targets, p:objectName())
			end
		end
	elseif card:isKindOf("Duel") then
		for _, p in ipairs(friends) do
			local damage = sgs.DamageStruct(card, use.from, p)
			damage = self:touhouDamage(damage, use.from, p)
			if not self:needToLoseHp(p) and damage.damage > 0 and self:isWeak(p) then
				table.insert(targets, p:objectName())
			elseif damage.damage > 1 then
				table.insert(targets, p:objectName())
			end
		end
	elseif card:objectName():match("dismantlement|snatch|haze") then
		for _, p in ipairs(friends) do
			if not self:isEnemy(use.from, p) then break end
			if getKnownCard(p, self.player, "Peach", true) >= p:getHandcardNum() / 3 then
				table.insert(targets, p:objectName())
			elseif self:isWeak(p) then
				table.insert(targets, p:objectName())
			elseif self:hasKeyEquip(p) then
				local keys = self:getKeyEquips(p)
				for _, e in ipairs(keys) do
					if e:isKindOf("WoodenOx") or e:isKindOf("Armor") or e:isKindOf("DefensiveHorse") or e:isKindOf("Crossbow") then
						table.insert(targets, p:objectName())
						break
					end
				end
			elseif getKnownCard(p, self.player, "Jink", true) == 1 and (self:getUnknownNum(p) <= p:getHandcardNum()) / 3
				or p:getHandcardNum() <= 3 then
				table.insert(targets, p:objectName())
			end
		end
		for _, p in ipairs(enemies) do
			if not self:isFriend(use.from, p) then break end
			if p:getCards("j"):length() > 0 then
				table.insert(targets, p:objectName())
			elseif p:hasArmorEffect("SilverLion") and p:isWounded() then
				table.insert(targets, p:objectName())
			end
		end
	end

	if #targets > 0 then
		return "@JingyueCard=.->" .. table.concat(targets, "+")
	end
	return "."
end

sgs.ai_skill_invoke.xianshi = true

sgs.ai_skill_playerchosen.xianshi = function(self, targets)
	local friends = {}
	for _, p in sgs.qlist(targets) do
		if self:isFriend(p) then
			table.insert(friends, p)
		end
	end
	if #friends == 0 then return nil end
	for _, p in ipairs(friends) do
		if p:isLord() and sgs.isLordInDanger() then
			return p
		end
	end
	if self:isVeryWeak() then return self.player end
	self:sort(friends, "hp")
	for _, p in ipairs(friends) do
		if p:hasSkills(sgs.maxhp_skill) then
			return p
		end
	end
	return friends[1]
end

sgs.ai_skill_use["@@xianshi"] = function(self, prompt)
	local drawers = self:findPlayersNameToDraw(true, 2, 3)
	if #drawers > 0 then
		return "@XianshiCard=.->" .. table.concat(drawers, "+")
	end
	return "."
end

sgs.ai_skill_use["@@diaoou"] = function(self, prompt)
	if #self.enemies == 0 then return "." end
	self:sort(self.enemies, "hp")
	sgs.updateIntention(self.player, self.enemies[1], 80)
	return ("@DiaoouCard=.->") .. self.enemies[1]:objectName()
end

sgs.ai_skill_cardask["@diaoou-damage-ask"] = function(self, data)
	local damage = data:toDamage()
	local _damage = sgs.DamageStruct()
	_damage.damage = 1
	_damage.from = damage.from
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if p:getMark("@ningyou") > 0 then
			_damage.to = p
			break
		end
	end
	if not _damage.to then return "." end
	_damage.reason = "diaoou"
	local real_damage = self:touhouDamage(_damage, _damage.from, _damage.to)
	if real_damage.damage == 0 then
		return "."
	end
	if self.player:isNude() then return "." end
	if self:isVeryWeak() and real_damage.damage < real_damage.to:getHp() then
		if self.player:getEquips():isEmpty() and self.player:getHandcardNum() == self:getCardsNum("Peach") + self:getCardsNum("Analeptic") then
			return "."
		elseif self.player:getHandcardNum() <= 3 and self:getKeyEquipNum() == self.player:getEquips():length() then
			local cards = sgs.QList2Table(self.player:getCards("he"))
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if not c:isKindOf("Peach") and not c:isKindOf("Analeptic") and not table.contains(self:getKeyEquips(), c) then
					return c:getEffectiveId()
				end
			end
		end
	end
	
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	--[[if self:getKeepValue(cards[1]) <= 4 then
		table.insert(to_discard, cards[1])
	end]]--
	for _, c in ipairs(cards) do
		if (not table.contains(self:getKeyEquips(), c)) or real_damage.damage >= real_damage.to:getHp() then
			return c:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_skill_cardask["@diaoou-lost-ask"] = function(self, data)
	local lost = data:toHpLost()
	if lost.num <= 0 then return "." end
	local loser
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if p:getMark("@ningyou") > 0 then
			loser = p
			break
		end
	end
	if not loser then return "." end
	if self.player:isNude() then return "." end
	if self:isVeryWeak() and lost.num < loser:getHp() then
		if self.player:getEquips():isEmpty() and self.player:getHandcardNum() == self:getCardsNum("Peach") + self:getCardsNum("Analeptic") then
			return "."
		elseif self.player:getHandcardNum() <= 3 and self:getKeyEquipNum() == self.player:getEquips():length() then
			local cards = sgs.QList2Table(self.player:getCards("he"))
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if not c:isKindOf("Peach") and not c:isKindOf("Analeptic") and not table.contains(self:getKeyEquips(), c) then
					return c:getEffectiveId()
				end
			end
		end
	end
	
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	--[[if self:getKeepValue(cards[1]) <= 4 then
		table.insert(to_discard, cards[1])
	end]]--
	for _, c in ipairs(cards) do
		if (not table.contains(self:getKeyEquips(), c)) or lost.num >= loser:getHp() then
			return c:getEffectiveId()
		end
	end
	return "."
end

local anji_skill = {}
anji_skill.name = "anji"
table.insert(sgs.ai_skills, anji_skill)
anji_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("AnjiCard") or self.player:isKongcheng() then return end
	if self:isWeak() and self.player:getHandcardNum() == 1 then return end
	
	if #self.friends_noself == 0 then return end
	
	local cards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByUseValue(cards, true)
	if self:isWeak() and self:getKeepValue(cards[1]) >= 3 then return end
	local blacks = {}
	for _, c in ipairs(cards) do
		if c:isBlack() then
			table.insert(blacks, c)
		end
	end
	if #blacks == 0 then return end
	
	local card_str = ("@AnjiCard=%d"):format(blacks[1]:getEffectiveId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.AnjiCard = function(card, use, self)
	local friends = self.friends_noself
	if #friends == 0 then return end
	use.card = card
	if use.to then use.to:append(friends[1]) end
	return
end

sgs.ai_skill_choice.anji = function(self, choices)
	if #self.enemies == 0 then return "KeepCard" end
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if self.player:inMyAttackRange(p) and not self:hasVine(p) and not self:isWeak() and self.player:getHandcardNum() > 1 then
			return "UseSlash"
		end
	end
	return "KeepCard"
end

sgs.ai_skill_playerchosen.anji = function(self, targets)
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if self.player:inMyAttackRange(p) and not self:hasVine(p) then
			return p
		end
	end
	return
end

sgs.ai_card_intention.AnjiCard = -70
sgs.ai_use_priority.AnjiCard = 7.3
sgs.ai_use_value.AnjiCard = 7.6
sgs.dynamic_value.benefit.AnjiCard = true

sgs.ai_skill_askforag.xuebeng = function(self, card_ids)
	local max_keep_value = -10
	local choice
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if self:getKeepValue(card) > max_keep_value then
			max_keep_value = self:getKeepValue(card)
			choice = id
		end
	end
	return choice
end

sgs.ai_skill_use["@@yaofeng"] = function(self, prompt)
	local snow = self.player:getPile("snow")
	if snow:length() == 0 then return "." end
	local can_use = false
	local cards = {}
	for _, i in sgs.qlist(snow) do
		local c = sgs.Sanguosha:getCard(i)
		table.insert(cards, c)
		if not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 1)
				and not (c:isKindOf("Analeptic") and self:isVeryWeak() and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 2) then
			can_use = true
			break
		end
	end
	if not can_use then return "." end
	
	if #self.enemies == 0 then return "." end
	self:sort(self.enemies, "hp")
	for _, p in ipairs(self.enemies) do
		if not self:hasVine(p) then
			self:sortByKeepValue(cards)
			return ("@YaofengCard=%d->%s"):format(cards[1]:getEffectiveId(), p:objectName())
		end
	end
	return "."
end

sgs.ai_skill_invoke.jihan = function(self, data)
	local snow = self.player:getPile("snow")
	if snow:length() == 1 then
		for _, id in sgs.qlist(snow) do
			local c = sgs.Sanguosha:getCard(id)
			if (c:isKindOf("Peach") and self.player:isWounded()) or c:isKindOf("ExNihilo") or c:isKindOf("Armor") then
				return true
			end
		end
		return false
	elseif snow:length() > 1 then
		return true
	end
end

sgs.ai_skill_use["@@kaihai"] = function(self, prompt)
	local value = -1000
	local player = nil
	for _, p in ipairs(self.enemies) do
		if p:getHp() >= p:getHandcardNum() then continue end
		local m_value = (p:getHandcardNum() - p:getHp()) * 1.37 - math.min(sgs.getDefense(p), 9) * 0.29
		if m_value >= value then
			value = m_value
			player = p
		end
	end
	if player ~= nil then
		sgs.updateIntention(self.player, player, 80)
		return ("@KaihaiCard=.->%s"):format(player:objectName())
	end
	return "."
end

sgs.ai_skill_playerchosen.kaihai = function(self, targets)
	local value = -1000
	local player = nil
	for _, p in ipairs(self.friends) do
		if p:getLostHp() <= p:getHandcardNum() then continue end
		local m_value = (p:getLostHp() - p:getHandcardNum()) * 1.15 - math.min(sgs.getDefense(p), 9) * 0.21
		if m_value >= value then
			value = m_value
			player = p
		end
	end
	if player ~= nil then
		sgs.updateIntention(self.player, player, -80)
		return player
	end
	return nil
end

sgs.ai_skill_invoke.qiji = function(self, data)
	local room = self.room
	local sanae = room:getLord()
	if self:isFriend(sanae) then
		sgs.updateIntention(self.player, sanae, -80)
		return true
	end
	sgs.updateIntention(self.player, sanae, 40)
	return false
end

local yuzhu_skill = {}
yuzhu_skill.name = "yuzhu"
table.insert(sgs.ai_skills, yuzhu_skill)
yuzhu_skill.getTurnUseCard = function(self)
	if self.player:getMark("yuzhu") >= 2 then return end
	
	local card_str = "@YuzhuCard=."
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.YuzhuCard = function(card, use, self)
	use.card = card
	return
end

sgs.ai_use_priority.YuzhuCard = 9.2
sgs.ai_use_value.YuzhuCard = 7.8
sgs.dynamic_value.control_card.YuzhuCard = true

sgs.ai_skill_invoke.yuzhu = function(self, data)
	local total = self.player:getPile("onbashiri"):length()
	local can_draw, can_recover, can_turn, can_damage
	can_draw = (self.player:getPile("onbashiri"):length() >= 1)
	can_recover = (self.player:getPile("onbashiri"):length() >= 2)
	can_turn = (self.player:getPile("onbashiri"):length() >= 3)
	can_damage = (self.player:getPile("onbashiri"):length() >= 4)
	if can_damage then
		local x = 0
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if not self:isFriend(self.player, p) then
				x = x + getKnownCard(p, self.player, "Peach", true)
			end
		end
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if self:isEnemy(p) and not self:cantbeHurt(p, self.player, 2) and p:getHp() + x <= 2 then
				return true
			end
		end
	end
	if can_turn then
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if (self:isEnemy(p) and p:faceUp() and p:hasSkills(sgs.active_skill)) or (self:isFriend(p) and not p:faceUp()
				and p:hasSkills(sgs.active_skill)) then
				return true
			end
		end
	end
	if can_recover then
		for _, p in ipairs(self.friends) do
			if p:isWounded() and self:isWeak(p) and p:getHp() < getBestHp(p) then
				return true
			end
		end
	end
	if can_damage then
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if (self:isEnemy(p) and p:getHp() - 2 < getBestHp(p) and not self:cantbeHurt(p, self.player, 2))
				or (self:isFriend(p) and p:getHp() - 2 >= getBestHp(p)) then
				return true
			end
		end
	end
	if can_turn then
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if (self:isEnemy(p) and p:faceUp()) or (self:isFriend(p) and not p:faceUp()) then
				return true
			end
		end
	end
	if can_recover then
		for _, p in ipairs(self.friends) do
			if p:isWounded() and p:getHp() < getBestHp(p) then
				return true
			end
		end
	end
	if can_draw then
		local p = self:findPlayerToDraw()
		if p ~= nil and self:isVeryWeak(p) and p:getHandcardNum() <= 1 and getKnownCard(p, self.player, "Peach", true) < 1 then
			return true
		end
	end
	return false
end

sgs.ai_skill_askforag.yuzhu = function(self, card_ids)
	local total = self.player:getPile("onbashiri"):length()
	local has_selected = total - #card_ids
	local can_draw, can_recover, can_turn, can_damage
	can_draw = (self.player:getPile("onbashiri"):length() >= 1)
	can_recover = (self.player:getPile("onbashiri"):length() >= 2)
	can_turn = (self.player:getPile("onbashiri"):length() >= 3)
	can_damage = (self.player:getPile("onbashiri"):length() >= 4)
	if can_damage and has_selected < 4 then
		local x = 0
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if not self:isFriend(self.player, p) then
				x = x + getKnownCard(p, self.player, "Peach", true)
			end
		end
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if self:isEnemy(p) and not self:cantbeHurt(p, self.player, 2) and p:getHp() + x <= 2 then
				return card_ids[1]
			end
		end
	end
	if can_turn and has_selected < 3 then
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if (self:isEnemy(p) and p:faceUp() and p:hasSkills(sgs.active_skill)) or (self:isFriend(p) and not p:faceUp()
				and p:hasSkills(sgs.active_skill)) then
				return card_ids[1]
			end
		end
	end
	if can_recover and has_selected < 2 then
		for _, p in ipairs(self.friends) do
			if p:isWounded() and self:isWeak(p) and p:getHp() < getBestHp(p) then
				return card_ids[1]
			end
		end
	end
	if can_damage and has_selected < 4 then
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if (self:isEnemy(p) and p:getHp() - 2 < getBestHp(p) and not self:cantbeHurt(p, self.player, 2))
				or (self:isFriend(p) and p:getHp() - 2 >= getBestHp(p)) then
				return card_ids[1]
			end
		end
	end
	if can_turn and has_selected < 3 then
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if (self:isEnemy(p) and p:faceUp()) or (self:isFriend(p) and not p:faceUp()) then
				return card_ids[1]
			end
		end
	end
	if can_recover and has_selected < 2 then
		for _, p in ipairs(self.friends) do
			if p:isWounded() and p:getHp() < getBestHp(p) then
				return card_ids[1]
			end
		end
	end
	if can_draw and has_selected < 1 then
		local p = self:findPlayerToDraw()
		if p ~= nil and self:isVeryWeak(p) and p:getHandcardNum() <= 1 and getKnownCard(p, self.player, "Peach", true) < 1 then
			return card_ids[1]
		end
	end
	return -1
end

sgs.ai_skill_playerchosen.yuzhu_1 = function(self, targets)
	return self:findPlayerToDraw()
end

sgs.ai_skill_playerchosen.yuzhu_2 = function(self, targets)
	for _, p in ipairs(self.friends) do
		if p:isWounded() and self:isWeak(p) and p:getHp() < getBestHp(p) then
			return p
		end
	end
	for _, p in ipairs(self.friends) do
		if p:isWounded() and p:getHp() < getBestHp(p) then
			return p
		end
	end
	return nil
end

sgs.ai_skill_playerchosen.yuzhu_3 = function(self, targets)
	for _, p in sgs.qlist(targets) do
		if (self:isEnemy(p) and p:faceUp() and p:hasSkills(sgs.active_skill)) or (self:isFriend(p) and not p:faceUp()
			and p:hasSkills(sgs.active_skill)) then
			return p
		end
	end
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if (self:isEnemy(p) and p:faceUp()) or (self:isFriend(p) and not p:faceUp()) then
			return p
		end
	end
	return nil
end

sgs.ai_skill_playerchosen.yuzhu_4 = function(self, targets)
	local x = 0
	for _, p in sgs.qlist(targets) do
		if not self:isFriend(self.player, p) then
			x = x + getKnownCard(p, self.player, "Peach", true)
		end
	end
	for _, p in sgs.qlist(targets) do
		if self:isEnemy(p) and not self:cantbeHurt(p, self.player, 2) and p:getHp() + x <= 2 then
			return p
		end
	end
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if (self:isEnemy(p) and p:getHp() - 2 < getBestHp(p) and not self:cantbeHurt(p, self.player, 2))
			or (self:isFriend(p) and p:getHp() - 2 >= getBestHp(p)) then
			return p
		end
	end
	return nil
end

sgs.ai_skill_invoke.suiwa = function(self, data)
	local movable = false
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		local field = p:getCards("ej")
		for _, c in sgs.qlist(field) do
			if c:isKindOf("DelayedTrick") and self:isFriend(p) and #self.enemies > 0 then
				for _, p2 in ipairs(self.enemies) do
					local delays = {}
					for _, c2 in sgs.qlist(p2:getCards("j")) do
						table.insert(delays, c2:objectName())
					end
					if not table.contains(delays, c:objectName()) then
						return true
					end
				end
			elseif c:isKindOf("SilverLion") and (#self.friends > 1 or (not self:isFriend(p) and #self.friends > 0)) then
				if self.player:getArmor() == nil then return true end
				self:sort(self.friends, "hp")
				for _, p2 in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p2:getArmor() == nil and self:isFriend(p2) and (p:isWounded() or p2:isWounded()) then
						return true
					end
				end
			elseif c:isKindOf("EquipCard") and (#self.friends > 1 or (not self:isFriend(p) and #self.friends > 0)) then
				if c:isKindOf("Armor") and self.player:getArmor() == nil then return true end
				local indices = {weapon = 0, armor = 1, defensive_horse = 2, offensive_horse = 3, treasure = 4}
				local index = indices[c:getSubtype()]
				self:sort(self.friends, "defense")
				for _, p2 in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p2:getEquip(index) == nil and self:isFriend(p2) then
						return true
					end
				end
			end
		end
	end
	return false
end

sgs.ai_skill_use["@@suiwa"] = function(self, prompt)
	local targets = self:findPlayersNameToDraw(true, 1, self.player:getLostHp())
	if #targets ~= 0 then return "@SuiwaCard=.->" .. table.concat(targets, "+") end
	return "."
end

sgs.ai_skill_playerchosen.suiwa_1 = function(self, data)
	local movable = false
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		local field = p:getCards("ej")
		for _, c in sgs.qlist(field) do
			if c:isKindOf("DelayedTrick") and self:isFriend(p) and #self.enemies > 0 then
				for _, p2 in ipairs(self.enemies) do
					local delays = {}
					for _, c2 in sgs.qlist(p2:getCards("j")) do
						table.insert(delays, c2:objectName())
					end
					if not table.contains(delays, c:objectName()) then
						return p
					end
				end
			elseif c:isKindOf("SilverLion") and (#self.friends > 1 or (not self:isFriend(p) and #self.friends > 0)) then
				if self.player:getArmor() == nil then return p end
				self:sort(self.friends, "hp")
				for _, p2 in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p2:getArmor() == nil and self:isFriend(p2) and (p:isWounded() or p2:isWounded()) then
						return p
					end
				end
			elseif c:isKindOf("EquipCard") and (#self.friends > 1 or (not self:isFriend(p) and #self.friends > 0)) then
				if c:isKindOf("Armor") and self.player:getArmor() == nil then return p end
				local indices = {weapon = 0, armor = 1, defensive_horse = 2, offensive_horse = 3, treasure = 4}
				local index = indices[c:getSubtype()]
				self:sort(self.friends, "defense")
				for _, p2 in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p2:getEquip(index) == nil and self:isFriend(p2) then
						return p
					end
				end
			end
		end
	end
	return nil
end

sgs.ai_skill_playerchosen.suiwa_2 = function(self, data)
	local movable = false
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		local field = p:getCards("ej")
		for _, c in sgs.qlist(field) do
			if c:isKindOf("DelayedTrick") and self:isFriend(p) and #self.enemies > 0 then
				for _, p2 in ipairs(self.enemies) do
					local delays = {}
					for _, c2 in sgs.qlist(p2:getCards("j")) do
						table.insert(delays, c2:objectName())
					end
					if not table.contains(delays, c:objectName()) then
						return p2
					end
				end
			elseif c:isKindOf("SilverLion") and (#self.friends > 1 or (not self:isFriend(p) and #self.friends > 0)) then
				if self.player:getArmor() == nil then return self.player end
				self:sort(self.friends, "hp")
				for _, p2 in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p2:getArmor() == nil and self:isFriend(p2) and (p:isWounded() or p2:isWounded()) then
						return p2
					end
				end
			elseif c:isKindOf("EquipCard") and (#self.friends > 1 or (not self:isFriend(p) and #self.friends > 0)) then
				if c:isKindOf("Armor") and self.player:getArmor() == nil then return self.player end
				local indices = {weapon = 0, armor = 1, defensive_horse = 2, offensive_horse = 3, treasure = 4}
				local index = indices[c:getSubtype()]
				self:sort(self.friends, "defense")
				for _, p2 in sgs.qlist(self.room:getOtherPlayers(p)) do
					if p2:getEquip(index) == nil and self:isFriend(p2) then
						return p2
					end
				end
			end
		end
	end
	return nil
end

--[[sgs.ai_skill_playerchosen.chiwa = function(self, targets)
	local use = self.room:getTag("ChiwaUsing"):toCardUse()
	local using = use.card:objectName()
	if using == "peach" then
		self:sort(self.friends_noself, "hp")
		for _, p in ipairs(self.friends) do
			if p:isWounded() and not use.to:contains(p) then
				sgs.updateIntention(self.player, p, -80)
				return p
			end
		end
	elseif using == "ex_nihilo" then
		self:sort(self.friends_noself, "handcard")
		sgs.updateIntention(self.player, self.friends_noself[1], -80)
		return self.friends_noself[1]
	elseif using == "archery_attack" then
		self:sort(self.friends_noself, "defense")
		for _, p in ipairs(self.friends_noself) do
			if not self:hasVine(p) then
				sgs.updateIntention(self.player, p, -80)
				return p
			end
		end
	elseif using == "amazing_grace" then
		self:sort(self.enemies, "chaofeng")
		sgs.updateIntention(self.player, self.enemies[1], 80)
		return self.enemies[1]
	elseif using == "god_salvation" then
		self:sort(self.enemies, "hp")
		for _, p in ipairs(self.enemies) do
			if p:isWounded() then
				sgs.updateIntention(self.player, p, 80)
				return p
			end
		end
	end
end]]--

sgs.ai_skill_use["@@xinyan"] = function(self, prompt)
	local mind_reading = sgs.Sanguosha:cloneCard("mind_reading", sgs.Card_NoSuit, 0)
	mind_reading:setSkillName("xinyan")
	mind_reading:deleteLater()
	local targets = {}
	local final_targets = {}
	local max_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, mind_reading)
	
	self:sort(self.enemies, "chaofeng")
	for _, p in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, p, mind_reading) and mind_reading:targetFilter(sgs.PlayerList(), p, self.player) then
			table.insert(targets, p)
		end
	end
	
	if #targets >= max_num then
		self:sort(targets, "handcard", true)
		for _, p in ipairs(targets) do
			if #final_targets < max_num then
				table.insert(final_targets, p:objectName())
				if #final_targets >= max_num then
					return mind_reading:toString() .. "->" .. table.concat(final_targets, "+")
				end
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
			if #final_targets < max_num then
				table.insert(final_targets, p:objectName())
				if #final_targets >= max_num then
					return mind_reading:toString() .. "->" .. table.concat(final_targets, "+")
				end
			end
		end
		if #final_targets > 0 then return mind_reading:toString() .. "->" .. table.concat(final_targets, "+") end
	end

	return "."
end

function SmartAI:getFangyingCard(card_str, type)
	type = type or "card"

	local dummy_use = {isDummy = true, to = sgs.SPlayerList()}
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	for _, c in ipairs(cards) do
		local card = sgs.Sanguosha:cloneCard(card_str, sgs.Card_NoSuit, 0)
		card:setSkillName("fangying")
		card:addSubcard(c)
		card:deleteLater()
		if self:getUseValue(c) <= self:getUseValue(card) then
			if card:isKindOf("TrickCard") then
				self:useTrickCard(card, dummy_use)
			else
				self:useBasicCard(card, dummy_use)
			end
			if not dummy_use.card then return nil end
			if dummy_use.to:isEmpty() then
				if card:isKindOf("IronChain") then return nil end
				if type == "card" then
					return dummy_use.card
				else
					return dummy_use.card:toString()
				end
			else
				local target_objectname = {}
				if card:isKindOf("IronChain") then
					for _, p in sgs.qlist(dummy_use.to) do
						if (self:isEnemy(p) and not p:isChained()) or (self:isFriend(p) and p:isChained()) then
							table.insert(target_objectname, p:objectName())
						end
						if #target_objectname==2 then break end
					end
				else
					for _, p in sgs.qlist(dummy_use.to) do
						if self:isEnemy(p) then
							table.insert(target_objectname, p:objectName())
							break
						end
					end
				end
				if #target_objectname > 0 then
					if type == "card" then
						return dummy_use.card
					elseif type == "string" then
						return dummy_use.card:toString() .. "->" .. table.concat(target_objectname, "+")
					end
				end
			end
		end
	end

	return nil
end

sgs.ai_skill_choice.fangying = function(self, choices)
	if #choices >= 3 then
		local first_card_str = self.player:getTag("FangyingFirstCard"):toString()
		local last_card_str = self.player:getTag("FangyingLastCard"):toString()
		local first_card = self:getFangyingCard(first_card_str, "card")
		local last_card = self:getFangyingCard(last_card_str, "card")
		local value1 = -100
		local value2 = -100
		if first_card ~= nil then
			first_card:deleteLater()
			value1 = self:getUseValue(first_card)
		end
		if last_card ~= nil then
			last_card:deleteLater()
			value2 = self:getUseValue(last_card)
		end
		if value1 <= 6 - self.player:getHandcardNum() and value2 <= 6 - self.player:getHandcardNum() then return "Cancel" end
		if value1 <= value2 then return "FYLast" end
		return "FYFirst"
	elseif #choices == 2 then
		if table.contains(choices, "FYFirst") then
			local first_card_str = self.player:getTag("FangyingFirstCard"):toString()
			local first_card = self:getFangyingCard(first_card_str, "card")
			if first_card == nil then return "Cancel" end
			if self:getUseValue(first_card) <= 6 - self.player:getHandcardNum() then return "Cancel" end
			return "FYFirst"
		elseif table.contains(choices, "FYLast") then
			local last_card_str = self.player:getTag("FangyingLastCard"):toString()
			local last_card = self:getFangyingCard(last_card_str, "card")
			if last_card == nil then return "Cancel" end
			if self:getUseValue(last_card) <= 6 - self.player:getHandcardNum() then return "Cancel" end
			return "FYLast"
		end
	end
	return "Cancel"
end

sgs.ai_skill_use["@@fangying"] = function(self, prompt)
	if self.player:isKongcheng() then return false end
	
	local card_str = self.player:property("fangying_card"):toString()
	local raw = self:getFangyingCard(card_str, "string")
	if raw ~= nil then return raw end

	return "."
end

sgs.ai_skill_use["@@duannian"] = function(self, prompt)
	local ids = {}
	if self:getCardsNum("Slash") > 1 then
		for _, c in sgs.qlist(self.player:getHandcards()) do
			if c:isKindOf("Slash") then
				table.insert(ids, c:getEffectiveId())
			end
		end
		return "@DuannianCard=" .. table.concat(ids, "+")
	elseif self:getCardsNum("Slash") == 1 then
		if self:getCardsNum("Jink") > 1 then
			local jinked = false
			for _, c in sgs.qlist(self.player:getHandcards()) do
				if c:isKindOf("Slash") then
					table.insert(ids, c:getEffectiveId())
				elseif c:isKindOf("Jink") and not jinked then
					table.insert(ids, c:getEffectiveId())
					jinked = true
				end
			end
			return "@DuannianCard=" .. table.concat(ids, "+")
		elseif self:getCardsNum("Analeptic") > 1 then
			local analepticked = false
			for _, c in sgs.qlist(self.player:getHandcards()) do
				if c:isKindOf("Slash") then
					table.insert(ids, c:getEffectiveId())
				elseif c:isKindOf("Analeptic") and not analepticked then
					table.insert(ids, c:getEffectiveId())
					analepticked = true
				end
			end
			return "@DuannianCard=" .. table.concat(ids, "+")
		elseif self:getCardsNum("Peach") > 1 then
			self:sort(self.friends, "hp")
			if not self:isWeak(self.friends[1]) and not self:isWeak() then
				local peached = false
				for _, c in sgs.qlist(self.player:getHandcards()) do
					if c:isKindOf("Slash") then
						table.insert(ids, c:getEffectiveId())
					elseif c:isKindOf("Peach") and not peached then
						table.insert(ids, c:getEffectiveId())
						peached = true
					end
				end
				return "@DuannianCard:" .. table.concat(ids, "+")
			else
				return "."
			end
		else
			return "."
		end
	else
		return "."
	end
end

sgs.ai_skill_use["@@hunqu"] = function(self, prompt)
	self:sort(self.friends, "hp")
	for _, p in ipairs(self.friends) do
		if p:isWounded() then
			sgs.updateIntention(self.player, p, -100)
			return "@HunquCard=.->" .. p:objectName()
		end
	end
	if #self.enemies == 0 then return false end
	self:sort(self.enemies, "hp")
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	sgs.updateIntention(self.player, self.enemies[1], 80)
	return "@HunquCard=.->" .. self.enemies[1]:objectName()
end

sgs.ai_skill_choice.hunqu = function(self, choices)
	local player = self.player:getTag("HunquTarget"):toPlayer()
	if player then
		if self:isFriend(player) then
			return "HunquRecover"
		else
			return "HunquDamage"
		end
	end
	return "HunquDamage"
end

sgs.ai_skill_invoke.zhangqi = function(self, data)
	if #self.friends_noself == 0 then return false end

	local damage = data:toDamage()
	local to = damage.to
	if damage.damage >= to:getHp() then return false end
	local effective_friends = 0
	for _, p in ipairs(self.friends_noself) do
		if not self:willSkipPlayPhase(p) and p:faceUp() and not (self:willSkipDrawPhase(p) and p:isKongcheng())
			and (((p:getHandcardNum() >= 3 or getKnownCard(p, self.player, "Slash", true) > 0) and p:inMyAttackRange(to)
			and p:canSlash(to)) or getKnownCard(p, self.player, "Duel") + getKnownCard(p, self.player, "SavageAssault", true)
			+ getKnownCard(p, self.player, "ArcheryAttack", true) + getKnownCard(p, self.player, "FireAttack", true) > 0) then
			effective_friends = effective_friends + 1
		end
	end
	if effective_friends == 0 then return false end
	if effective_friends >= 2 then return true end

	local cards = {}
	for _, c in sgs.qlist(self.player:getHandcards()) do
		table.insert(cards, c)
	end
	for _, id in sgs.qlist(self.player:getPile("wooden_ox")) do
		table.insert(cards, sgs.Sanguosha:getCard(id))
	end
	for _, c in ipairs(cards) do
		local dummy_use = {isDummy = true, to = sgs.SPlayerList()}
		if c:isKindOf("Slash") then
			if self:slashIsAvailable() and self.player:canSlash(to, c)
				and not self:hasUnknownCard(to) and getKnownCard(to, self.player, "Jink", true) == 0 then
				return true
			end
		elseif c:isKindOf("Duel") then
			if not self.room:isProhibited(self.player, to, c) and c:targetFilter(to, sgs.PlayerList(), self.player)
				and not self:hasUnknownCard(to) and getKnownCard(to, self.player, "Slash", true) <= self:getCardsNum("Slash") then
				return true
			end
		elseif c:isKindOf("AOE") then
			self:useTrickCard(c, dummy_use)
			if dummy_use.card and not self.room:isProhibited(self.player, to, c) and not self:hasUnknownCard(to) then
				if c:isKindOf("SavageAssault") and getKnownCard(to, self.player, "Slash", true) == 0 then
					return true
				elseif c:isKindOf("ArcheryAttack") and getKnownCard(to, self.player, "Jink", true) == 0 then
					return true
				end
			end
		elseif c:isKindOf("FireAttack") then
			if not self.room:isProhibited(self.player, to, c) and c:targetFilter(to, sgs.PlayerList(), self.player)
				and not self:hasUnknownCard(to) then
				local have_all = true
				for _, c in ipairs(self:getKnownCard(to)) do
					local have_this_suit = false
					for _, c2 in sgs.qlist(self.player:getHandcards()) do
						if c:getSuit() == c2:getSuit() then
							have_this_suit = true
							break
						end
					end
					if not have_this_suit then
						have_all = false
						break
					end
				end
				if have_all then return true end
			end
		end
	end
	return false
end

sgs.ai_skill_use["@@thzhusi"] = function(self, prompt)
	self:sort(self.friends, "hp")
	for _, p in ipairs(self.friends) do
		if p:isChained() and p:isWounded() and p:hasArmorEffect("SilverLion") then
			return p
		end
	end
	for _, p in ipairs(self.friends) do
		if p:isChained() and p:isNude() then
			return p
		end
	end
	self:sort(self.enemies, "defense")
	local target = nil
	d = 1
	for _, p in ipairs(self.enemies) do
		if p:isChained() then continue end
		local damage1 = sgs.DamageStruct("thzhusi", nil, p, 1, sgs.DamageStruct_Fire)
		damage1 = self:touhouDamage(damage1, nil, p)
		local damage2 = sgs.DamageStruct("thzhusi", nil, p, 1, sgs.DamageStruct_Thunder)
		damage2 = self:touhouDamage(damage2, nil, p)
		if math.max(damage1.damage, damage2.damage) > d then
			d = math.max(damage1.damage, damage2.damage)
			target = p
		end
	end
	if target ~= nil then return "@THZhusiCard=.->" .. target:objectName() end
	local chained_enemies = {}
	for _, p in ipairs(self.enemies) do
		if p:isChained() then
			table.insert(chained_enemies, p)
		end
	end
	if #chained_enemies == 1 and not chained_enemies[1]:isNude() and (self:hasKeyEquip(chained_enemies[1])
		or getKnownCard(chained_enemies[1], self.player, "Peach", true) > 0) then
		return chained_enemies[1]
	end
	if #chained_enemies > 2 then
		for _, p in ipairs(chained_enemies) do
			local damage1 = sgs.DamageStruct("thzhusi", nil, p, 1, sgs.DamageStruct_Fire)
			damage1 = self:touhouDamage(damage1, nil, p)
			local damage2 = sgs.DamageStruct("thzhusi", nil, p, 1, sgs.DamageStruct_Thunder)
			damage2 = self:touhouDamage(damage2, nil, p)
			if math.max(damage1.damage, damage2.damage) <= 1 then return p end
		end
	end
	for _, p in ipairs(self.enemies) do
		if not p:isChained() then return p end
	end
	return "."
end

sgs.ai_skill_invoke.diaofeng = function(self, data)
	local from = data:toCardUse().from
	return from and from:isAlive() and (self:isFriend(from) or self:isEnemy(from))
end

sgs.ai_skill_choice.diaofeng = function(self, choices, data)
	local from = data:toCardUse().from
	if self:isFriend(from) then
		return "DFDraw"
	else
		return "DFDiscard"
	end
end

sgs.ai_view_as.tuji = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceHand and not card:hasFlag("using") then
		return ("amazing_grace:tuji[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local tuji_skill = {}
tuji_skill.name = "tuji"
table.insert(sgs.ai_skills, tuji_skill)
tuji_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("AmazingGrace") then return end

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local amazing_card
	for _, card in ipairs(cards) do
		if not card:isKindOf("AmazingGrace") then
			amazing_card = card
			break
		end
	end

	if amazing_card then
		local suit = amazing_card:getSuitString()
		local number = amazing_card:getNumberString()
		local card_id = amazing_card:getEffectiveId()
		local card_str = ("amazing_grace:tuji[%s:%s]=%d"):format(suit, number, card_id)
		local amazing_grace = sgs.Card_Parse(card_str)

		assert(amazing_grace)
		return amazing_grace
	end
end

sgs.ai_skill_invoke.tuji = true

sgs.ai_skill_cardask["@qiangyun-retrial"] = function(self, data)
	local judge = data:toJudge()
	local cards = self.player:getHandcards()
	if cards:isEmpty() then return "." end
	
	cards = sgs.QList2Table(cards)
	local card_id = self:getRetrialCardId(cards, judge)
	if card_id == -1 then
		if self:needRetrial(judge) and judge.reason ~= "beige" then
			self:sortByUseValue(cards, true)
			if self:getUseValue(judge.card) > self:getUseValue(cards[1]) then
				return "$" .. cards[1]:getId()
			end
		end
	elseif self:needRetrial(judge) or self:getUseValue(judge.card) > self:getUseValue(sgs.Sanguosha:getCard(card_id)) then
		local card = sgs.Sanguosha:getCard(card_id)
		return "$" .. card_id
	end

	return "."
end

sgs.ai_skill_invoke.jiahu = function(self, data)
	local target = self.player:getTag("JiahuTarget"):toPlayer()
	if self:isFriend(target) then
		sgs.updateIntention(self.player, target, -80)
		return true
	else
		return false
	end
end

function sgs.ai_cardneed.jiahu(to, card)
	return card:isRed()
end

local xianbo_skill = {name = "xianbo"}
table.insert(sgs.ai_skills, xianbo_skill)
xianbo_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("XianboCard") then return end
	if self.player:isKongcheng() then return end
	local next_player
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if p:faceUp() then next_player = p end
	end
	local card_str = "."
	if self:isEnemy(next_player) then
		local has_indul = false
		local has_light = false
		local has_supply = false
		for _, c in sgs.qlist(next_player:getJudgingArea()) do
			if c:isKindOf("Lightning") then
				has_light = true
			elseif c:isKindOf("Indulgence") then
				has_indul = true
			elseif c:isKindOf("SupplyShortage") then
				has_supply = true
			end
		end
		if has_light then
			local cards = self.player:getHandcards()
			cards = sgs.QList2Table(cards)
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if c:getSuit() == sgs.Card_Spade and c:getNumber() >= 2 and c:getNumber() <= 9 then
					card_str = ("@XianboCard=%d"):format(c:getEffectiveId())
					break
				end
			end
		elseif has_indul then
			local cards = self.player:getHandcards()
			cards = sgs.QList2Table(cards)
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if c:getSuit() ~= sgs.Card_Heart then
					card_str = ("@XianboCard=%d"):format(c:getEffectiveId())
					break
				end
			end
		elseif has_supply then
			local cards = self.player:getHandcards()
			cards = sgs.QList2Table(cards)
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if c:getSuit() ~= sgs.Card_Club then
					card_str = ("@XianboCard=%d"):format(c:getEffectiveId())
					break
				end
			end
		else
			if self.player:getHandcardNum() <= self.player:getHp() then
				for _, p in ipairs(self.enemies) do
					if not p:isKongcheng() then
						local cards = self.player:getHandcards()
						cards = sgs.QList2Table(cards)
						self:sortByKeepValue(cards)
						for _, c in ipairs(cards) do
							if self:getKeepValue(c) < 3 or (c:isKindOf("Jink") and self:getCardsNum("Jink") > 1 and not self:isWeak(p)) then
								card_str = ("@XianboCard=%d"):format(c:getEffectiveId())
								break
							end
						end
					end
					if card_str ~= "." then break end
				end
			end
		end
	elseif self:isFriend(next_player) then
		local has_indul = false
		local has_light = false
		local has_supply = false
		for _, c in sgs.qlist(next_player:getJudgingArea()) do
			if c:isKindOf("Lightning") then
				has_light = true
			elseif c:isKindOf("Indulgence") then
				has_indul = true
			elseif c:isKindOf("SupplyShortage") then
				has_supply = true
			end
		end
		if has_light then
			local cards = self.player:getHandcards()
			cards = sgs.QList2Table(cards)
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if c:getSuit() ~= sgs.Card_Spade or (c:getNumber() < 2 and c:getNumber() > 9) then
					card_str = ("@XianboCard=%d"):format(c:getEffectiveId())
					break
				end
			end
		elseif has_indul then
			local cards = self.player:getHandcards()
			cards = sgs.QList2Table(cards)
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if c:getSuit() == sgs.Card_Heart then
					card_str = ("@XianboCard=%d"):format(c:getEffectiveId())
					break
				end
			end
		elseif has_supply then
			local cards = self.player:getHandcards()
			cards = sgs.QList2Table(cards)
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if c:getSuit() == sgs.Card_Club then
					card_str = ("@XianboCard=%d"):format(c:getEffectiveId())
					break
				end
			end
		else
			for _, p in ipairs(self.enemies) do
				if not p:isKongcheng() then
					if self.player:getHandcardNum() <= self.player:getHp() then
						local cards = self.player:getHandcards()
						cards = sgs.QList2Table(cards)
						self:sortByKeepValue(cards)
						card_str = ("@XianboCard=%d"):format(cards[1]:getEffectiveId())
						break
					end
				end
			end
		end
	else
		for _, p in ipairs(self.enemies) do
			if not p:isKongcheng() then
				if self.player:getHandcardNum() <= self.player:getHp() then
					local cards = self.player:getHandcards()
					cards = sgs.QList2Table(cards)
					self:sortByKeepValue(cards)
					for _, c in ipairs(cards) do
						if self:getKeepValue(c) < 3 or (c:isKindOf("Jink") and self:getCardsNum("Jink") > 1) then
							card_str = ("@XianboCard=%d"):format(c:getEffectiveId())
							break
						end
					end
				end
			end
			if card_str ~= "." then break end
		end
	end
	if card_str == "." then return end
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.XianboCard = function(card, use, self)
	use.card = card
	return
end

sgs.ai_use_priority.XianboCard = 3.5
sgs.ai_use_value.XianboCard = 7.2
sgs.dynamic_value.control_card.XianboCard = true

sgs.ai_skill_playerchosen.xianbo = function(self, targets)
	if #self.enemies == 0 then return nil end
	self:sort(self.enemies, "chaofeng")
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() then
			sgs.updateIntention(self.player, p, 80)
			return p
		end
	end
	return nil
end

sgs.ai_skill_cardask["@huanni-show"] = function(self, data)
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isRed() then
			return c:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_skill_use["@@huanni"] = function(self, prompt)
	local room = self.room
	local reisen = room:findPlayerBySkillName("huanni")
	if not self:isEnemy(reisen) then return "." end
	local cards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByKeepValue(cards)
	local to_discard = {}
	for _, c in ipairs(cards) do
		if c:isRed() and not (c:isKindOf("Peach") and (self:getCardsNum("Peach") < 2
				or (#to_discard == 1 and sgs.Sanguosha:getCard(to_discard[1]):isKindOf("Peach") and self:getCardsNum("Peach") < 3)))
				and not (c:isKindOf("Jink") and (self:getCardsNum("Jink") < 2
				or (#to_discard == 1 and sgs.Sanguosha:getCard(to_discard[1]):isKindOf("Jink") and self:getCardsNum("Jink") < 3)))
				and not (c:isKindOf("ExNihilo")) and not (c:isKindOf("Indulgence")) then
			table.insert(to_discard, c:getEffectiveId())
		end
		if #to_discard == 2 then
			return "@HuanniCard=" .. table.concat(to_discard, "+")
		end
	end
	return "."
end

local citan_skill = {name = "citan"}
table.insert(sgs.ai_skills, citan_skill)
citan_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("CitanCard") then return end
	
	local can_enemy = false
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() and math.abs(self.player:getHandcardNum() - p:getHandcardNum()) <= self.player:getLostHp()
				and self.player:getHandcardNum() + p:getHandcardNum() > 0 then
			can_enemy = true
			break
		end
	end
	if not can_enemy then return end
	if self:isWeak() then
		local can_use = false
		for _, p in ipairs(self.enemies) do
			if p:getHandcardNum() > self.player:getHandcardNum()
					and math.abs(self.player:getHandcardNum() - p:getHandcardNum()) <= self.player:getLostHp()
					and self.player:getHandcardNum() + p:getHandcardNum() > 0 then
				can_use = true
				break
			end
		end
		if not can_use then return end
	end
	
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf("Peach") then
			return
		elseif c:isKindOf("Analeptic") then
			local all_weak = true
			for _, p in ipairs(self.enemies) do
				if not self:isWeak(p) then
					all_weak = false
					break
				end
			end
			if all_weak then return end
		end
	end
	
	local card_str = "@CitanCard=."
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.CitanCard = function(card, use, self)
	if self:isWeak() then
		self:sort(self.enemies, "handcard", true)
		for _, p in ipairs(self.enemies) do
			if p:getHandcardNum() > self.player:getHandcardNum() and math.abs(self.player:getHandcardNum() - p:getHandcardNum()) <= self.player:getLostHp()
					and getKnownCard(p, self.player, "Peach", true) >= self:getCardsNum("Peach") then
				use.card = card
				if use.to then
					sgs.updateIntention(self.player, p, 80)
					use.to:append(p)
				end
				return
			end
		end
	end
	
	if not self:isWeak() then
		self:sort(self.enemies, "handcard", true)
		for _, p in ipairs(self.enemies) do
			local damage = self:touhouDamage(sgs.DamageStruct("citan", self.player, p, 1, sgs.DamageStruct_Thunder), self.player, p)
			if damage.damage == 0 then continue end
			if p:getHandcardNum() <= self.player:getHandcardNum() and math.abs(self.player:getHandcardNum() - p:getHandcardNum()) <= self.player:getLostHp()
					and getKnownCard(p, self.player, "Peach", true) >= self:getCardsNum("Peach") and not self:cantbeHurt(p) and not self:needToLoseHp(p) then
				use.card = card
				if use.to then
					sgs.updateIntention(self.player, p, 80)
					use.to:append(p)
				end
				return
			end
		end
		for _, p in ipairs(self.enemies) do
			local damage = self:touhouDamage(sgs.DamageStruct("citan", self.player, p, 1, sgs.DamageStruct_Thunder), self.player, p)
			if damage.damage == 0 then continue end
			if p:getHandcardNum() > self.player:getHandcardNum() and math.abs(self.player:getHandcardNum() - p:getHandcardNum()) <= self.player:getLostHp()
					and getKnownCard(p, self.player, "Peach", true) >= self:getCardsNum("Peach") and not self:cantbeHurt(p) and not self:needToLoseHp(p) then
				use.card = card
				if use.to then
					use.to:append(p)
					sgs.updateIntention(self.player, p, 80)
				end
				return
			end
		end
		for _, p in ipairs(self.friends) do
			local damage = self:touhouDamage(sgs.DamageStruct("citan", self.player, p, 1, sgs.DamageStruct_Thunder), self.player, p)
			if damage.damage == 0 then continue end
			if p:getHandcardNum() <= self.player:getHandcardNum() and math.abs(self.player:getHandcardNum() - p:getHandcardNum()) <= self.player:getLostHp()
					and self:needToLoseHp(p) then
				use.card = card
				if use.to then
					sgs.updateIntention(self.player, p, -80)
					use.to:append(p)
				end
				return
			end
		end
	end
end

sgs.ai_use_priority.CitanCard = 3.3
sgs.ai_use_value.CitanCard = 7.2
sgs.dynamic_value.damage_card.CitanCard = true
sgs.dynamic_value.control_card.CitanCard = true

sgs.ai_chaofeng.marisa = 3
sgs.ai_chaofeng.aya = 4
sgs.ai_chaofeng.cirno = 2
sgs.ai_chaofeng.remilia = 4
sgs.ai_chaofeng.kyouko = 3
sgs.ai_chaofeng.kanako = 4
sgs.ai_chaofeng.koishi = 3
sgs.ai_chaofeng.mystia = 5
sgs.ai_chaofeng.tewi = 3
