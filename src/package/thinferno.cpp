#include "thinferno.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "room.h"
#include "WrappedCard.h"
#include "roomthread.h"

PhoenixFlame::PhoenixFlame(Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("phoenix_flame");
}

QString PhoenixFlame::getSubtype() const
{
    return "double_effect";
}

bool PhoenixFlame::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    int total_num = 2 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    return targets.length() < total_num && to_select != Self && to_select->inMyAttackRange(Self);
}

void PhoenixFlame::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    bool useSlash = false;
	if (effect.to->canSlash(effect.from, NULL, false)) {
		QString prompt = QString("phoenix_flame-slash:%1:%2").arg(effect.from->objectName()).arg(effect.from->objectName());
        useSlash = room->askForUseSlashTo(effect.to, effect.from, prompt);
	}
	if (!useSlash) {
		room->damage(DamageStruct(this, effect.from->isAlive() ? effect.from : NULL, effect.to, 1, DamageStruct::Fire));
	}
}

class AzraelScytheSkill : public WeaponSkill
{
public:
    AzraelScytheSkill() : WeaponSkill("AzraelScythe")
    {
        events << TargetSpecified;
    }
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		if (use.from && use.from->isAlive() && use.card->isKindOf("Slash") && use.from->getWeapon()
				&& use.from->getWeapon()->isKindOf("AzraelScythe")) {
			foreach(ServerPlayer *to, use.to) {
				if (!to->isKongcheng() && to->getHp() < to->getHandcardNum())
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, use.from, use.from, NULL, false, to);
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *invoker = invoke->invoker;
		invoker->tag["AzraelTarget"] = QVariant::fromValue(invoke->preferredTarget);
		return room->askForSkillInvoke(invoker, objectName(), data);
	}

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *from = invoke->invoker;
		ServerPlayer *to = invoke->preferredTarget;
		int card_id = room->askForCardChosen(from, to, "h", objectName());
		const Card *card = Sanguosha->getCard(card_id);
		room->throwCard(card, to, from);
		if (card->isBlack() && use.m_addHistory) {
			room->addPlayerHistory(from, use.card->getClassName(), -1);
			use.m_addHistory = false;
			data = QVariant::fromValue(use);
		}

        return false;
    }
};

AzraelScythe::AzraelScythe(Suit suit, int number)
    : Weapon(suit, number, 3)
{
    setObjectName("AzraelScythe");
}

class YasakaninomagatamaSkill : public TreasureSkill
{
	
public:
	YasakaninomagatamaSkill() : TreasureSkill("Yasakaninomagatama")
	{
		events << CardsMoveOneTime << ConfirmDamage;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardsMoveOneTime) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			if (move.from_places.contains(Player::PlaceEquip)) {
				for (int i = 0; i < move.card_ids.size(); i++) {
					if (move.from_places[i] != Player::PlaceEquip) continue;
					const Card *card = Sanguosha->getEngineCard(move.card_ids[i]);
					if (card->objectName() == objectName()) {
						ServerPlayer *player = qobject_cast<ServerPlayer *>(move.from);
						if (player && player->isAlive())
							return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player, NULL, true);
					}
				}
				return QList<SkillInvokeDetail>();
			}
			if (move.to_place == Player::PlaceEquip) {
				for (int i = 0; i < move.card_ids.size(); i++) {
					const Card *card = Sanguosha->getEngineCard(move.card_ids[i]);
					if (card->objectName() == objectName()) {
						ServerPlayer *player = qobject_cast<ServerPlayer *>(move.to);
						if (player && player->isAlive())
							return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player, NULL, true);
					}
				}
				return QList<SkillInvokeDetail>();
			}
			return QList<SkillInvokeDetail>();
		}
		else if (event == ConfirmDamage) {
			DamageStruct damage = data.value<DamageStruct>();
			if (damage.to && damage.to->isAlive() && damage.to->getTreasure() && damage.to->getTreasure()->objectName() == objectName()
					&& damage.nature != DamageStruct::Normal)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to, NULL, false);
			return QList<SkillInvokeDetail>();
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
        if (event == ConfirmDamage) {
			ServerPlayer *player = invoke->invoker;
			return room->askForSkillInvoke(player, objectName(), data);
        }
		else
			return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == CardsMoveOneTime) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			if (move.from_places.contains(Player::PlaceEquip)) {
				for (int i = 0; i < move.card_ids.size(); i++) {
					if (move.from_places[i] != Player::PlaceEquip) continue;
					const Card *card = Sanguosha->getEngineCard(move.card_ids[i]);
					if (card->objectName() == objectName()) {
						ServerPlayer *player = qobject_cast<ServerPlayer *>(move.from);
						if (player && player->isAlive())
							room->loseMaxHp(player, 1);
					}
				}
			}
			if (move.to_place == Player::PlaceEquip) {
				for (int i = 0; i < move.card_ids.size(); i++) {
					const Card *card = Sanguosha->getEngineCard(move.card_ids[i]);
					if (card->objectName() == objectName()) {
						ServerPlayer *player = qobject_cast<ServerPlayer *>(move.to);
						if (player && player->isAlive()) {
							room->setPlayerProperty(player, "maxhp", player->getMaxHp() + 1);
							RecoverStruct recover;
							recover.recover = 1;
							recover.who = NULL;
							recover.reason = objectName();
							room->recover(player, recover);
						}
					}
				}
			}
		}
		else if (event == ConfirmDamage) {
			ServerPlayer *player = invoke->invoker;
			DamageStruct damage = data.value<DamageStruct>();
			damage.damage--;
			data = QVariant::fromValue(damage);
			player->drawCards(2);
			room->throwCard(player->getTreasure()->getEffectiveId(), player, player);
		}
		return false;
	}
};

Yasakaninomagatama::Yasakaninomagatama(Card::Suit suit, int number)
    : Treasure(suit, number)
{
    setObjectName("Yasakaninomagatama");
}

THInfernoPackage::THInfernoPackage()
    : Package("thinferno", Package::CardPack)
{
    QList<Card *> cards;

    // spade
	cards << new AzraelScythe(Card::Spade, 8);
    
    // club

    // heart
    cards << new PhoenixFlame(Card::Heart, 10);

    // diamond
    cards << new PhoenixFlame(Card::Diamond, 2)
        << new PhoenixFlame(Card::Diamond, 3)
		<< new Yasakaninomagatama(Card::Diamond, 13);

    foreach(Card *card, cards)
        card->setParent(this);
	
	skills << new AzraelScytheSkill << new YasakaninomagatamaSkill;

}

ADD_PACKAGE(THInferno)

