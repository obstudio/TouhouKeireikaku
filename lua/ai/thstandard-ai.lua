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

function SmartAI:isEasilyAvailable(target)
	local room = target:getRoom()
	local attack_point = 0
	for _, p in sgs.qlist(room:getOtherPlayers(target)) do
		if p:inMyAttackRange(target) and self:isEnemy(p, target) then
			attack_point = attack_point + 1
		end
	end
	if attack_point == 0 then return false end
	attack_point = attack_point - target:getHandcardNum()
	return (attack_point >= 0)
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
	self:sortByUseValue(cards)
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
		if self:getCardsNum("Armor", self.player, "e") > 0 then
			return "ObtainWithSkip"
		end
		if self:getCardsNum("Weapon", self.player, "e") > 0 then
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

sgs.ai_skill_choice.shengqiang = function(self, choices)
	local room = self.room
	if self.player:isWounded() and self:hasLion() then
		return "SQDiscard"
	end
	if self:isWeak() and self.player:getArmor() and not self:needToThrowArmor() then
		return "SQDraw"
	end
	return "SQDiscard"
end

sgs.ai_skill_invoke.hongye = function(self, data)
	local players = self:getEnemies()
	for _, p in ipairs(players) do
		if not p:isNude() then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.hongye = function(self, targets)
	local players = self:getEnemies()
	self:sort(players, "chaofeng")
	for _, p in ipairs(players) do
		if not p:isNude() then
			sgs.updateIntention(self.player, p, 80)
			return p
		end
	end
	return players[1]
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
	self:sortByUseValue(cards)
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

sgs.ai_skill_invoke.shuimu = function(self, data)
	local damage = data:toDamage()
	if damage.nature == sgs.DamageStruct_Fire then
		if self.player:isKongcheng() then return false end
		for _, c in sgs.qlist(self.player:getHandcards()) do
			if (self:isWeak() and not (c:isKindOf("Jink") or c:isKindOf("Analeptic")))
					or (not self:isWeak() and not (c:isKindOf("Peach") or c:isKindOf("ExNihilo") or c:isKindOf("Snatch"))) then
				return true
			end
		end
		return false
	elseif damage.nature == sgs.DamageStruct_Thunder then
		local hp = self.player:getHp() - damage.damage - 1
		if self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 1 - hp then
			return false
		end
		
		if self.player:isChained() then
			local w = 0
			for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if self:isEnemy(p) and p:isChained() then
					w = w + 1
				elseif self:isFriend(p) and p:isChained() then
					w = w - 1
				end
			end
			if w < 0 then return false end
			if w > 0 then return true end
		end
		
		local enemy_get = false
		for _, p in ipairs(self.enemies) do
			if not p:isNude() then
				enemy_get = true
			end
		end
		if not enemy_get then return false end
		
		local n = 0
		for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if not p:isNude() then
				n = n + 1
			end
		end
		if n <= 2 then return false end
		
		return true
	end
end

sgs.ai_skill_discard.shuimu = function(self, discard_num, optional, include_equip)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local to_discard = {}
	for _, c in ipairs(cards) do
		if (self:isWeak() and not (c:isKindOf("Jink") or c:isKindOf("Analeptic")))
				or (not self:isWeak() and not (c:isKindOf("Peach") or c:isKindOf("ExNihilo") or c:isKindOf("Snatch"))) then
			table.insert(to_discard, c)
			break
		end
	end
	return to_discard
end

