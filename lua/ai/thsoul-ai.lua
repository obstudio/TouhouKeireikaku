function SmartAI:useCardSoulSlash(...)
	self:useCardSlash(...)
end

sgs.ai_card_intention.SoulSlash = sgs.ai_card_intention.Slash

sgs.ai_use_value.SoulSlash = 4.6
sgs.ai_keep_value.SoulSlash = 3.63
sgs.ai_use_priority.SoulSlash = 2.5
sgs.dynamic_value.damage_card.SoulSlash = true

sgs.ai_skill_cardask["necro-music-card-ask"] = function(self, data, pattern, target)
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if not c:isKindOf("BasicCard") then
			return c:getEffectiveId()
		end
	end
	return "."
end

sgs.dynamic_value.damage_card.NecroMusic = true
sgs.ai_use_value.NecroMusic = 3.8
sgs.ai_use_priority.NecroMusic = 3.5
sgs.ai_keep_value.NecroMusic = 3.38