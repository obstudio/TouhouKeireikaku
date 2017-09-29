sgs.ai_skill_use["@@liuyue"] = function(self, prompt, method)
	local others = self.room:getOtherPlayers(self.player)
	local slash = self.player:getTag("liuyue-card"):toCard()
	others = sgs.QList2Table(others)
	local source
	for _, player in ipairs(others) do
		if player:hasFlag("LiuyueSlashSource") then
			source = player
			break
		end
	end
	self:sort(self.enemies, "defense")

	local doliuyue = function(who)
		if not self:isFriend(who) and who:hasSkill("leiji")
			and (self:hasSuit("spade", true, who) or who:getHandcardNum() >= 3)
			and (getKnownCard(who, self.player, "Jink", true) >= 1 or self:hasEightDiagramEffect(who)) then
			return "."
		end

		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			if not self.player:isCardLimited(card, method) and self.player:canSlash(who) then
				if self:isFriend(who) and not (isCard("Peach", card, self.player) or isCard("Analeptic", card, self.player)) then
					return "@LiuyueCard="..card:getEffectiveId().."->"..who:objectName()
				else
					return "@LiuyueCard="..card:getEffectiveId().."->"..who:objectName()
				end
			end
		end

		local cards = self.player:getCards("e")
		cards=sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			local range_fix = 0
			if card:isKindOf("Weapon") then range_fix = range_fix + sgs.weapon_range[card:getClassName()] - self.player:getAttackRange(false) end
			if card:isKindOf("OffensiveHorse") then range_fix = range_fix + 1 end
			if not self.player:isCardLimited(card, method) and self.player:canSlash(who, nil, true, range_fix) then
				return "@LiuyueCard=" .. card:getEffectiveId() .. "->" .. who:objectName()
			end
		end
		return "."
	end

	for _, enemy in ipairs(self.enemies) do
		if not (source and source:objectName() == enemy:objectName()) then
			local ret = doliuyue(enemy)
			if ret ~= "." then return ret end
		end
	end

	for _, player in ipairs(others) do
		if self:objectiveLevel(player) == 0 and not (source and source:objectName() == player:objectName()) then
			local ret = doliuyue(player)
			if ret ~= "." then return ret end
		end
	end


	self:sort(self.friends_noself, "defense")
	self.friends_noself = sgs.reverse(self.friends_noself)


	for _, friend in ipairs(self.friends_noself) do
		if not self:slashIsEffective(slash, friend) or self:findLeijiTarget(friend, 50, source) then
			if not (source and source:objectName() == friend:objectName()) then
				local ret = doliuyue(friend)
				if ret ~= "." then return ret end
			end
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		if self:needToLoseHp(friend, source, true) or self:getDamagedEffects(friend, source, true) then
			if not (source and source:objectName() == friend:objectName()) then
				local ret = doliuyue(friend)
				if ret ~= "." then return ret end
			end
		end
	end

	if (self:isWeak() or self:hasHeavySlashDamage(source, slash)) and source:hasWeapon("axe") and source:getCards("he"):length() > 2
	  and not self:getCardId("Peach") and not self:getCardId("Analeptic") then
		for _, friend in ipairs(self.friends_noself) do
			if not self:isWeak(friend) then
				if not (source and source:objectName() == friend:objectName()) then
					local ret = doliuyue(friend)
					if ret ~= "." then return ret end
				end
			end
		end
	end

	if (self:isWeak() or self:hasHeavySlashDamage(source, slash)) and not self:getCardId("Jink") then
		for _, friend in ipairs(self.friends_noself) do
			if not self:isWeak(friend) or (self:hasEightDiagramEffect(friend) and getCardsNum("Jink", friend) >= 1) then
				if not (source and source:objectName() == friend:objectName()) then
					local ret = doliuyue(friend)
					if ret ~= "." then return ret end
				end
			end
		end
	end
	return "."
end

sgs.ai_card_intention.LiuyueCard = function(self, card, from, to)
	sgs.ai_liuyue_effect = true
	if not hasExplicitRebel(self.room) then sgs.ai_liuyue_user = from
	else sgs.ai_liuyue_user = nil end
end

function sgs.ai_slash_prohibit.liuyue(self, from, to, card)
	if self:isFriend(to, from) then return false end
	if from:hasFlag("NosJiefanUsed") then return false end
	if to:isNude() then return false end
	for _, friend in ipairs(self:getFriendsNoself(from)) do
		if to:canSlash(friend, card) and self:slashIsEffective(card, friend, from) then return true end
	end
end

function sgs.ai_cardneed.liuyue(to, card)
	return to:getCards("he"):length() <= 2
end

sgs.ai_skill_invoke.fangmu = function(self, data)
	local damage = data:toDamage()
	if not self:isFriend(damage.to) then return false end
	if not self.player:isLord() and ((not self:isWeak() or self:getCardsNum("Peach") + self:getCardsNum("Analeptic") >= 1)
			and self:isWeak(damage.to)) or (damage.to:isLord() and (self:isWeak(damage.to) or not self:isVeryWeak())) then
		sgs.updateIntention(self.player, damage.to, -80)
		return true
	end
	if not self:isWeak() and damage.damage > 1 and not damage.to:hasSkill("jinlun") then return true end
	return false