sgs.ai_skill_cardchosen.shuimu = function(self, who, flags)
	if self:isEnemy(who) then
		if self:isWeak(who) and not who:isKongcheng() then
			return sgs.Sanguosha:getCard(self:getCardRandomly(who, "h"))
		elseif who:isKongcheng() and not who:getCards("e"):isEmpty() then
			local card
			for _, c in sgs.qlist(who:getCards("e")) do
				if c:isKindOf("OffensiveHorse") then card = c end
			end
			for _, c in sgs.qlist(who:getCards("e")) do
				if c:isKindOf("Weapon") then card = c end
			end
			for _, c in sgs.qlist(who:getCards("e")) do
				if c:isKindOf("DefensiveHorse") then card = c end
			end
			for _, c in sgs.qlist(who:getCards("e")) do
				if c:isKindOf("Armor") and not (e:isKindOf("SilverLion") and who:isWounded() and card) then card = c end
			end
			return card
		else
			return sgs.Sanguosha:getCard(self:getCardRandomly(who, "he"))
		end
	elseif self:isFriend(who) then
		local card
		if self:isWeak(who) and not who:isKongcheng() and not who:getCards("e"):isEmpty() then
			local has_weapon = false
			local has_armor = false
			local has_defensive = false
			local has_offensive = false
			for _, c in sgs.qlist(who:getCards("e")) do
				if c:isKindOf("Weapon") then has_weapon = true
				elseif c:isKindOf("Armor") then has_armor = true
				elseif c:isKindOf("OffensiveHorse") then has_offensive = true
				elseif c:isKindOf("DefensiveHorse") then has_defensive = true
				end
			end
			if has_offensive then
				for _, c in sgs.qlist(who:getCards("e")) do
					if c:isKindOf("OffensiveHorse") then card = c end
				end
			elseif has_weapon then
				for _, c in sgs.qlist(who:getCards("e")) do
					if c:isKindOf("Weapon") then card = c end
				end
			elseif has_defensive then
				local available = false
				for _, p in ipairs(self.enemies) do
					if p:inMyAttackRange(who) then
						available = true
					end
				end
				if available then
					for _, c in sgs.qlist(who:getCards("e")) do
						if c:isKindOf("DefensiveHorse") then card = c end
					end
				else
					card = sgs.Sanguosha:getCard(self:getCardRandomly(who, "h"))
				end
			else
				card = sgs.Sanguosha:getCard(self:getCardRandomly(who, "h"))
			end
		else
			card = sgs.Sanguosha:getCard(self:getCardRandomly(who, "he"))
		end
		return card
	else
		return sgs.Sanguosha:getCard(self:getCardRandomly(who, "he"))
	end
end

local huangyan_skill = {}
huangyan_skill.name = "huangyan"
table.insert(sgs.ai_skills, huangyan_skill)
huangyan_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("HuangyanCard") or self.player:isKongcheng() then return end
	if (self:isWeak() and self.player:getHandcardNum() <= 2) then return end
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()
	self:sort(self.enemies, "handcard")
	for _, p in ipairs(self.enemies) do
		if p:getHandcardNum() == 1 or (self:isWeak(p) and p:getHandcardNum() <= 3) or (self.player:getHp() >= 2 and max_point >= 7) then
			return sgs.Card_Parse("@HuangyanCard=.")
		end
	end
	--return
	--return sgs.Card_Parse("#huangyan:.:")
end

sgs.ai_skill_use_func.HuangyanCard = function(card, use, self)
	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()
	
	for _, p in ipairs(self.enemies) do
		if not p:getCards("e"):isEmpty() then
			for _, c in sgs.qlist(p:getCards("e")) do
				if c:isKindOf("Vine") and (self:isWeak(p) or p:getHandcardNum() < 2) then
					use.card = card
					if use.to then use.to:append(p) end
					return
				end
			end
		end
	end
	
	for _, p in ipairs(self.enemies) do
		if p:getHandcardNum() == 1 then
			use.card = card
			if use.to then use.to:append(p) end
			return
		elseif p:getHandcardNum() > 0 and p:getHandcardNum() <= 3 then
			if (self:isWeak(p) and max_point >= 3) or max_point >= 7 then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
	end
end

