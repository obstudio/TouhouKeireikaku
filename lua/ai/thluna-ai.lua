sgs.ai_skill_use["@@liebo"] = function(self, prompt)
	self:sort(self.friends, "defense")
	local targets = {}
	local n = 1
	for _, p in ipairs(self.friends) do
		if #targets == 0 then
			if (p:objectName() ~= self.player:objectName() or p:hasFlag("LieboUseTo")) and not self:needKongcheng(p) then
				sgs.updateIntention(self.player, p, -80)
				table.insert(targets, p:objectName())
				if p:hasFlag("LieboUseTo") then
					n = 1000
				end
				if #targets >= n then break end
			end
		else
			if p:hasFlag("LieboUseTo") and not table.contains(targets, p:objectName()) and not self:needKongcheng(p) then
				sgs.updateIntention(self.player, p, -80)
				table.insert(targets, p:objectName())
			end
		end
	end
	if #targets > 0 then
		return "@LieboCard=.->" .. table.concat(targets, "+")
	end
	return "."
end

sgs.ai_skill_use["@@duanxiang"] = function(self, prompt)
	local yatsuhashi = self.room:findPlayerBySkillName("duanxiang")
	if not self:isFriend(yatsuhashi) then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if not c:isAvailable(self.player) then
			return "@DuanxiangCard=" .. c:getEffectiveId()
		end
	end
	for _, c in ipairs(cards) do
		return "@DuanxiangCard=" .. c:getEffectiveId()
	end
	return "."
end