end

sgs.ai_view_as.yaodao = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceHand and card:isKindOf("BasicCard") and not card:hasFlag("using")
			and sgs.Sanguosha:getCurrentCardUseReason() ~= sgs.CardUseStruct_CARD_USE_REASON_RESPONSE then
		return ("soul_slash:yaodao[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local yaodao_skill = {}
yaodao_skill.name = "yaodao"
table.insert(sgs.ai_skills, yaodao_skill)
yaodao_skill.getTurnUseCard = function(self, inclusive)
	if self.player:hasFlag("YaodaoUsed") then return end
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)

	local spade_card
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		local soul_slash = sgs.Sanguosha:cloneCard("soul_slash", card:getSuit(), card:getNumber())
		if card:isKindOf("BasicCard") and (self:getUseValue(card) <= self:getKeepValue(soul_slash) or inclusive) then
			spade_card = card
			break
		end
	end

	if spade_card then
		local suit = spade_card:getSuitString()
		local number = spade_card:getNumberString()
		local card_id = spade_card:getEffectiveId()
		local card_str = ("soul_slash:yaodao[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)

		assert(slash)
		return slash
	end
end

sgs.ai_skill_invoke.thbanling = function(self, data)
	local players = sgs.Table2SPlayerList(self.friends)
	if self:touhouFindPlayerToDraw(true, self.player:getLostHp(), players) then
		return true
	end
	return false
end

sgs.ai_skill_askforag.thbanling = function(self, card_ids)
	local maxvalue = -10
	local choice
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if self:getKeepValue(card) > maxvalue then
			maxvalue = self:getKeepValue(card)
			choice = id
		end
	end
	return choice
end

sgs.ai_skill_playerchosen.thbanling = function(self, targets)
	local players = sgs.Table2SPlayerList(self.friends)
	local player = self:touhouFindPlayerToDraw(true, self.player:getLostHp(), players)
	sgs.updateIntention(self.player, player, -80)
	return player
end

sgs.ai_skill_invoke.huadie = function(self, data)
	local player = data:toDamage().from
	if self:isFriend(player) or (self:isEnemy(player) and not player:isKongcheng()) then
		return true
	end
	return false
end

sgs.ai_skill_choice.huadie = function(self, choices)
	local playerdata = self.player:getTag("HuadieTarget")
	local player
	if playerdata then
		player = playerdata:toPlayer()
	end
	if self:isFriend(player) then
		sgs.updateIntention(self.player, player, -80)
		return "HuadieDraw"
	else
		sgs.updateIntention(self.player, player, 80)
		return "HuadieThrow"
	end
end

sgs.ai_skill_use["@@lie"] = function(self, prompt)
	if self.player:isKongcheng() then return "." end
	local room = self.room
	local player
	for _, p in sgs.qlist(room:getOtherPlayers(self.player)) do
		if p:hasFlag("LieTarget") then
			player = p
			break
		end
	end
	if not player then return "." end
	
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if self:isFriend(player) then
		if player:getMark("lie") == 0 then
			return "@LieCard=" .. cards[1]:getEffectiveId() .. "->" .. player:objectName()
		end
		for _, c in ipairs(cards) do
			if not (c:isKindOf("Analeptic") and self.player:getHp() < player:getHp()
					and self:getCardsNum("Analeptic") + self:getCardsNum("Peach") < 2)
					and not (c:isKindOf("TrickCard") and not c:isKindOf("Nullification") and (self:hasIndul(player)
					or self:isVeryWeak(player)) and not self:hasIndul()) then
				if self.player:getHandcardNum() > 1 then
					sgs.updateIntention(self.player, player, -80)
				end
				return "@LieCard=" .. c:getEffectiveId() .. "->" .. player:objectName()
			end
		end
	elseif self:isEnemy(player) then
		if player:getMark("lie") > 0 then
			if self.player:getHandcardNum() == 1 and self.player:isWounded() then
				local c = self.player:getHandcards():at(0)
				if not c:isKindOf("Peach") and not c:isKindOf("Analeptic") and not c:isKindOf("Jink") and not c:isKindOf("ExNihilo")
						and not c:isKindOf("Dismantlement") and not c:isKindOf("Snatch") and not c:isKindOf("Haze") then
					return "@LieCard=" .. c:getEffectiveId() .. "->" .. player:objectName()
				end
			end
			return "."
		end
		for _, c in ipairs(cards) do
			if not (c:isKindOf("Jink") or c:isKindOf("Peach") or c:isKindOf("Analeptic")
					or (c:isKindOf("TrickCard") and not c:isKindOf("AmazingGrace") and not c:isKindOf("GodSalvation"))
					or (c:isKindOf("Armor") and not player:getArmor()) or (c:isKindOf("DefensiveHorse") and not player:getDefensiveHorse())) then
				return "@LieCard=" .. c:getEffectiveId() .. "->" .. player:objectName()
			end
		end
	end
	return "."
end

sgs.ai_skill_invoke.anyun = function(self, data)
	return true
end

local julang_skill = {}
julang_skill.name = "julang"
table.insert(sgs.ai_skills, julang_skill)
julang_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("JulangCard") then return end
	if self.player:getCards("he"):length() < 2 then return nil end
	if #self.enemies + #self.friends_noself == 0 then return nil end
	if #self.friends_noself > 0 then
		for _, p in ipairs(self.friends_noself) do
			if p:isWounded() and self:hasLion(p) then
				local cards = self.player:getCards("he")
				cards = sgs.QList2Table(cards)
				local first
				local second
				self:sortByUseValue(cards, true)
				if self.player:getArmor() and self.player:getArmor():isKindOf("SilverLion") and self.player:isWounded() then
					first = self.player:getArmor():getEffectiveId()
				end
				for _, card in ipairs(cards) do
					if not ((card:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
							or (card:isKindOf("Jink") and self:getCardsNum("Jink") == 1 and self:isWeak())
							or (card:isKindOf("Analeptic") and self:isWeak()
							and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 2)) then
						if not first then first = card:getEffectiveId()
						elseif first ~= card:getEffectiveId() and not second then second = card:getEffectiveId()
						end
						if first and second then break end
					end
				end
				if not second or not first then return nil end
				local card_str = ("@JulangCard=%d+%d"):format(first, second)
				local skillcard = sgs.Card_Parse(card_str)
				assert(card_str)
				return skillcard
			end
		end
	elseif #self.enemies > 0 then
		for _, p in ipairs(self.enemies) do
			if (p:getCards("e"):length() > 0 and not (self:hasLion(p) and p:isWounded())) or (p:getCards("e"):length() == 0) then
				local cards = self.player:getCards("he")
				cards = sgs.QList2Table(cards)
				local first
				local second
				self:sortByUseValue(cards, true)
				if self.player:getArmor() and self.player:getArmor():isKindOf("SilverLion") and self.player:isWounded() then
					first = self.player:getArmor():getEffectiveId()
				end
				for _, card in ipairs(cards) do
					if not ((card:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
							or (card:isKindOf("Jink") and self:getCardsNum("Jink") == 1 and self:isWeak())
							or (card:isKindOf("Analeptic") and self:isWeak()
							and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 2)) then
						if not first then first = card:getEffectiveId()
						elseif first ~= card:getEffectiveId() and not second then second = card:getEffectiveId()
						end
						if first and second then break end
					end
				end
				if not second or not first then return nil end
				local card_str = ("@JulangCard=%d+%d"):format(first, second)
				local skillcard = sgs.Card_Parse(card_str)
				assert(card_str)
				return skillcard
			end
		end
	end
	return nil
end

sgs.ai_skill_use_func.JulangCard = function(card, use, self)
	for _, p in ipairs(self.enemies) do
		if p:getCards("e"):length() > 0 then
			has_lion = false
			for _, c in sgs.qlist(p:getCards("e")) do
				if c:isKindOf("SilverLion") then
					has_lion = true
					break
				end
			end
			if not (has_lion and p:isWounded()) then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
	end
end

sgs.ai_skill_choice.julang = function(self, choices)
	if self.player:isWounded() and self:hasLion() then
		return "JulangThrow"
	end
	if self.player:getCards("e"):length() >= 3 and not self:isWeak() then
		return "JulangDamage"
	end
	return "JulangThrow"
end

sgs.ai_card_intention.JulangCard = 80
sgs.ai_use_value.JulangCard = 8.5
sgs.ai_use_priority.JulangCard = 8.9
sgs.dynamic_value.damage_card.JulangCard = true
sgs.dynamic_value.control_card.JulangCard = true

sgs.ai_skill_use["@@bamao"] = function(self, prompt)
	local room = self.room
	if #self.enemies == 0 then return "." end
	self:sort(self.enemies, "handcard", true)
	for _, p in ipairs(self.enemies) do
		if p:getHandcardNum() >= 6 and (self:isWeak() or p:getHp() <= 2) then
			return "@BamaoCard=.->" .. p:objectName()
		end
	end
	self:sort(self.enemies, "hp")
	for _, p in ipairs(self.enemies) do
		if (p:getHandcardNum() >= 2 and p:getHp() <= 1) or (p:getHandcardNum() >= 3 and p:getHp() <= 2) then
			for _, q in ipairs(self.friends_noself) do
				if q:inMyAttackRange(p) and not (q:isKongcheng() and q:isVeryWeak()) then
					return "@BamaoCard=.->" .. p:objectName()
				end
			end
		end
	end
	self:sort(self.enemies, "handcard", true)
	if self:isWeak() and self.player:getHandcardNum() <= 1
			and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") == 0 then
		for _, p in ipairs(self.enemies) do
			if p:getHandcardNum() >= 3 then
				return "@BamaoCard=.->" .. p:objectName()
			end
		end
	end
	self:sort(self.friends, "hp")
	for _, p in ipairs(self.friends) do
		if self:isVeryWeak(p) and self:getCardsNum("Peach") == 0 then
			for _, q in ipairs(self.enemies) do
				if q:getHandcardNum() >= 4 then
					return "@BamaoCard=.->" .. p:objectName()
				end
			end
		end
	end
	return "."
end

sgs.ai_skill_invoke.mianxian = function(self, data)
	local room = self.room
	local player = room:getCurrent()
	if self:isFriend(player) then
		if (self:hasLion(player) and player:isWounded()) or self:hasJudges(player) then
			return true
		end
	elseif self:isEnemy(player) then
		for _, c in sgs.qlist(player:getCards("ej")) do
			if not (c:isKindOf("SilverLion") and player:isWounded()) and not c:isKindOf("TrickCard") then
				return true
			end
		end
	end
	return false
end

sgs.ai_skill_cardchosen.mianxian = function(self, who)
	if self:isFriend(who) then
		if self:hasLion(who) and who:isWounded() then
			return who:getArmor()
		elseif self:hasJudges(who) then
			for _, c in sgs.qlist(who:getCards("j")) do
				if c:isKindOf("Lightning") and who:getHp() < 3 then
					return c
				end
			end
			for _, c in sgs.qlist(who:getCards("j")) do
				if c:isKindOf("Indulgence") then
					return c
				end
			end
			for _, c in sgs.qlist(who:getCards("j")) do
				if c:isKindOf("SupplyShortage") then
					return c
				end
			end
			for _, c in sgs.qlist(who:getCards("j")) do
				if c:isKindOf("Lightning") then
					return c
				end
			end
		end
	elseif self:isEnemy(who) then
		local cards = sgs.QList2Table(who:getCards("ej"))
		self:sortByUseValue(cards, true)
		for _, c in ipairs(cards) do
			if not (c:isKindOf("SilverLion") and who:isWounded()) and not c:isKindOf("TrickCard") then
				return c
			end
		end
	end
	local cards = sgs.QList2Table(who:getCards("ej"))
	self:sortByUseValue(cards, true)
	return cards[1]
end

sgs.ai_skill_invoke.dianyin = function(self, data)
	local damage = data:toDamage()
	if damage.to:getHp() <= damage.damage then
		if self.player:isLord() and not self:isEnemy(damage.to) then
			return false
		end
		return true
	end
	if damage.to:hasSkill("xinyan") then
		return false
	end
	for _, p in ipairs(self.enemies) do
		if p:hasSkill("kuaiqing") and damage.card:isKindOf("Slash") then
			return false
		end
	end
	return true
end

sgs.ai_skill_invoke.yingchong = function(self, data)
	if #self.enemies == 0 then return false end
	self:sort(self.enemies, "handcard", true)
	for _, p in ipairs(self.enemies) do
		local x = 0
		if not self:hasIndul() then
			x = self:getCardsNum("Peach")
		end
		if p:getHandcardNum() > self.player:getHandcardNum() and (p:getHandcardNum() - 1 >= self.player:getHp() + x
				or getKnownCard(p, self.player, "Peach", true) > 0) then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.yingchong = function(self, data)
	self:sort(self.enemies, "handcard", true)
	for _, p in ipairs(self.enemies) do
		local x = 0
		if not self:hasIndul() then
			x = self:getCardsNum("Peach")
		end
		if p:getHandcardNum() > self.player:getHandcardNum() and (p:getHandcardNum() - 1 >= self.player:getHp() + x
				or getKnownCard(p, self.player, "Peach", true) > 0) then
			return p
		end
	end
end

sgs.ai_skill_use["@@yingchong"] = function(self, prompt)
	local keep_red = {}
	local not_keep_red = {}
	local keep_black = {}
	local not_keep_black = {}
	local peach = self:getCardsNum("Peach") + self:getCardsNum("Analeptic")
	local jink = self:getCardsNum("Jink")
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards, true)
	for _, c in ipairs(cards) do
		if c:isRed() then
			if (c:isKindOf("Peach") or (c:isKindOf("Analeptic") and self:isWeak())) and peach < 3 then
				table.insert(keep_red, c)
				peach = peach - 1
			elseif c:isKindOf("Jink") and jink < 2 then
				table.insert(keep_red, c)
				jink = jink - 1
			elseif c:isKindOf("ExNihilo") or c:isKindOf("Snatch") or c:isKindOf("Indulgence") then
				table.insert(keep_red, c)
			else
				table.insert(not_keep_red, c)
			end
		elseif c:isBlack() then
			if c:isKindOf("Analeptic") and self:isWeak() and peach < 3 then
				table.insert(keep_black, c)
				peach = peach - 1
			elseif c:isKindOf("Haze") then
				table.insert(keep_black, c)
			else
				table.insert(not_keep_black, c)
			end
		end
	end
	self:sortByKeepValue(keep_black, true)
	self:sortByKeepValue(keep_red, true)
	self:sortByKeepValue(not_keep_black)
	self:sortByKeepValue(not_keep_red)
	local card_ids = {}
	if #not_keep_black + #not_keep_red > #keep_black + #keep_red + 2 then
		if #not_keep_red > 0 and (#not_keep_black == 0 or self:getKeepValue(not_keep_black[1]) > self:getKeepValue(not_keep_red[1])) then
			table.insert(card_ids, not_keep_red[1]:getEffectiveId())
			table.remove(not_keep_red, 1)
		elseif #not_keep_black > 0 then
			table.insert(card_ids, not_keep_black[1]:getEffectiveId())
			table.remove(not_keep_black, 1)
		end
	elseif #keep_red > 0 then
		table.insert(card_ids, keep_red[1]:getEffectiveId())
		table.remove(keep_red, 1)
	elseif #keep_black > 0 then
		table.insert(card_ids, keep_black[1]:getEffectiveId())
		table.remove(keep_black, 1)
	elseif #not_keep_red > 0 then
		table.insert(card_ids, not_keep_red[#not_keep_red]:getEffectiveId())
		table.remove(not_keep_red, #not_keep_red)
	else
		table.insert(card_ids, not_keep_black[#not_keep_black]:getEffectiveId())
		table.remove(not_keep_black, #not_keep_black)
	end
	if #cards > 2 and #not_keep_red > 0 and not sgs.Sanguosha:getCard(card_ids[1]):isRed() then
		table.insert(card_ids, not_keep_red[1]:getEffectiveId())
		table.remove(not_keep_red, 1)
	elseif #cards > 2 and #not_keep_black > 0 then
		table.insert(card_ids, not_keep_black[1]:getEffectiveId())
		table.remove(not_keep_black, 1)
	end
	return "@YingchongCard=" .. table.concat(card_ids, "+")
end

sgs.ai_skill_choice.yingchong = function(self, choices)
	local whole_id = self.player:getTag("YingchongCards"):toInt()
	local player = self.player:getTag("YingchongTarget"):toPlayer()
	local card_ids = sgs.QList2Table(sgs.Sanguosha:getCard(whole_id):getSubcards())
	self:sortByKeepValue(card_ids)
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if (c:isKindOf("Peach") or (c:isKindOf("Jink") and (player:getHandcardNum() <= 3
				or getKnownCard(player, self.player, "Jink", true) <= 1))) and (self:isWeak(player) or player:hasSkills(sgs.masochism_skill))
				and not self.player:getHandcardNum() < self.player:getHp() then
			return "YCThrow"
		end
	end
	local red = 0
	for _, id in ipairs(card_ids) do
		local c = sgs.Sanguosha:getCard(id)
		if c:isRed() then
			red = red + 1
		end
	end
	if red >= 2 then
		return "YCThrow"
	end
	return "YCSnatch"	
end

sgs.ai_skill_use["@@anchao"] = function(self, prompt)
	if sgs.turncount <= 1 then return "." end
	if self.player:getMark("@anchao") > 0 then return "." end
	local use = self.player:getTag("AnchaoData"):toCardUse()
	
	local has_valid_gainer = false
	for _, p in ipairs(self.friends) do
		if use.from:objectName() ~= p:objectName() and not use.to:contains(p) then
			has_valid_gainer = true
			break
		end
	end
	if not has_valid_gainer then
		return "."
	end
	
	if use.card:isKindOf("Collateral") then
		local enemy_with_weapon = false
		for _, p in ipairs(self.enemies) do
			if p:getWeapon() then
				enemy_with_weapon = true
				break
			end
		end
		if not enemy_with_weapon then
			return "."
		end
	elseif use.card:isKindOf("IronChain") then
		local enemy_not_chained = 0
		local friend_chained = 0
		for _, p in ipairs(self.enemies) do
			if not p:isChained() then
				enemy_not_chained = enemy_not_chained + 1
			end
		end
		for _, p in ipairs(self.friends) do
			if p:isChained() then
				friend_chained = friend_chained + 1
			end
		end
		if enemy_not_chained == 0 and friend_chained == 0 then
			return "."
		end
	end
	
	self:sort(self.friends, "defense")
	for _, p in ipairs(self.friends) do
		if p:hasSkill("jingyue") and self:isWeak(p) and use.from:objectName() ~= p:objectName() and not use.to:contains(p) then
			return "@AnchaoCard=.->" .. p:objectName()
		end
	end
	
	local p = self.player
	while p:getNextAlive():objectName() ~= self.player:objectName() and (self:hasIndul(p)
			or (use.from and use.from:objectName() == p:objectName()) or (use.to:contains(p))
			or p:getHandcardNum() >= p:getMaxCards() + 2 or not self:isFriend(p)) do
		p = p:getNextAlive()
	end
	if (use.from and use.from:objectName() == p:objectName()) or (use.to:contains(p)) then
		self:sort(self.friends, "handcard")
		for _, player in ipairs(self.friends) do
			if not self:hasIndul(player) then
				p = player
				break
			end
		end
		if not self:hasIndul(p) and self:isFriend(p) then
			return "@AnchaoCard=.->" .. p:objectName()
		end
	elseif self:isFriend(p) then
		return "@AnchaoCard=.->" .. p:objectName()
	end
	return "."
end

sgs.ai_skill_cardask["@zonghuo-discard"] = function(self, data)
	local damage = data:toDamage()
	if not self:isEnemy(damage.to) then return "." end
	local chained_friends = 0
	local chained_enemies = 0
	for _, p in ipairs(self.friends) do
		if p:isChained() then
			if p:isWeak() then
				return "."
			end
			chained_friends = chained_friends + 1
		end
	end
	if chained_friends >= 2 then return "." end
	for _, p in ipairs(self.enemies) do
		if p:isChained() then
			chained_enemies = chained_enemies + 1
		end
	end
	if chained_friends == 1 and chained_enemies <= 1 then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards)
	for _, c in ipairs(cards) do
		if c:isRed() and not (c:isKindOf("Peach") or c:isKindOf("ExNihilo")
				or (c:isKindOf("Jink") and self:getCardsNum("Jink") == 1)) then
			sgs.updateIntention(self.player, damage.to, 80)
			return c:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_skill_choice.zonghuo = function(self, choices)
	local chained_friends = 0
	local chained_weak_friends = 0
	local damage = self.player:getTag("ZonghuoDamage"):toDamage()
	for _, p in ipairs(self.friends_noself) do
		if p:isChained() then
			chained_friends = chained_friends + 1
			if p:getHp() <= damage.damage then
				chained_weak_friends = chained_weak_friends + 1
			end
		end
	end
	if chained_weak_friends >= 2 or (chained_weak_friends == 1 and self.player:getHp() > damage.damage + 1) then
		return "ZHAdd"
	end
	return "ZHFire"
end

sgs.ai_skill_use["@@jinpan"] = function(self, prompt)
	local maxnum = self.player:getLostHp()
	self:sort(self.friends, "hp")
	local friends_list = {}
	for _, p in ipairs(self.friends) do
		if p:isChained() then
			table.insert(friends_list, p)
		end
	end
	local chain_list = {}
	for i = 1, math.min(maxnum, #friends_list), 1 do
		sgs.updateIntention(self.player, friends_list[i], -80)
		table.insert(chain_list, friends_list[i]:objectName())
	end
	maxnum = maxnum - #chain_list
	if maxnum > 0 then
		local enemies_list = {}
		for _, p in ipairs(self.enemies) do
			if not p:isChained() then
				table.insert(enemies_list, p)
			end
		end
		for i = 1, math.min(maxnum, #enemies_list), 1 do
			sgs.updateIntention(self.player, enemies_list[i], 80)
			table.insert(chain_list, enemies_list[i]:objectName())
		end
	end
	if #chain_list > 0 then return "@JinpanCard=.->" .. table.concat(chain_list, "+") end
	return "."
end

sgs.ai_skill_cardask["@jinpan-heart"] = function(self, data)
	if self.player:isChained() then
		return "."
	else
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		for _, c in ipairs(cards) do
			if c:getSuit() == sgs.Card_Heart and not c:isKindOf("Peach") and not c:isKindOf("ExNihilo")
					and not (c:isKindOf("Jink") and self:getCardsNum("Jink") < 3) then
				return c:getEffectiveId()
			end
		end
	end
	return "."
end

sgs.ai_skill_invoke.michun = function(self, data)
	local judge = data:toJudge()
	if self:isFriend(judge.who) then
		sgs.updateIntention(self.player, judge.who, -80)
		return true
	end
	sgs.updateIntention(self.player, judge.who, 40)
	return false
end

local huawu_skill = {}
huawu_skill.name = "huawu"
table.insert(sgs.ai_skills, huawu_skill)
huawu_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("HuawuCard") then return end
	if self.player:getHandcardNum() < 2 then return end
	if #self.friends_noself == 0 then return end
	self:sort(self.friends_noself, "hp")
	if self:isVeryWeak() or (self:isWeak() and self.player:getHandcardNum() == 2) then return end
	if self.player:getHandcardNum() - self:getCardsNum("Peach") - self:getCardsNum("ExNihilo") <= 1 then return end
	local wounded = {}
	for _, p in ipairs(self.friends_noself) do
		if p:isWounded() then
			table.insert(wounded, p)
		end
	end
	if #wounded == 0 then return end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards)
	local card_str = ("@HuawuCard=%d+%d"):format(cards[1]:getEffectiveId(), cards[2]:getEffectiveId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.HuawuCard = function(card, use, self)
	self:sort(self.friends_noself, "defense")
	local wounded = {}
	for _, p in ipairs(self.friends_noself) do
		if p:isWounded() then
			if self:isWeak(p) then
				use.card = card
				if use.to then use.to:append(p) end
				return
			end
		end
		table.insert(wounded, p)
	end
	if #wounded == 1 then
		if wounded[1]:isWounded() then
			use.card = card
			if use.to then use.to:append(wounded[1]) end
			return
		end
	end
	if #wounded == 2 then
		use.card = card
		if use.to then use.to:append(wounded[1]) end
		if use.to then use.to:append(wounded[2]) end
		return
	end
	return
end

sgs.ai_card_intention.HuawuCard = -80
sgs.ai_use_priority.HuawuCard = 7.2
sgs.ai_use_value.HuawuCard = 8.6
sgs.dynamic_value.benefit.HuawuCard = true

sgs.ai_skill_use["@@feixiang"] = function(self, prompt)
	if #self.friends == 0 then return "." end
	local players = sgs.Table2SPlayerList(self.friends)
	--local player = self:touhouFindPlayerToDraw(true, 2, players)
	self:sort(self.friends, "defense")
	for _, p in ipairs(self.friends) do
		if p:getHandcardNum() < p:getHp() then
			sgs.updateIntention(self.player, p, -80)
			return "@FeixiangCard=.->" .. p:objectName()
		end
	end
	if player then
		sgs.updateIntention(self.player, player, -80)
		return "@FeixiangCard=.->" .. player:objectName()
	end
	return "."
end

local guiqiao_skill = {}
guiqiao_skill.name = "guiqiao"
table.insert(sgs.ai_skills, guiqiao_skill)
guiqiao_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("GuiqiaoCard") then return end
	if #self.enemies == 0 or #self.friends_noself == 0 then return end
	local spades = {}
	for _, c in sgs.qlist(self.player:getCards("he")) do
		if c:getSuit() == sgs.Card_Spade and not (c:isKindOf("Analeptic") and self.player:getHp() <= 2) then
			table.insert(spades, c)
		end
	end
	if #spades == 0 then return end
	
	self:sortByUseValue(spades)
	local card_str = ("@GuiqiaoCard=%d"):format(spades[1]:getEffectiveId())
	if not self.player:faceUp() then
		return sgs.Card_Parse(card_str)
	end
	if self.player:getHp() <= 2 and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") + self:getCardsNum("Jink") <= 2
			and (#self.enemies > 1 or self.enemies[1]:getHp() >= 3 or (self.enemies[1]:getHp() >= 2
			and self.enemies[1]:getHandcardNum() >= 2)) and self.player:getRole() == "lord" then
		return
	end
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.GuiqiaoCard = function(card, use, self)
	self:sort(self.friends_noself, "defense")
	use.card = card
	if use.to then use.to:append(self.friends_noself[1]) end
	return
end

sgs.ai_skill_playerchosen.guiqiao = function(self, targets)
	self:sort(self.enemies, "hp")
	sgs.updateIntention(self.player, self.enemies[1], 80)
	return self.enemies[1]
end

function sgs.ai_cardneed.guiqiao(to, card)
	return card:getSuit() == sgs.Card_Spade
end

sgs.ai_card_intention.GuiqiaoCard = -80
sgs.ai_use_priority.GuiqiaoCard = 8.2
sgs.ai_use_value.GuiqiaoCard = 9.2
sgs.dynamic_value.damage_card.GuiqiaoCard = true

sgs.ai_skill_invoke.chuangshi = function(self, data)
	local player = data:toPlayer()
	if self:isFriend(player) then
		return true
	elseif player:getHandcardNum() <= 2 then
			return true
	end
	return false
end

sgs.ai_skill_choice.chuangshi = function(self, choices)
	local player = self.player:getTag("ChuangshiTarget"):toPlayer()
	if self:isWeak(player) then
		if player:hasSkill("jingyue") then
			if player:getEquips():length() <= 1 then
				return "equip"
			else
				return "trick"
			end
		end
		return "basic"
	end
	if player:hasSkill("molian") then
		if player:getEquips():length() <= 2 then
			return "equip"
		end
	end
	if player:getHandcardNum() >= 4 or (player:getHandcardNum() >= 3 and player:getHp() >= 3) then
		if player:getEquips():length() <= 1 then
			return "equip"
		else
			return "trick"
		end
	end
	return "basic"
end

sgs.ai_skill_askforag.chuangshi = function(self, card_ids)
	if #card_ids == 0 then return -1 end
	local max_keep_value = -10
	local choice = -1
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if self:cardNeed(card) > max_keep_value then
			max_keep_value = self:getKeepValue(card)
			choice = id
		end
	end
	return choice
end

sgs.ai_skill_use["@@shenquan"] = function(self, prompt)
	if #self.friends == 0 then return "." end
	self:sort(self.friends, "hp")
	for _, c in ipairs(self.friends) do
		if c:isWounded() and c:isAlive() then
			return "@ShenquanCard=.->" .. c:objectName()
		end
	end
	return "."
end

local shenfeng_skill = {}
shenfeng_skill.name = "shenfeng"
table.insert(sgs.ai_skills, shenfeng_skill)
shenfeng_skill.getTurnUseCard = function(self)
	local room = self.player:getRoom()
	for _, p in sgs.qlist(room:getAlivePlayers()) do
		if not self:isFriend(p) and not self:isEnemy(p) then
			return
		end
	end
	if self.player:getMark("@kamikaze") <= 0 then return end
	local x = 1
	local v = 0
	local y = self.player:getHandcardNum()
	local p = self.player:getNextAlive()
	while p:objectName() ~= self.player:objectName() do
		room:writeToConsole("Get Shenfeng Target " .. p:objectName())
		if not p:isAlive() then continue end
		local peaches = getKnownCard(p, self.player, "Peach", true)
		local anas = getKnownCard(p, self.player, "Analeptic", true)
		if self:isEnemy(p) then
			if self:isWeak(p) then
				v = v + 0.5
			end
			if p:getEquips():length() >= x and p:getPile("wooden_ox"):length() <= 2 then
				v = v - 0.5 + 0.2 * (p:getEquips():length() + p:getPile("wooden_ox"):length())
				x = x + 1
				if p:isWounded() and self:hasLion(p) then
					v = v - 0.6
				end
				if p:hasSkills(sgs.need_equip_skill) then
					v = v + 0.2
				end
				if p:hasSkills(sgs.lose_equip_skill) then
					v = v - 0.2
				end
			elseif x <= 2 and ((p:getHp() >= x + 2 and (getKnownCard(p, self.player, "Peach", true) > 0 or self.player:getHandcardNum() >= 3)) or
					(p:getHp() >= x + 1 and p:hasSkills(sgs.masochism_skill))) then
				v = v - 0.4 * p:getHp()
			elseif not p:isKongcheng() and (p:getHp() + peaches + anas <= x or x >= 3 or (p:getHandcardNum() <= 2 and peaches + anas == 0)) then
				v = v - 0.5 + 0.2 * p:getHandcardNum() - 0.04 * y
				y = y + p:getHandcardNum()
			else
				v = v + 0.4 * x
			end
		elseif self:isFriend(p) then
			if p:getRole() == "lord" then
				if self:isVeryWeak(p) then
					v = v - 4
				end
				if self.player:getEquips() < x and self.player:isKongcheng() then
					v = v - 3.7
				end
			end
			if self:isWeak(p) then
				v = v - 0.5
			end
			if p:getEquips():length() >= x and p:getPile("wooden_ox"):length() <= 2 then
				v = v + 0.5 - 0.15 * (p:getEquips():length() + p:getPile("wooden_ox"):length())
				x = x + 1
				if p:isWounded() and self:hasLion(p) then
					v = v + 0.8
				end
				if p:hasSkills(sgs.need_equip_skill) then
					v = v - 0.2
				end
				if p:hasSkills(sgs.lose_equip_skill) then
					v = v + 0.2
				end
			elseif x <= 2 and ((p:getHp() >= x + 2 and (getKnownCard(p, self.player, "Peach", true) > 0 or self.player:getHandcardNum() >= 3)) or
					(p:getHp() >= x + 1 and p:hasSkills(sgs.masochism_skill))) then
				v = v + 0.4 * p:getHp() - 0.08 * x
			elseif not p:isKongcheng() then
				v = v + 0.5 - 0.2 * p:getHandcardNum() - 0.07 * y
				y = y + p:getHandcardNum()
			else
				v = v - 0.44 * x
			end
		end
		p = p:getNextAlive()
	end
	if v > 0 then
		local card_str = "@ShenfengCard=."
		return sgs.Card_Parse(card_str)
	end
	return
end

sgs.ai_skill_use_func.ShenfengCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_choice.shenfeng = function(self, choices)
	local room = self.player:getRoom()
	local p = self.player
	local x = p:getTag("ShenfengX"):toInt()
	local kanako = room:findPlayerBySkillName("shenfeng")
	if p:getEquips():length() >= x and p:getPile("wooden_ox"):length() <= 2 and contains(choices, "SFThrow") then
		return "SFThrow"
	elseif p:getHandcardNum() <= 2 and p:getHandcardNum() <= self:getCardsNum("Peach") + self:getCardsNum("Analeptic") and kanako
			and kanako:isAlive() and self:isFriend(kanako) and contains(choices, "SFGive") then
		return "SFGive"
	elseif x <= p:getMaxHp() / 2 or (x <= 2 and p:getHp() + self:getCardsNum("Peach") + self:getCardsNum("Analeptic") >= x + 3
			and p:getHandcardNum() >= 2) or p:getHp() - x >= getBestHp(p) or (x <= 2
			and p:getHp() + self:getCardsNum("Peach") + self:getCardsNum("Analeptic") > x and p:hasSkills(sgs.masochism_skill)) then
		return "SFDamage"
	elseif not p:isKongcheng() and (self:isFriend(p) or (p:getHp() + self:getCardsNum("Peach") + self:getCardsNum("Analeptic") <= x or x >= 3
			or (p:getHandcardNum() <= 2 and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") == 0)))
			and table.contains(string.split(choices, "+"), "SFGive") then
		return "SFGive"
	else
		return "SFDamage"
	end
end

sgs.ai_use_priority.ShenfengCard = 4.7
sgs.ai_use_value.ShenfengCard = 8.6
sgs.dynamic_value.damage_card.ShenfengCard = true

sgs.ai_chaofeng.daiyousei = 3
sgs.ai_chaofeng.yuyuko = 2
sgs.ai_chaofeng.suimitsu = 2
sgs.ai_chaofeng.prismriver = 5
sgs.ai_chaofeng.wriggle = 4
sgs.ai_chaofeng.lily = 5
sgs.ai_chaofeng.ex_keine = 3
sgs.ai_chaofeng.ex_kanako = 4