--[[sgs.ai_cardneed.huangyan = function(to, card, self)
	local cards = to:getHandcards()
	local has_big = false
	for _, c in sgs.qlist(cards) do
		local flag = string.format("%s_%s_%s","visible",self.room:getCurrent():objectName(),to:objectName())
		if c:hasFlag("visible") or c:hasFlag(flag) then
			if c:getNumber()>10 then
				has_big = true
				break
			end
		end
	end
	if not has_big then
		return card:getNumber() > 10
	else
		return card:isKindOf("Slash")
	end
end]]--

sgs.ai_card_intention.HuangyanCard = 80
sgs.ai_use_value.HuangyanCard = 9.2
sgs.ai_use_priority.HuangyanCard = 8.5
sgs.dynamic_value.damage_card.HuangyanCard = true

sgs.ai_view_as.jingyue = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if not card:isKindOf("BasicCard") and card_place == sgs.Player_PlaceHand then
		return ("jink:jingyue[%s:%s]=%d"):format(suit, number, card_id)
	end
end

function sgs.ai_cardneed.jingyue(to, card)
	return not card:isKindOf("BasicCard")
end

sgs.ai_skill_use["@@diaoou"] = function(self, prompt)
	if #self.enemies == 0 then return "." end
	self:sort(self.enemies, "hp")
	sgs.updateIntention(self.player, self.enemies[1], 80)
	return ("@DiaoouCard=.->") .. self.enemies[1]:objectName()
end

sgs.ai_skill_cardask["@diaoou-ask"] = function(self, data)
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
	if self.player:isKongcheng() then return "." end
	if self:isVeryWeak() then
		if self.player:getHandcardNum() == self:getCardsNum("Peach") + self:getCardsNum("Analeptic") then
			return "."
		elseif self.player:getHandcardNum() <= 3 then
			local cards = sgs.QList2Table(self.player:getHandcards())
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if not c:isKindOf("Peach") and not c:isKindOf("Analeptic") then
					return "$" .. c:getEffectiveId()
				end
			end
		end
	end
	
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	--[[if self:getKeepValue(cards[1]) <= 4 then
		table.insert(to_discard, cards[1])
	end]]--
	return "$" .. cards[1]:getEffectiveId()
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
	if self.player:hasUsed("YuzhuCard") then return end
	if self:isWeak() then return end
	if #self.enemies == 0 then return end
	local all_nude = true
	for _, p in ipairs(self.enemies) do
		if not p:isNude() then
			all_nude = false
			break
		end
	end
	if all_nude then return end
	
	local card_str = "@YuzhuCard=."
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.YuzhuCard = function(card, use, self)
	self:sort(self.enemies, "chaofeng")
	for _, p in ipairs(self.enemies) do
		if not p:isNude() then
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
	end
end

sgs.ai_card_intention.YuzhuCard = 80
sgs.ai_use_priority.YuzhuCard = 10
sgs.ai_use_value.YuzhuCard = 8.3
sgs.dynamic_value.control_card.YuzhuCard = true