sgs.ai_skill_cardask["@duanxiang-exchange"] = function(self, data)
	local use = data:toCardUse()
	local friend = use.from
	local cards = sgs.QList2Table(self.player:getHandcards())
	local card = self.player:getTag("DuanxiangCard"):toCard()
	if self:isWeakerThan(friend) and self:isWeak() then
		self:sortByKeepValue(cards)
		for _, c in ipairs(cards) do
			if not (c:isKindOf("Jink") and self:getCardsNum("Jink") == 1) and not (c:isKindOf("Analeptic") and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") == 1) then
				return c:getEffectiveId()
			end
		end
	elseif self:isWeakerThan(friend, self.player) and self:isWeak(friend) then
		self:sortByKeepValue(cards, true)
		if not card:isKindOf("Jink") then
			self:sortByKeepValue(cards)
			for _, c in ipairs(cards) do
				if not (c:isKindOf("Jink") and (self:isWeak() or self:getCardsNum("Jink") <= 1)) and self:getKeepValue(c) > self:getKeepValue(card) then
					return c:getEffectiveId()
				end
			end
		end
	else
		self:sortByKeepValue(cards)
		for _, c in ipairs(cards) do
			if (self:getKeepValue(c) < self:getKeepValue(card) and self:isWeakerThan(friend))
					or ((self:getKeepValue(c) > self:getKeepValue(card) or self:getJinxianTarget()) and self:isWeakerThan(friend)) then
				return c:getEffectiveId()
			end
		end
	end
	return "."
end

sgs.ai_skill_playerchosen.jinxian = function(self, targets)
	self:sort(self.friends, "defense")
	for _, p in ipairs(self.friends) do
		if self:cannotDraw(p, 2) then continue end
		local num = p:getHandcardNum()
		if p:objectName() == self.player:objectName() then
			local move = self.player:getTag("JinxianData"):toMoveOneTime()
			num = num - move.card_ids:length()
		end
		num = num + 2
		local maxcard = true
		for _, pp in sgs.qlist(self.room:getOtherPlayers(p)) do
			if pp:getHandcardNum() > num then
				maxcard = false
				break
			end
		end
		if not maxcard then return p end
		if not self:isWeak() or (p:getRole() == "lord" and sgs.isLordInDanger()) or (self:isWeakerThan(p, self.player) and self:isWeak(p) and self.player:getHp() > 1) then
			return p
		end
	end
	return nil
end

local yaoshu_skill = {}
yaoshu_skill.name = "yaoshu"
table.insert(sgs.ai_skills, yaoshu_skill)
yaoshu_skill.getTurnUseCard = function(self)
	sgs.ai_use_priority.YaoshuCard = 1.5
	if self.player:hasUsed("YaoshuCard") or self.player:isKongcheng() or self.player:getHandcardNum() >= self.player:getMaxHp() then return end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	local ycard = nil
	for _, c in ipairs(cards) do
		if not (c:isKindOf("Peach") and self:getCardsNum("Peach") == 1) and not (c:isKindOf("Analeptic") and self:getCardsNum("Analeptic") + self:getCardsNum("Peach") == 1
				and self:isVeryWeak()) then
			ycard = c
			break
		end
	end
	if ycard == nil then return end

	local suit = ycard:getSuit()
	local aoename = "savage_assault|archery_attack"
	local aoenames = aoename:split("|")
	local aoe
	local i
	local good, bad = 0, 0

	local yaoshutrick = "savage_assault|archery_attack|necro_music|ex_nihilo|god_salvation|snatch|dismantlement|haze|phoenix_flame|doom_night"
	-- 待解决 necro_music|phoenix_flame
	-- necro_music要顺便连standard_cards-ai.lua中的sgs.ai_skill_cardask.aoe与thsoul-ai.lua中的相关代码一起改
	local yaoshutricks = yaoshutrick:split("|")
	local aoe_available, ge_available, ex_available, snatch_available, dismantlement_available, haze_available = true, true, true, true, true, true
	local phoenix_available, doom_available = true, true
	for i = 1, #yaoshutricks do
		local forbiden = yaoshutricks[i]
		forbid = sgs.Sanguosha:cloneCard(forbiden, suit)
		if self.player:isCardLimited(forbid, sgs.Card_MethodUse, true) or not forbid:isAvailable(self.player) then
			if forbid:isKindOf("AOE") then aoe_available = false end
			if forbid:isKindOf("GlobalEffect") then ge_available = false end
			if forbid:isKindOf("ExNihilo") then ex_available = false end
			if forbid:isKindOf("Snatch") then snatch_available = false end
			if forbid:isKindOf("Dismantlement") then dismantlement_available = false end
			if forbid:isKindOf("Haze") then haze_available = false end
			if forbid:isKindOf("PhoenixFlame") then phoenix_available = false end
			if forbid:isKindOf("DoomNight") then doom_available = false end
		end
	end

	local yaoshubasic = "slash|fire_slash|thunder_slash|analeptic"
	-- 待解决 slash|fire_slash|thunder_slash
	local yaoshubasics = yaoshubasic:split("|")
	local slash_available, fire_available, thunder_available, analeptic_available = true, true, true, true
	for i = 1, #yaoshubasics do
		local forbiden = yaoshubasics[i]
		forbid = sgs.Sanguosha:cloneCard(forbiden, suit)
		if self.player:isCardLimited(forbid, sgs.Card_MethodUse, true) or not forbid:isAvailable(self.player) then
			if forbid:isKindOf("Slash") then slash_available = false end
			if forbid:isKindOf("FireSlash") then fire_available = false end
			if forbid:isKindOf("ThunderSlash") then thunder_available = false end
			if forbid:isKindOf("Analeptic") then analeptic_available = false end
		end
	end

	if self.player:hasUsed("YaoshuCard") then return end
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			good = good + 10 / friend:getHp()
			if friend:isLord() then good = good + 10 / friend:getHp() end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10 / enemy:getHp()
			if enemy:isLord() then
				bad = bad + 10 / enemy:getHp()
			end
		end
	end

	local godsalvation = sgs.Sanguosha:cloneCard("god_salvation", suit, 0)
	
	if aoe_available then
		for i = 1, #aoenames do
			local newyaoshu = aoenames[i]
			aoe = sgs.Sanguosha:cloneCard(newyaoshu)
			if self:getAoeValue(aoe) > 0 then
				local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. ycard:getEffectiveId() .. ":" .. newyaoshu)
				return parsed_card
			end
		end
	end
	if snatch_available or dismantlement_available or haze_available and self.player:getHp() > 1 then
		if self:findSnatchOrDismantlementTarget(sgs.Sanguosha:cloneCard("snatch", suit, 0), sgs.CardUseStruct()) then
			local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. ycard:getEffectiveId() .. ":" .. "snatch")
			return parsed_card
		end
		if self:findSnatchOrDismantlementTarget(sgs.Sanguosha:cloneCard("haze", suit, 0), sgs.CardUseStruct()) then
			local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. ycard:getEffectiveId() .. ":" .. "haze")
			return parsed_card
		end
		if self:findSnatchOrDismantlementTarget(sgs.Sanguosha:cloneCard("dismantlement", suit, 0), sgs.CardUseStruct()) then
			local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. ycard:getEffectiveId() .. ":" .. "dismantlement")
			return parsed_card
		end
	end
	if ge_available and self:willUseGodSalvation(godsalvation) then
		local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. ycard:getEffectiveId() .. ":" .. "god_salvation")
		return parsed_card
	end
	if ex_available and not self:cannotDraw(self.player, 2) and self:getCardsNum("Jink") + self:getCardsNum("Peach") <= 1 and self.player:getHp() > 1 then
		local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. ycard:getEffectiveId() .. ":" .. "ex_nihilo")
		return parsed_card
	end
	if doom_available then
		local doom = sgs.Sanguosha:cloneCard("doom_night", suit, 0)
		if #self:getDoomNightTargets(doom, 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, doom)) > 0 then
			local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. ycard:getEffectiveId() .. ":" .. "doom_night")
			return parsed_card
		end
	end
	if analeptic_available then
		for _, p in ipairs(self.enemies) do
			for _, c in ipairs(cards) do
				if c ~= ycard and c:isKindOf("Slash") and self:shouldUseAnaleptic(p, c) and self.player:getHp() > 1 then
					local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. ycard:getEffectiveId() .. ":" .. "analeptic")
					return parsed_card
				end
			end
		end
	end

	if aoe_available then
		for i = 1, #aoenames do
			local newyaoshu = aoenames[i]
			aoe = sgs.Sanguosha:cloneCard(newyaoshu)
			if self:getAoeValue(aoe) > -5 and caocao and self:isFriend(caocao) and caocao:getHp() > 1 and not self:willSkipPlayPhase(caocao)
				and not self.player:hasSkill("jueqing") and self:aoeIsEffective(aoe, caocao, self.player) then
				local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. table.concat(allcard, "+") .. ":" .. newyaoshu)
				return parsed_card
			end
		end
	end

	if self:getCardsNum("Peach") == 0 and self:getCardsNum("Analeptic") == 0
		and self:getCardsNum("Nullification") == 0 then
		if ge_available and self:willUseGodSalvation(godsalvation) and self.player:isWounded() then
			local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. table.concat(allcard, "+") .. ":" .. "god_salvation")
			return parsed_card
		end
		if ex_available then
			local parsed_card = sgs.Card_Parse("@YaoshuCard=" .. table.concat(allcard, "+") .. ":" .. "ex_nihilo")
			return parsed_card
		end
	end
