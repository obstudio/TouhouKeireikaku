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