sgs.ai_skill_use["@@chiwa"] = function(self, prompt)
	local use = self.player:getTag("ChiwaUse"):toCardUse()
	if self.player:isNude() then return "." end
	if self:getCardsNum("Peach") == self.player:getCards("he"):length() and self:getCardsNum("Peach") < 3 then return "." end
	local invoke = false
	local target = nil
	if use.card:isKindOf("Peach") then
		if #self.friends_noself == 0 then return "." end
		for _, p in ipairs(self.friends) do
			if p:isWounded() and not use.to:contains(p) then
				target = p
				invoke = true
			end
		end
	elseif use.card:isKindOf("ExNihilo") then
		if #self.friends_noself > 0 then
			self:sort(self.friends_noself, "defense")
			for _, p in ipairs(self.friends_noself) do
				if not use.to:contains(p) then
					target = p
					invoke = true
					break
				end
			end
		end
	elseif use.card:isKindOf("GodSalvation") then
		if #self.enemies == 0 then return "." end
		self:sort(self.enemies, "hp")
		for _, p in ipairs(self.enemies) do
			if p:isWounded() and use.to:contains(p) then
				target = p
				invoke = true
				break
			end
		end
	elseif use.card:isKindOf("ArcheryAttack") then
		if #self.friends_noself == 0 then return "." end
		for _, p in ipairs(self.friends_noself) do
			if self:isWeak(p) and not self:hasVine(p) and use.to:contains(p) then
				target = p
				invoke = true
				break
			end
		end
	elseif use.card:isKindOf("AmazingGrace") then
		if #self.enemies > 0 then
			self:sort(self.enemies, "defense")
			for _, p in ipairs(self.enemies) do
				if use.to:contains(p) then
					target = p
					invoke = true
					break
				end
			end
		end
		if self:isWeak() and self.player:getCards("e"):isEmpty() and self.player:getHandcardNum() == 1 then
			local c = self.player:getHandcards():at(0)
			if c:isKindOf("Peach") or c:isKindOf("Analeptic") or c:isKindOf("Jink") then
				invoke = false
			end
		end
	end
	if not invoke then return "." end
	if target == nil then return "." end
	if self:hasLion() and self.player:isWounded() then
		self:speak("chiwa", self.player:isFemale())
		return ("@ChiwaCard=%d->%s"):format(self.player:getArmor():getEffectiveId(), target:objectName())
	end
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	if (cards[1]:isKindOf("Peach") and self:getCardsNum("Peach") < 3) or (cards[1]:isKindOf("Analeptic") and self:isVeryWeak())
			or (cards[1]:isKindOf("Jink") and (self:getCardsNum("Jink") < 2 or not use.card:isKindOf("ExNihilo")))
			or (cards[1]:isKindOf("ExNihilo")) then
		return "."
	end
	self:speak("chiwa", self.player:isFemale())
	return ("@ChiwaCard=%d->%s"):format(cards[1]:getEffectiveId(), target:objectName())
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

sgs.ai_skill_invoke.xinyan = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.from) and not (damage.from:getHandcardNum() >= 4 and not self:isWeak(damage.from)) then
		return false
	end
	local targets = {}
	for _, p in ipairs(self.enemies) do
		if damage.from and p:objectName() ~= damage.from:objectName() and not p:isKongcheng() then
			table.insert(targets, p)
		end
	end
	if #targets == 0 then return false end
	return true
end

sgs.ai_skill_playerchosen.xinyan = function(self, targets)
	local choices = sgs.QList2Table(targets)
	self:sort(choices, "chaofeng")
	for _, p in ipairs(choices) do
		if not self:isFriend(p) and not p:isKongcheng() then
			sgs.updateIntention(self.player, p, 70)
			return p
		end
	end
end