end

sgs.ai_skill_use_func.YaoshuCard = function(card, use, self)
	local userstring = card:toString()
	userstring = (userstring:split(":"))[3]
	local yaoshucard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	yaoshucard:setSkillName("yaoshu")
	self:useTrickCard(yaoshucard, use)
	use.card = card
	return
end

sgs.ai_use_priority.YaoshuCard = 1.5

sgs.ai_skill_invoke.xiaoyan = function(self, data)
	local target = self.player:getTag("XiaoyanPlayer"):toPlayer()
	local damage = self:touhouDamage(sgs.DamageStruct("xiaoyan", self.player, target, 1, sgs.DamageStruct_Fire), self.player, target)
	if self:isFriend(target) then
		if target:getHp() > getBestHp(target) and target:getHp() > damage.damage then
			return true
		end
		if self:hasSkills(sgs.masochism_skill, target) then
			if (damage.damage < target:getHp() and damage.damage <= 1) or damage.damage == 0 then
				return true
			end
		end
		if not target:isWounded() then return false end
		if damage.damage <= target:getHp() - 2 and damage.damage <= target:getMaxHp() / 2 then
			return true
		end
		if target:isWounded() then
			local v = (target:getHp() - damage.damage) * 1.4
			for _, sk in ipairs(string.split(sgs.cardneed_skill, "|")) do
				if target:hasSkill(sk) then
					v = v + 1.2
				end
			end
			for _, sk in ipairs(string.split(sgs.priority_skill,"|")) do
				if target:hasSkill(sk) then
					v = v + 0.8
				end
			end
			local p = self.room:getCurrent()
			while p:objectName() ~= target:objectName() do
				p = p:getNextAlive()
				if self:isEnemy(p) and p:faceUp() and not self:willSkipPlayPhase(p) then
					v = v - 1.5
				end
				if not self:isFriend(p) and not self:isEnemy(p) and p:faceUp() and not self:willSkipPlayPhase(p) then
					v = v - 0.7
				end
			end
			if v > 0 then return true end
		end
	elseif self:isEnemy(target) then
		if target:getHp() > getBestHp(target) and target:getHp() > damage.damage then
			return false
		end
		if self:hasSkills(sgs.masochism_skill, target) then
			if (damage.damage < target:getHp() and damage.damage <= 1) or damage.damage == 0 then
				return false
			end
		end
		if not target:isWounded() then
			return true
		end
		if target:isWounded() then
			local v = (target:getHp() - damage.damage) * 1.4
			for _, sk in ipairs(string.split(sgs.priority_skill, "|")) do
				if target:hasSkill(sk) then
					v = v + 1.2
				end
			end
			for _, sk in ipairs(string.split(sgs.priority_skill, "|")) do
				if target:hasSkill(sk) then
					v = v + 0.8
				end
			end
			local p = self.room:getCurrent()
			while p:objectName() ~= target:objectName() do
				p = p:getNextAlive()
				if self:isEnemy(p, target) and p:faceUp() and not self:willSkipPlayPhase(p) then
					v = v - 1.5
				end
				if not self:isFriend(p, target) and not self:isEnemy(p, target) and p:faceUp() and not self:willSkipPlayPhase(p) then
					v = v - 0.7
				end
			end
			if v < 0 then return true end
		end
	end
	return false
end

sgs.ai_skill_invoke.xiaoyan_draw = function(self, data)
	local clownpiece = self.room:findPlayerBySkillName("xiaoyan")
	return self:isFriend(clownpiece)
end

function SmartAI:findWizardWeapon()
	local nextAlive = self.player
	repeat
		nextAlive = nextAlive:getNextAlive()
	until nextAlive:faceUp()

	if not self:isFriend(nextAlive) and not self:isEnemy(nextAlive) then return nil end

	local hasLightning, hasIndulgence, hasSupplyShortage, hasOracle
	local tricks = nextAlive:getJudgingArea()
	if not tricks:isEmpty() then
		local trick = tricks:at(tricks:length() - 1)
		if self:hasTrickEffective(trick, nextAlive) then
			if trick:isKindOf("Lightning") then hasLightning = true
			elseif trick:isKindOf("Indulgence") then hasIndulgence = true
			elseif trick:isKindOf("Oracle") then hasOracle = true
			elseif trick:isKindOf("SupplyShortage") then hasSupplyShortage = true
			end
		end
	end

	local suit = ""

	if hasLightning then
		if self:isFriend(nextAlive) then
			for _, p in ipairs(self.enemies) do
				local weapon = p:getWeapon()
				if weapon and not (weapon:getSuitString() == "spade" and weapon:getNumber() >= 2 and weapon:getNumber() <= 9) then
					return weapon
				end
			end
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				local weapon = p:getWeapon()
				if weapon and not (weapon:getSuitString() == "spade" and weapon:getNumber() >= 2 and weapon:getNumber() <= 9) then
					return weapon
				end
			end
		elseif self:isEnemy(nextAlive) then
			for _, p in ipairs(self.enemies) do
				local weapon = p:getWeapon()
				if weapon and (weapon:getSuitString() == "spade" and weapon:getNumber() >= 2 and weapon:getNumber() <= 9) then
					return weapon
				end
			end
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				local weapon = p:getWeapon()
				if weapon and (weapon:getSuitString() == "spade" and weapon:getNumber() >= 2 and weapon:getNumber() <= 9) then
					return weapon
				end
			end
		end
	end
	if hasIndulgence then
		if self:isFriend(nextAlive) then
			for _, p in ipairs(self.enemies) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() == "heart" then
					return weapon
				end
			end
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() == "heart" then
					return weapon
				end
			end
		elseif self:isEnemy(nextAlive) then
			for _, p in ipairs(self.enemies) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() ~= "heart" then
					return weapon
				end
			end
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() ~= "heart" then
					return weapon
				end
			end
		end
	end
	if hasOracle then
		if self:isFriend(nextAlive) then
			for _, p in ipairs(self.enemies) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() ~= "diamond" then
					return weapon
				end
			end
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() ~= "diamond" then
					return weapon
				end
			end
		elseif self:isEnemy(nextAlive) then
			for _, p in ipairs(self.enemies) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() == "diamond" then
					return weapon
				end
			end
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() == "diamond" then
					return weapon
				end
			end
		end
	end
	if hasSupplyShortage then
		if self:isFriend(nextAlive) then
			for _, p in ipairs(self.enemies) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() == "club" then
					return weapon
				end
			end
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() == "club" then
					return weapon
				end
			end
		elseif self:isEnemy(nextAlive) then
			for _, p in ipairs(self.enemies) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() ~= "club" then
					return weapon
				end
			end
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				local weapon = p:getWeapon()
				if weapon and weapon:getSuitString() ~= "club" then
					return weapon
				end
			end
		end
	end
	return nil
end

local wanchu_skill = {name = "wanchu"}
table.insert(sgs.ai_skills, wanchu_skill)
wanchu_skill.getTurnUseCard = function(self)
	local weapon = self:findWizardWeapon()
	local id = -1
	if weapon ~= nil then id = weapon:getEffectiveId() end

	sgs.ai_use_priority.WanchuCard = 8.0
	for _, p in ipairs(self.enemies) do
		if p:getWeapon() and p:getWeapon():getEffectiveId() ~= id and p:getHp() >= self.player:getHp() then
			return sgs.Card_Parse("@WanchuCard=.")
		end
	end

	sgs.ai_use_priority.WanchuCard = 7.7
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf("Armor") and self:needToThrowArmor() and self.room:getCardPlace(c:getEffectiveId()) == sgs.Player_Equip then
			return sgs.Card_Parse("@WanchuCard=" .. c:getEffectiveId())
		end
		if c:isKindOf("Armor") and self.room:getCardPlace(c:getEffectiveId()) ~= sgs.Player_Equip and self.player:getArmor()
				and self:getKeepValue(c) >= self:getKeepValue(self.player:getArmor()) then
			return sgs.Card_Parse("@WanchuCard=" .. self.player:getArmor():getEffectiveId())
		end
		if c:isKindOf("EquipCard") and not (c:isKindOf("Armor") and ((not self:needToThrowArmor() and self.room:getCardPlace(c:getEffectiveId()) == sgs.Player_Equip)
				or (self.room:getCardPlace(c:getEffectiveId()) ~= sgs.Player_Equip and self.player:getArmor()
				and self:getKeepValue(c) > self:getKeepValue(self.player:getArmor())))) and self:cardNeed(c) >= 6 and c:getEffectiveId() ~= id then
			return sgs.Card_Parse("@WanchuCard=" .. c:getEffectiveId())
		end
	end

	sgs.ai_use_priority.WanchuCard = 1.3
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:getWeapon() and p:getWeapon():getEffectiveId() == id and p:getHp() >= self.player:getHp() then
			return sgs.Card_Parse("@WanchuCard=.")
		end
	end
	return nil
end

sgs.ai_skill_use_func.WanchuCard = function(card, use, self)
	local n = card:getSubcards():length()
	local weapon = self:findWizardWeapon()
	local id = -1
	if weapon then id = weapon:getEffectiveId() end

	for _, p in ipairs(self.enemies) do
		if p:getWeapon() and p:getWeapon():getEffectiveId() ~= id and p:getHp() >= self.player:getHp() and n == 0 then
			use.card = card
			if use.to then
				sgs.updateIntention(self.player, p, 80)
				use.to:append(p)
			end
			return
		end
	end

	if n == 1 then
		use.card = card
		return
	end

	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:getWeapon() and p:getWeapon():getEffectiveId() == id and p:getHp() >= self.player:getHp() and n == 0 then
			use.card = card
			if use.to then
				use.to:append(p)
			end
			return
		end
	end
end

sgs.ai_use_value.WanchuCard = 6.1
sgs.dynamic_value.control_card.WanchuCard = true

sgs.ai_skill_playerchosen.wanchu = function(self, targets)
	self:sort(self.friends, "hp")
	for _, p in ipairs(self.friends) do
		if p:isWounded() then
			return p
		end
	end
	return nil
end

local yingdan_skill = {name = "yingdan"}
table.insert(sgs.ai_skills, yingdan_skill)
yingdan_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("YingdanCard") then return nil end
	return sgs.Card_Parse("@YingdanCard=.")
end

sgs.ai_skill_use_func.YingdanCard = function(card, use, self)
	use.card = card
	return
end