sgs.ai_skill_askforag.xinyan = function(self, card_ids)
	local id
	local peach_num = 0
	local exnihilo_num = 0
	local snatch_num = 0
	local special_num = 0
	for _, i in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(i)
		if c:isKindOf("Peach") then peach_num = peach_num + 1
		elseif c:isKindOf("ExNihilo") then exnihilo_num = exnihilo_num + 1
		elseif c:isKindOf("Snatch") then snatch_num = snatch_num + 1
		elseif c:isKindOf("Jink") or c:isKindOf("Nullification") then special_num = special_num + 1
		end
	end
	
	if peach_num > 0 then
		for _, i in ipairs(card_ids) do
			if sgs.Sanguosha:getCard(i):isKindOf("Peach") then
				id = i
			end
		end
	elseif exnihilo_num > 0 then
		for _, i in ipairs(card_ids) do
			if sgs.Sanguosha:getCard(i):isKindOf("ExNihilo") then
				id = i
			end
		end
	elseif snatch_num > 0 then
		for _, i in ipairs(card_ids) do
			if sgs.Sanguosha:getCard(i):isKindOf("Snatch") then
				id = i
			end
		end
	elseif special_num > 0 then
		for _, i in ipairs(card_ids) do
			if sgs.Sanguosha:getCard(i):isKindOf("Jink") or sgs.Sanguosha:getCard(i):isKindOf("Nullification") then
				id = i
			end
		end
	else
		local heart_num = 0
		local spade_num = 0
		local club_num = 0
		local diamond_num = 0
		for _, i in ipairs(card_ids) do
			local c = sgs.Sanguosha:getCard(i)
			if c:getSuit() == sgs.Card_Heart then
				heart_num = heart_num + 1
			elseif c:getSuit() == sgs.Card_Spade then
				spade_num = spade_num + 1
			elseif c:getSuit() == sgs.Card_Club then
				club_num = club_num + 1
			elseif c:getSuit() == sgs.Card_Diamond then
				diamond_num = diamond_num + 1
			end
		end
		if heart_num > 0 then
			for _, i in ipairs(card_ids) do
				if sgs.Sanguosha:getCard(i):getSuit() == sgs.Card_Heart then
					id = i
				end
			end
		elseif spade_num > 0 then
			for _, i in ipairs(card_ids) do
				if sgs.Sanguosha:getCard(i):getSuit() == sgs.Card_Spade then
					id = i
				end
			end
		elseif club_num > 0 then
			for _, i in ipairs(card_ids) do
				if sgs.Sanguosha:getCard(i):getSuit() == sgs.Card_Club then
					id = i
				end
			end
		elseif diamond_num > 0 then
			for _, i in ipairs(card_ids) do
				if sgs.Sanguosha:getCard(i):getSuit() == sgs.Card_Diamond then
					id = i
				end
			end
		end
	end
	
	return id
end

sgs.ai_skill_use["@@fangying"] = function(self, prompt)
	if self.player:isKongcheng() then return false end
	
	local room = self.room
	
	local use = room:getTag("fangyingTag"):toCardUse()
	if use.card:isKindOf("GodSalvation") or use.card:isKindOf("AmazingGrace") then return "." end
	if self:getUseValue(use.card) <= 6 then return "." end
	
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
				and not (c:isKindOf("Analeptic") and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 2
				and self:isWeak()) and not (c:isKindOf("Jink") and self:getCardsNum("Jink") <= 1 and self:isWeak())
				and not c:isKindOf("ExNihilo") then
			return ("@FangyingCard=%d"):format(c:getEffectiveId())
		end
	end
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

sgs.ai_skill_use["@@zhangqi"] = function(self, prompt)
	if #self.enemies == 0 then return "." end
	self:sort(self.enemies, "chaofeng")
	for _, p in ipairs(self.enemies) do
		if self.player:distanceTo(p) > 1 and self.player:inMyAttackRange(p)
				and self:getCardsNum("Snatch") + self:getCardsNum("SupplyShortage") > 0 and not p:isKongcheng() then
			sgs.updateIntention(self.player, p, 80)
			return "@ZhangqiCard=.->" .. p:objectName()
		end
	end
	self:sort(self.enemies, "defense")
	for _, p in ipairs(self.enemies) do
		if self.player:inMyAttackRange(p) and not p:isKongcheng() then
			sgs.updateIntention(self.player, p, 80)
			return "@ZhangqiCard=.->" .. p:objectName()
		end
	end
	for _, p in ipairs(self.enemies) do
		if not p:isKongcheng() then
			sgs.updateIntention(self.player, p, 80)
			return "@ZhangqiCard=.->" .. p:objectName()
		end
	end
	return "."
end

sgs.ai_skill_invoke.diaofeng = true

sgs.ai_skill_choice.diaofeng = function(self, choices, data)
	local from = data:toCardUse().from
	if self:isFriend(from) then
		return "DFDraw"
	else
		if not self:isEnemy(from) then
			return "DFDraw"
		else
			if self:isWeak() or self.player:getHandcardNum() <= 1 then
				return "DFDraw"
			else
				sgs.updateIntention(self.player, from, 70)
				return "DFDiscard"
			end
		end
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