sgs.ai_use_priority.YingdanCard = 1.5
sgs.ai_use_value.YingdanCard = 6.3
sgs.dynamic_value.control_card.YingdanCard = true
sgs.dynamic_value.damage_card.YingdanCard = true

sgs.ai_skill_askforag.yingdan = function(self, card_ids)
	local slashes = {}
	for _, id in ipairs(card_ids) do
		table.insert(slashes, sgs.Sanguosha:getCard(id))
	end
	self:sortByUseValue(slashes)
	for _, slash in ipairs(slashes) do
		self:sort(self.enemies, "defense")
		for _, p in ipairs(self.enemies) do
			if self:slashIsEffective(slash, p, self.player) and sgs.isGoodTarget(p, self.enemies, self) and self.player:canSlash(p, slash, false)
					and not self:slashProhibit(slash, p, self.player) and slash:targetFilter(sgs.PlayerList(), p, self.player) then
				self.room:setPlayerFlag(p, "YingdanSlashTo")
				return slash:getEffectiveId()
			end
		end
	end
	return "."
end

sgs.ai_skill_playerchosen.yingdan = function(self, targets)
	for _, p in ipairs(self.enemies) do
		if p:hasFlag("YingdanSlashTo") then
			self.room:setPlayerFlag(p, "-YingdanSlashTo")
			return p
		end
	end
	return nil
end

local nianli_skill = {name = "nianli"}
table.insert(sgs.ai_skills, nianli_skill)
nianli_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("NianliCard") and self.player:hasFlag("RuhuanLess") then return nil end
	if self.player:getMark("nianli") >= self.room:getAlivePlayers():length() - 1 then return nil end

	self:sort(self.friends, "defense")
	self:sort(self.enemies, "defense")
	self:sort(self.friends_noself, "defense")

	if self.player:getHp() == 1 and not self.player:hasFlag("RuhuanLess") then
		local peaches = 0
		for _, p in ipairs(self.friends) do
			peaches = peaches + self:getAllPeachNum(p)
		end
		if peaches == 0 then return nil end
		if peaches == 1 then
			if self:findCardToMoveFrom(self.player, "card"):isKindOf("SilverLion") and self.player:isWounded() then
				return sgs.Card_Parse("@NianliCard=.")
			end
			if self.role ~= "lord" and self.role ~= "renegade" then
				for _, p in ipairs(self.friends) do
					if self:findCardToMoveFrom(p, ".", "ruhuan") then
						return sgs.Card_Parse("@NianliCard=.")
					end
				end
				for _, p in ipairs(self.enemies) do
					if self:findCardToMoveFrom(p, ".", "ruhuan") then
						return sgs.Card_Parse("@NianliCard=.")
					end
				end
				for _, p in ipairs(self.friends_noself) do
					if self:findCardToMoveFrom(p, ".", "ruhuan") then
						return sgs.Card_Parse("@NianliCard=.")
					end
				end
			end
		end
	end

	for _, p in ipairs(self.friends) do
		if self:findCardToMoveFrom(p, ".", "ruhuan") then
			return sgs.Card_Parse("@NianliCard=.")
		end
	end
	for _, p in ipairs(self.enemies) do
		if self:findCardToMoveFrom(p, ".", "ruhuan") then
			return sgs.Card_Parse("@NianliCard=.")
		end
	end
	for _, p in ipairs(self.friends_noself) do
		if self:findCardToMoveFrom(p, ".", "ruhuan") then
			return sgs.Card_Parse("@NianliCard=.")
		end
	end
	return nil
end

sgs.ai_skill_use_func.NianliCard = function(card, use, self)
	self:sort(self.friends, "defense")
	self:sort(self.enemies, "defense")
	self:sort(self.friends_noself, "defense")

	if self.player:getHp() == 1 and not self.player:hasFlag("RuhuanLess") then
		local peaches = 0
		for _, p in ipairs(self.friends) do
			peaches = peaches + self:getAllPeachNum(p)
		end
		if peaches == 0 then return nil end
		if peaches == 1 then
			if self:findCardToMoveFrom(self.player, ".", "ruhuan") and self:findCardToMoveFrom(self.player, "card"):isKindOf("SilverLion") and self.player:isWounded() then
				use.card = card
				if use.to then
					use.to:append(self.player)
					use.to:append(self:findCardToMoveFrom(self.player, "target"))
				end
				return
			end
		end
	end

	for _, p in ipairs(self.friends) do
		if self:findCardToMoveFrom(p, ".", "ruhuan") then
			use.card = card
			if use.to then
				use.to:append(p)
				use.to:append(self:findCardToMoveFrom(p, "target"))
			end
			return
		end
	end
	for _, p in ipairs(self.enemies) do
		if self:findCardToMoveFrom(p, ".", "ruhuan") then
			use.card = card
			if use.to then
				use.to:append(p)
				use.to:append(self:findCardToMoveFrom(p, "target"))
			end
			return
		end
	end
	for _, p in ipairs(self.friends_noself) do
		if self:findCardToMoveFrom(p, ".", "ruhuan") then
			use.card = card
			if use.to then
				use.to:append(p)
				use.to:append(self:findCardToMoveFrom(p, "target"))
			end
			return
		end
	end
end

sgs.ai_use_priority.NianliCard = 5.6
sgs.ai_use_value.NianliCard = 8
sgs.dynamic_value.control_card.NianliCard = true

sgs.ai_skill_askforag.nianli = function(self, card_ids)
	local from = self.player:getTag("NianliMoveFrom"):toPlayer()
	local id = self:findCardToMoveFrom(from, "card"):getEffectiveId()
	if table.contains(card_ids, id) then
		return id
	end
	return card_ids[1]
end

function SmartAI:needRuhuanMore()
	if ((self.player:getHandcardNum() >= 3 and self.player:getHp() >= 3) or self.player:getHandcardNum() >= 5)
			and self:getCardsNum("Peach") <= self.player:getHandcardNum() - 2 then return true end
	if self:isWeak() and self:getCardsNum("Jink") + self:getCardsNum("Peach") + self:getCardsNum("Analeptic") == 0 then return true end
	return false
end

function SmartAI:needRuhuanLess()
	if self:isWeak() and self.player:isWounded() and self:getCardsNum("Jink") + self:getCardsNum("Peach") + self:getCardsNum("Analeptic") > 0 then return true end
	return false
end

sgs.ai_skill_invoke.ruhuan = function(self, data)
	return self:needRuhuanMore() or self:needRuhuanLess()
end

sgs.ai_skill_choice.ruhuan = function(self, choices)
	if self:needRuhuanMore() then return "RuhuanMore" end
	if self:needRuhuanLess() then return "RuhuanLess" end
	return "RuhuanLess"
end

local donghe_skill = {name = "donghe"}
table.insert(sgs.ai_skills, donghe_skill)
donghe_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("DongheCard") then return end
	if #self.enemies == 0 then return end
	self:sort(self.enemies, "defense")
	for i = 1, #self.enemies - 1, 1 do
		for j = i + 1, #self.enemies, 1 do
			if self.player:distanceTo(self.enemies[i]) > self.player:distanceTo(self.enemies[j]) then
				self.enemies[i], self.enemies[j] = self.enemies[j], self.enemies[i]
			end
		end
	end
	local maxv = 0
	local target
	for _, p in ipairs(self.enemies) do
		if self.player:distanceTo(p) > self.player:getHandcardNum() then continue end
		local damage = self:touhouDamage(sgs.DamageStruct("donghe", self.player, p), self.player, p)
		if self:cantbeHurt(p, self.player, 1) then continue end
		local v = math.min(self.player:getHandcardNum() - math.min(self:getCardsNum("Peach"), 1) - math.min(self:getCardsNum("Jink"), 1), 3) / math.max(self.player:distanceTo(p), 1)
				+ 6 / math.max(sgs.getDefense(p), 2) - 1
		if self.player:distanceTo(p) == 1 then
			v = v - 1
		end
		if p:getRole() == "lord" then
			v = v + 1.8 - math.min(p:getHp(), 4) * 0.4
			if sgs.isLordInDanger() then
				v = v + 0.5
			end
		end
		if self.player:distanceTo(p) == self.player:getHp() then
			v = v + 0.6
		end
		if v > maxv then
			maxv = v
			target = p
		end
	end
	if target then
		self.room:setPlayerFlag(target, "DongheFlag")
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		local to_discard = {}
		for i = 1, math.min(self.player:distanceTo(target), #cards), 1 do
			table.insert(to_discard, cards[i]:getEffectiveId())
		end
		if #to_discard > 0 then
			return sgs.Card_Parse("@DongheCard=" .. table.concat(to_discard, "+"))
		end
	end
end

sgs.ai_skill_use_func.DongheCard = function(card, use, self)
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:hasFlag("DongheFlag") then
			use.card = card
			if use.to then
				use.to:append(p)
				self.room:setPlayerFlag(p, "-DongheFlag")
			end
			return
		end
	end
end

sgs.ai_card_intention.DongheCard = 80
sgs.ai_use_priority.DongheCard = 6.9
sgs.ai_use_value.DongheCard = 7.6
sgs.dynamic_value.control_card.DongheCard = true
sgs.dynamic_value.damage_card.DongheCard = true

sgs.ai_skill_discard.donghe = function(self, discard_num, min_num, optional, include_equip)
	local kogasa = self.room:findPlayerBySkillName("donghe")
	local damage = self:touhouDamage(sgs.DamageStruct("donghe", kogasa, self.player), kogasa, self.player)
	if damage.damage == 0 then return {} end

	if self.player:getHp() - damage.damage >= getBestHp(self.player) then return {} end

	if discard_num >= kogasa:getHp() and self:isEnemy(kogasa) and not self:isWeakerThan(kogasa) and self.player:getHp() > damage.damage
			and (self:getDefenseCardsNum() >= 1 or self.player:getHp() >= 4) then
		return {}
	end

	local discards = {}
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)

	if self.player:getHandcardNum() >= discard_num + 2 and discard_num <= 2 then
		for _, c in ipairs(cards) do
			table.insert(discards, c:getEffectiveId())
			if #discards >= discard_num then return discards end
		end
	end

	if discard_num >= 4 then
		local peaches = self.player:getHp()
		for _, p in ipairs(self.friends) do
			peaches = peaches + self:getAllPeachNum(p)
		end
		if peaches > damage.damage then return {} end
	end

	discards = {}
	if discard_num == 3 then
		for _, c in ipairs(cards) do
			if self:cardNeed(c) < 6 then
				table.insert(discards, c:getEffectiveId())
				if #discards >= discard_num then return discards end
			end
		end
	end

	discards = {}
	for _, c in ipairs(cards) do
		if (not (c:isKindOf("Jink") and self:isWeak() and self:getCardsNum("Jink") == 1) and not (c:isKindOf("Peach") and self:getCardsNum("Peach") == 1)
				and not (c:isKindOf("Analeptic") and self:isWeak() and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") == 1))
				or (self:cardNeed(c) < 6) then
			table.insert(discards, c:getEffectiveId())
			if #discards >= discard_num then return discards end
		end
	end

	return {}
end

sgs.ai_skill_use["@@yeyu"] = function(self, prompt)
	self:sort(self.friends, "defense")
	local targets = {}
	for _, p in ipairs(self.friends) do
		table.insert(targets, p:objectName())
		if #targets >= self.player:getMark("yeyu") then
			return "@YeyuCard=.->" .. table.concat(targets, "+")
		end
	end
	if #targets > 0 then
		return "@YeyuCard=.->" .. table.concat(targets, "+")
	end
	return "."
end

sgs.ai_skill_choice.yeyu = function(self, choices)
	local target = self.player:getTag("YeyuDecideOn"):toPlayer()
	if getBestHp(target) <= target:getHp() then
		return "YeyuDraw"
	end
	if (target:getLostHp() <= 1 or (target:getLostHp() <= 2 and not self:isWeak(target))) and (target:getHandcardNum() <= 2
			or (target:getHandcardNum() <= 4 and self:hasSkills(sgs.cardneed_skill, target))) then
		return "YeyuDraw"
	end
	return "YeyuRecover"
end

sgs.ai_skill_use["@@mieli"] = function(self, prompt)
	local targets = {}
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isEnemy(p) and self.player:distanceTo(p) == 1 and not self:cantbeHurt(p) and getBestHp(p) >= p:getHp() then
			table.insert(targets, p:objectName())
		end
	end
	if #targets > 0 then
		return "@MieliCard=.->" .. table.concat(targets, "+")
	end
	return "."
end

local youwang_skill = {name = "youwang"}
table.insert(sgs.ai_skills, youwang_skill)
youwang_skill.getTurnUseCard = function(self)
	if self.player:getMark("@lure") <= 0 or not self.player:isWounded() then return nil end

	local suit = sgs.Card_NoSuit

	local aoename = "savage_assault|archery_attack"
	local aoenames = aoename:split("|")
	local aoe
	local i
	local good, bad, ge = 0, 0, 0

	local youwangtrick = "savage_assault|archery_attack|necro_music|ex_nihilo|god_salvation|snatch|dismantlement|haze|phoenix_flame|doom_night|iron_chain"
	-- 待解决 necro_music|phoenix_flame
	-- necro_music要顺便连standard_cards-ai.lua中的sgs.ai_skill_cardask.aoe与thsoul-ai.lua中的相关代码一起改
	local youwangtricks = youwangtrick:split("|")
	local aoe_available, ge_available, ex_available, snatch_available, dismantlement_available, haze_available = true, true, true, true, true, true
	local phoenix_available, doom_available, iron_available = true, true, true
	for i = 1, #youwangtricks do
		local forbiden = youwangtricks[i]
		forbid = sgs.Sanguosha:cloneCard(forbiden, suit)
		if self.player:isCardLimited(forbid, sgs.Card_MethodUse, true) or not forbid:isAvailable(self.player) then
			if forbid:isKindOf("AOE") then aoe_available = false end
			if forbid:isKindOf("GlobalEffect") then ge_available = false end
			if forbid:isKindOf("ExNihilo") then ex_available = false end
			if forbid:isKindOf("Snatch") then snatch_available = false end
			if forbid:isKindOf("Dismantlement") then dismantlement_available = false end
			if forbid:isKindOf("Haze") then haze_available = false end
			if forbid:isKindOf("PhoenixFlame") then phoenix_available = false end
			if forbid:isKindOf("DoomNight") then doom_available = false end
		end
	end

	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			good = good + 10 / friend:getHp()
			if friend:isLord() then good = good + 10 / friend:getHp() end
		end
		if friend:getHandcardNum() <= self.player:getLostHp() + 1 and (self:isWeak(friend) and not (self.role == "rebel" and friend:getHp() > 1))
				and not (self.role == "renegade" and not friend:isLord()) then
			good = good - 1
			if friend:isLord() then
				good = good - 1
			end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10 / enemy:getHp()
			if enemy:isLord() then
				bad = bad + 10 / enemy:getHp()
			end
		end
		if enemy:isLord() and enemy:getHandcardNum() > self.player:getLostHp() + 1 then
			bad = bad + 1
		end
		if enemy:isLord() and enemy:getHandcardNum() <= self.player:getLostHp() then
			bad = bad - 1
		end
	end

	local godsalvation = sgs.Sanguosha:cloneCard("god_salvation", suit, 0)
	
	if aoe_available then
		for i = 1, #aoenames do
			local newyouwang = aoenames[i]
			aoe = sgs.Sanguosha:cloneCard(newyouwang)
			if self:getAoeValue(aoe) > 0 and good > bad + 1 then
				local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. newyouwang)
				return parsed_card
			end
		end
	end

	if phoenix_available then
		local phoenix_flame = sgs.Sanguosha:cloneCard("phoenix_flame", suit, 0)
		local targets = self:findPhoenixFlameTargets(phoenix_flame, "youwang")
		if #targets > 0 then
			local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. "phoenix_flame")
			return parsed_card
		end
	end

	--[[if doom_available then
		local doom = sgs.Sanguosha:cloneCard("doom_night", suit, 0)
		if #self:getDoomNightTargets(doom, 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, doom), "youwang") > 0 then
			local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. "doom_night")
			return parsed_card
		end
	end

	if snatch_available or dismantlement_available or haze_available and self.player:getHp() > 1 then
		if self:findSnatchOrDismantlementTarget(sgs.Sanguosha:cloneCard("snatch", suit, 0), sgs.CardUseStruct()) then
			local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. "snatch")
			return parsed_card
		end
		if self:findSnatchOrDismantlementTarget(sgs.Sanguosha:cloneCard("haze", suit, 0), sgs.CardUseStruct()) then
			local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. "haze")
			return parsed_card
		end
		if self:findSnatchOrDismantlementTarget(sgs.Sanguosha:cloneCard("dismantlement", suit, 0), sgs.CardUseStruct()) then
			local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. "dismantlement")
			return parsed_card
		end
	end

	if ex_available and not self:cannotDraw(self.player, 2) and self:getCardsNum("Jink") + self:getCardsNum("Peach") <= 1 and self.player:getHp() > 1 then
		local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. "ex_nihilo")
		return parsed_card
	end]]--

	if ge_available and self:willUseGodSalvation(godsalvation) and good < bad - 1 then
		local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. "god_salvation")
		return parsed_card
	end

	if aoe_available then
		for i = 1, #aoenames do
			local newyouwang = aoenames[i]
			aoe = sgs.Sanguosha:cloneCard(newyouwang)
			if self:getAoeValue(aoe) > -5 and caocao and self:isFriend(caocao) and caocao:getHp() > 1 and not self:willSkipPlayPhase(caocao)
				and not self.player:hasSkill("jueqing") and self:aoeIsEffective(aoe, caocao, self.player) and good > bad + 1 then
				local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. newyouwang)
				return parsed_card
			end
		end
	end

	if self:getCardsNum("Peach") == 0 and self:getCardsNum("Analeptic") == 0
		and self:getCardsNum("Nullification") == 0 then
		if ge_available and self:willUseGodSalvation(godsalvation) and self.player:isWounded() and good < bad - 1 then
			local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. "god_salvation")
			return parsed_card
		end
		--[[if ex_available then
			local parsed_card = sgs.Card_Parse("@YouwangCard=.:" .. "ex_nihilo")
			return parsed_card
		end]]--
	end
end

sgs.ai_skill_use_func.YouwangCard = function(card, use, self)
	local userstring = card:toString()
	userstring = (userstring:split(":"))[3]
	local youwangcard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	youwangcard:setSkillName("yaoshu")
	self:useTrickCard(youwangcard, use)
	use.card = card
	return
end

sgs.ai_use_priority.YouwangCard = 1.5

sgs.ai_skill_use["@@zaobi"] = function(self, prompt)
	--if self.player:hasFlag("Zaobi1") then
		if self.player:isKongcheng() then return "." end
		if #self.enemies == 0 then return "." end
		self:sort(self.enemies, "handcard")

		local function compare_func(a, b)
			return a:getNumber() < b:getNumber()
		end

		local cards = sgs.QList2Table(self.player:getHandcards())
		if #cards <= 2 then return "." end
		table.sort(cards, compare_func)
		local biggest = 0
		local card
		for _, c in ipairs(cards) do
			if c:getNumber() > biggest and not (c:isKindOf("Jink") and self:getCardsNum("Jink") == 0) and not (c:isKindOf("Peach") and self:getCardsNum("Peach") < 3)
					and not (c:isKindOf("ExNihilo")) and not (c:isKindOf("Analeptic") and self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < 3 and self:isVeryWeak()) then
				biggest = c:getNumber()
				card = c
			end
		end
		if not card then return "." end
		local targets = {}
		for _, p in ipairs(self.enemies) do
			if p:isKongcheng() then continue end
			local enemybig = 0
			local unknown = 0
			local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), p:objectName())
			for _, c in sgs.qlist(p:getHandcards()) do
				if (c:hasFlag("visible") or c:hasFlag(flag)) then
					if c:getNumber() > enemybig then
						enemybig = c:getNumber()
					end
				else
					unknown = unknown + 1
				end
			end
			if (unknown <= 4 and biggest > math.max(10, enemybig)) or (unknown <= 3 and biggest > math.max(9, enemybig)) or (unknown <= 1 and biggest > math.max(7, enemybig))
					or (unknown == 0 and biggest > enemybig) then
				sgs.updateIntention(self.player, p, 80)
				table.insert(targets, p:objectName())
				if #targets >= 3 then break end
			end
		end
		if #targets > 0 then
			return "@ZaobiCard=" .. card:getEffectiveId() .. "->" .. table.concat(targets, "+")
		end
		--return "@ZaobiCard=" .. cards[1]:getEffectiveId() .. "->" .. self.enemies[1]:objectName()
	--end
	return "."
end

sgs.ai_skill_cardask["@zaobi-extra"] = function(self, data, pattern)
	if self.player:isKongcheng() then return "." end
	local use = data:toCardUse()
	local card = use.card
	if card:isKindOf("ExNihilo") then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if self:getUseValue(c) < 6 and self:getKeepValue(c) < 6 then
			return c:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_chaofeng.yatsuhashi = 3
sgs.ai_chaofeng.kosuzu = 4
sgs.ai_chaofeng.seiran = 2
sgs.ai_chaofeng.sumireko = 3
sgs.ai_chaofeng.ex_yuyuko = 5
sgs.ai_chaofeng.ex_seiga = 5
