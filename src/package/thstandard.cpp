#include "thstandard.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "clientplayer.h"
#include "WrappedCard.h"
#include "room.h"
#include "roomthread.h"
#include "json.h"
#include "maneuvering.h"

FengmoCard::FengmoCard()
{
	target_fixed = true;
}

void FengmoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	if (source->isAlive())
	{
		room->drawCards(source, 2, "fengmo");
		room->addPlayerMark(source, "@seal", 1);
	}
}

class Fengmo : public ViewAsSkill
{
public:
	Fengmo() : ViewAsSkill("fengmo")
	{
	}
	
	bool isEnabledAtPlay(const Player *player) const
	{
		return player->getHandcardNum() >= 2;
	}
	
	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		if (selected.length() == 0) return !to_select->isEquipped();
		else if (selected.length() == 1)
		{
			const Card *card = selected.first();
			return !to_select->isEquipped() && to_select->getTypeId() != card->getTypeId();
		}
		else return false;
	}
	
	const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() != 2)
			return NULL;
		
		FengmoCard *card = new FengmoCard();
		card->addSubcards(cards);
		card->setSkillName(objectName());
		return card;
	}
};

class FengmoAdd : public TriggerSkill
{
public:
	FengmoAdd() : TriggerSkill("#fengmoadd")
	{
		events << EventPhaseStart;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &) const
	{
		ServerPlayer *reimu = room->getCurrent();
		if (reimu->hasSkill("fengmo") && reimu->getPhase() == Player::Finish && reimu->getMark("@seal") >= 3) {
			QList<ServerPlayer *> targets;
			foreach (ServerPlayer *target, room->getOtherPlayers(reimu)) {
				if (reimu->canDiscard(target, "h"))
					targets << target;
			}
			if (!targets.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, reimu, reimu, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
		ServerPlayer *reimu = room->getCurrent();
		return room->askForSkillInvoke(reimu, "fengmo", data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &) const
	{
		ServerPlayer *reimu = room->getCurrent();
		
		QList<ServerPlayer *> targets;
		foreach (ServerPlayer *target, room->getOtherPlayers(reimu)) {
			if (reimu->canDiscard(target, "h"))
				targets << target;
		}
		
		//room->broadcastSkillInvoke("fengmo");
		ServerPlayer *target = room->askForPlayerChosen(reimu, targets, "fengmo");
		int id = room->askForCardChosen(reimu, target, "h", "fengmo", false, Card::MethodDiscard);
		room->throwCard(id, target, reimu);
		
		return false;
	}
};

class FengmoClear : public TriggerSkill
{
public:
	FengmoClear() : TriggerSkill("#fengmo-clear")
	{
		events << EventPhaseEnd;
		frequency = Compulsory;
	}
	
    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
	{
		ServerPlayer *player = data.value<ServerPlayer *>();
		if (player->hasSkill(this) && player->getPhase() == Player::Finish && player->getMark("@seal") > 0) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent triggerevent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *reimu = data.value<ServerPlayer *>();
		room->removePlayerMark(reimu, "@seal", reimu->getMark("@seal"));
		
		return false;
	}
};

GuayuCard::GuayuCard()
{
	will_throw = false;
}

bool GuayuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->hasLordSkill("guayu");
}

void GuayuCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	CardMoveReason reason(CardMoveReason::S_REASON_GIVE, effect.from->objectName(), effect.to->objectName(), "guayu$", QString());
	effect.to->obtainCard(this, false);
	room->setPlayerFlag(effect.from, "guayu");
}

class GuayuViewAsSkill : public OneCardViewAsSkill
{
public:
	GuayuViewAsSkill() : OneCardViewAsSkill("guayu$")
	{
		filter_pattern = ".|.|.|hand!";
		response_pattern = "@@guayu";
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		GuayuCard *card = new GuayuCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Guayu : public TriggerSkill
{
public:
	Guayu() : TriggerSkill("guayu$")
	{
		events << EventPhaseEnd;
		view_as_skill = new GuayuViewAsSkill;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *player = room->getCurrent();
		ServerPlayer *reimu = room->findPlayerBySkillName(objectName());
		if (reimu && reimu->isAlive() && reimu->hasLordSkill(objectName()) && player->getPhase() == Player::Draw
				&& !player->isKongcheng() && player != reimu && player->getKingdom() == "hakurei") {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, reimu, player, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        return room->askForUseCard(invoke->invoker, "@@guayu", "@guayu", -1, Card::MethodUse);
    }
	
	bool effect(TriggerEvent triggerevent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
        ServerPlayer *reimu = room->findPlayerBySkillName(objectName());
		room->notifySkillInvoked(reimu, objectName());
		
		return false;
	}
};

class GuayuAdd : public MaxCardsSkill
{
public:
	GuayuAdd() : MaxCardsSkill("#guayu-add")
	{
	}
	
	int getExtra(const Player *target) const
	{
		if (target->hasFlag("guayu")) {
			return -1;
		} else
			return 0;
	}
};

class GuayuClear : public TriggerSkill
{
public:
	GuayuClear() : TriggerSkill("#guayu-clear")
	{
		events << EventPhaseEnd;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &) const
	{
		ServerPlayer *player = room->getCurrent();
		if (player->hasFlag("guayu") && player->getPhase() == Player::Discard)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player, NULL, true);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent triggerevent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
        ServerPlayer *player = room->getCurrent();
		room->setPlayerFlag(player, "-guayu");
		
		return false;
	}
};

class Xingchen : public TriggerSkill
{
public:
	Xingchen() : TriggerSkill("xingchen")
	{
		events << Damage;
	}
	
    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		if (damage.from->hasSkill(objectName()) && damage.card && damage.card->isKindOf("Slash")
				&& damage.to && damage.to->isAlive()) {
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *target, room->getOtherPlayers(damage.to)) {
				if (damage.to->inMyAttackRange(target)) {
					targets << target;
				}
			}
			if (!targets.empty()) {
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, targets, false);
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		return room->askForSkillInvoke(damage.from, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		
		JudgeStruct judge;
		judge.pattern = ".|black";
		judge.good = true;
		judge.reason = objectName();
		judge.who = damage.from;
		
		room->judge(judge);
		
		if (judge.isGood()) {
			QList<ServerPlayer *> targets;
            foreach (ServerPlayer *target, room->getOtherPlayers(damage.to)) {
				if (damage.to->inMyAttackRange(target)) {
					targets << target;
				}
			}
            ServerPlayer *victim = room->askForPlayerChosen(damage.from, targets, objectName(), "@xingchen-invoke");
			Card *slash = new Slash(Card::NoSuit, -1);
			slash->setSkillName(objectName());
            room->useCard(CardUseStruct(slash, damage.from, victim));
		}
		
		return false;
	}
};

class Sheyue : public OneCardViewAsSkill
{
public:
	Sheyue() : OneCardViewAsSkill("sheyue")
	{
		response_or_use = true;
	}
	
	bool isEnabledAtPlay(const Player *player) const
	{
		return Slash::IsAvailable(player);
	}
	
	bool isEnabledAtResponse(const Player *, const QString &pattern) const
	{
		return pattern == "slash";
	}
	
	bool viewFilter(const Card *card) const
	{
		if (card->getSuit() != Card::Spade)
            return false;

        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            Slash *slash = new Slash(Card::SuitToBeDecided, -1);
            slash->addSubcard(card->getEffectiveId());
            slash->deleteLater();
            return slash->isAvailable(Self);
        } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE
        		|| Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        	return true;
        }
        return false;
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		Card *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
		slash->addSubcard(originalCard);
		slash->setSkillName(objectName());
		return slash;
	}
};

class SheyueAdd : public TriggerSkill
{
public:
	SheyueAdd() : TriggerSkill("#sheyueadd")
	{
		events << TargetConfirmed;
		frequency = Frequent;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *marisa = room->findPlayerBySkillName("sheyue");
		if ((use.from == marisa || use.to.contains(marisa)) && use.card && use.card->isKindOf("Slash") && !use.card->isRed()) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, marisa, marisa, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent triggerevent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &) const
	{
		ServerPlayer *marisa = room->findPlayerBySkillName("sheyue");
		marisa->drawCards(1);
		
		return false;
	}
};

ShantouCard::ShantouCard()
{
	
}

bool ShantouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->getHp() > Self->getHp();
}

void ShantouCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	room->damage(DamageStruct("shantou", effect.from, effect.to, 1));
	if (effect.to->getHandcardNum() < effect.from->getHandcardNum())
		effect.to->drawCards(1);
}

class ShantouVS : public ZeroCardViewAsSkill
{
public:
	ShantouVS() : ZeroCardViewAsSkill("shantou")
	{
	}
	
	const Card *viewAs() const
	{
		return new ShantouCard;
	}
	
	bool isEnabledAtPlay(const Player *) const
	{
		return false;
	}
	
	bool isEnabledAtResponse(const Player *, const QString &pattern) const
	{
		return pattern == "@@shantou";
	}
};

class Shantou : public TriggerSkill
{
public:
	Shantou() : TriggerSkill("shantou")
	{
		events << EventPhaseStart;
		view_as_skill = new ShantouVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &) const
	{
		ServerPlayer *suika = room->getCurrent();
		if (suika->hasSkill(this) && suika->getPhase() == Player::Finish) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, suika, suika, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &) const
	{
		ServerPlayer *suika = room->getCurrent();
		return room->askForUseCard(suika, "@@shantou", "@shantou");
	}
	
	bool effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &) const
	{
		return false;
	}
};

DaosheCard::DaosheCard()
{
}

bool DaosheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void DaosheCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	room->askForDiscard(effect.to, "daoshe", 1, 1, false, false, "@daoshe-discard-1:" + effect.from->objectName());
	if (!effect.to->isKongcheng())
		for (int i = 1; i <= 2; i++)
			if (!room->askForCard(effect.to, ".|.|.|hand", "@daoshe-discard-more:" + effect.from->objectName()) || effect.to->isKongcheng())
				break;
	if (effect.to->getHandcardNum() == effect.to->getHp()) {
		QList<int> ids;
		foreach(const Card *c, effect.to->getHandcards()) {
			ids << c->getEffectiveId();
		}
		int id = room->doGongxin(effect.from, effect.to, ids, "daoshe");
		if (id > 0) {
			const Card *card = Sanguosha->getCard(id);
			effect.from->obtainCard(card, false);
		}
	}
	else
		effect.from->drawCards(1);
}

class Daoshe : public ViewAsSkill
{
	
public:
	Daoshe() : ViewAsSkill("daoshe")
	{
		//response_pattern = "@@daoshe";
	}
	
	virtual bool isEnabledAtPlay(const Player *player) const
	{
		return !player->hasUsed("DaosheCard");
	}
	
	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		if (selected.isEmpty())
            return true;
		else if (selected.length() == 1)
			return to_select->getType() == selected.at(0)->getType();
		else
			return false;
	}
	
	/*virtual const Card *viewAs(const Card *originalCard) const
	{
		DaosheCard *card = new DaosheCard;
		card->addSubcard(originalCard);
		return card;
	}*/
	
	virtual const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() != 2)
			return NULL;
		DaosheCard *card = new DaosheCard;
		card->addSubcards(cards);
		return card;
	}
};
/*
class Daoshe : public TriggerSkill
{
	
public:
    Daoshe() : TriggerSkill("daoshe")
	{
		events << EventPhaseStart;
		view_as_skill = new DaosheVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *aya = data.value<ServerPlayer *>();
		if (aya && aya->isAlive() && aya->hasSkill("daoshe") && aya->getPhase() == Player::Finish) {
			QList<ServerPlayer *> targets;
			foreach (ServerPlayer *p, room->getOtherPlayers(aya)) {
				if (!p->isKongcheng())
					targets << p;
			}
            if (!targets.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, aya, aya, targets, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@daoshe", "@daoshe");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};
*/
class Fengmi : public TriggerSkill
{
	
public:
	Fengmi() : TriggerSkill("fengmi")
	{
		events << CardsMoveOneTime << EventPhaseStart;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardsMoveOneTime) {
			ServerPlayer *aya = room->findPlayerBySkillName(objectName());
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			/*if (aya && aya->isAlive() && move.from && ((move.reason.m_reason ^ 3) % 16 == 0 || move.reason.m_reason == 0x17
					|| move.reason.m_reason == 0x27 || move.reason.m_reason == 0x57) && aya->getPhase() == Player::Play) {
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, aya, aya, NULL, true);
			}*/
			if (aya && aya->isAlive() && move.from && ((move.reason.m_reason ^ 3) % 16 == 0) && aya->getPhase() == Player::Play)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, aya, aya, NULL, true);
		}
		else if (event == EventPhaseStart) {
			ServerPlayer *aya = data.value<ServerPlayer *>();
			if (aya && aya->isAlive() && aya->hasSkill(this) && aya->getPhase() == Player::Discard)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, aya, aya, NULL, false);
			if (aya && aya->isAlive() && aya->hasSkill(this) && aya->getPhase() == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, aya, aya, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart && invoke->invoker->getPhase() == Player::Discard) {
            invoke->invoker->tag["FengmiNum"] = QVariant::fromValue(invoke->invoker->getMark("@fengmi"));
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		}
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *aya = invoke->invoker;
		if (event == EventPhaseStart) {
			if (aya->getPhase() == Player::Discard) {
				aya->drawCards(1);
				room->setPlayerFlag(aya, "FengmiAdjustMaxCards");
			}
			else {
				if (aya->hasFlag("FengmiAdjustMaxCards"))
					room->setPlayerFlag(aya, "-FengmiAdjustMaxCards");
				room->setPlayerMark(aya, "@fengmi", 0);
			}
		}
		else {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			int n = 0;
			foreach (int id, move.card_ids) {
				const Card *c = Sanguosha->getCard(id);
				if (c->isRed())
					n++;
			}
			if (n > 0)
				room->addPlayerMark(aya, "@fengmi", n);
		}
		return false;
	}
};

class FengmiMax : public MaxCardsSkill
{
	
public:
    FengmiMax() : MaxCardsSkill("#fengmi-max")
	{
	}
	
	int getFixed(const Player *aya) const
	{
		if (aya->hasFlag("FengmiAdjustMaxCards"))
			return aya->getMark("@fengmi");
		else
			return -1;
	}
};

class Bingpu : public TriggerSkill
{
	
public:
	Bingpu() : TriggerSkill("bingpu")
	{
		events << EventPhaseStart;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &) const
	{
		ServerPlayer *cirno = room->findPlayerBySkillName(objectName());
		if (cirno && cirno->isAlive() && cirno->getPhase() == Player::NotActive && room->getCurrent()->getPhase() == Player::RoundStart
				&& !cirno->isKongcheng() && cirno->inMyAttackRange(room->getCurrent())) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, cirno, cirno, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
		ServerPlayer *cirno = room->findPlayerBySkillName(objectName());
		ServerPlayer *player = room->getCurrent();
        const Card *card = room->askForCard(cirno, ".|.|.|hand", "@bingpu", data, objectName());
		if (!card)
			return false;
		QString choice;
		if (player->isAllNude())
			choice = "ObtainWithSkip";
		else
			choice = room->askForChoice(player, objectName(), "ObtainWithSkip+LetMeSnatch");
		if (choice == "ObtainWithSkip") {
			player->obtainCard(card);
			player->skip(Player::Draw);
		}
		else if (choice == "LetMeSnatch") {
			int gcard = room->askForCardChosen(cirno, player, "hej", objectName());
			room->obtainCard(cirno, gcard, false);
		}
		
		return false;
	}
};

class Shengqiang : public AttackRangeSkill
{
	
public:
	Shengqiang() : AttackRangeSkill("shengqiang")
	{
	}
	
	int getExtra(const Player *player, bool include_weapon) const
	{
        if (player->hasSkill(objectName()) && player->getHandcardNum() <= player->getMaxHp())
			return 2;
		else
			return 0;
	}
};

class ShengqiangMaso : public MasochismSkill
{
	
public:
	ShengqiangMaso() : MasochismSkill("#shengqiang-maso")
	{
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(const Room *room, const DamageStruct &damage) const
	{
		ServerPlayer *remilia = damage.to;
		if (remilia->hasSkill("shengqiang") && damage.from && damage.from->isAlive() && remilia->inMyAttackRange(damage.from)
				&& damage.card && damage.card->isKindOf("Slash"))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, remilia, remilia, NULL, true);
		return QList<SkillInvokeDetail>();
	}
	
	void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail>, const DamageStruct &damage) const
	{
		ServerPlayer *remilia = damage.to;
		room->sendCompulsoryTriggerLog(remilia, "shengqiang");
		QString choice;
		if (damage.from->getEquips().isEmpty())
			choice = "SQDraw";
		else
			choice = room->askForChoice(damage.from, "shengqiang", "SQDraw+SQDiscard");
		if (choice == "SQDraw")
			remilia->drawCards(remilia->getAttackRange());
		else if (choice == "SQDiscard")
			damage.from->throwAllEquips();
	}
};

class Hongye : public TriggerSkill
{
	
public:
	Hongye() : TriggerSkill("hongye")
	{
		events << EventPhaseStart;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &) const
	{
		ServerPlayer *remilia = room->getCurrent();
		if (remilia->hasSkill(this) && remilia->getPhase() == Player::RoundStart) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, remilia, remilia, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
		ServerPlayer *remilia = room->getCurrent();
		return room->askForSkillInvoke(remilia, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
        ServerPlayer *remilia = room->getCurrent();
		int x = room->alivePlayerCount();
		if (x > 5)
			x = 5;
		QList<int> ids = room->getNCards(x, false);
		CardsMoveStruct move(ids, NULL, NULL, Player::DrawPile, Player::PlaceTable,
			CardMoveReason(CardMoveReason::S_REASON_TURNOVER, remilia->objectName(), "hongye", QString()));
		room->moveCardsAtomic(move, true);

		QList<int> card_heart;
		QList<int> card_not_heart;
		for (int i = 0; i < x; i++) {
			if (Sanguosha->getCard(ids[i])->getSuit() == Card::Heart)
				card_heart << ids[i];
			else
				card_not_heart << ids[i];
		}
		if (!card_heart.isEmpty()) {
			foreach (int id, card_heart) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *dest, room->getOtherPlayers(remilia)) {
					if (!dest->isNude())
						targets << dest;
				}
				if (!targets.isEmpty()) {
					ServerPlayer *target = room->askForPlayerChosen(remilia, targets, objectName());
					int tcard = room->askForCardChosen(remilia, target, "he", objectName());
					room->throwCard(tcard, target, remilia);
				}
			}
		}
		DummyCard *dummy = new DummyCard(ids);
		CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, remilia->objectName(), "hongye", QString());
		room->throwCard(dummy, reason, NULL);
		delete dummy;
		room->broadcastSkillInvoke(objectName());
		
		return false;
	}
};

class Shengxue : public TriggerSkill
{
	
public:
	Shengxue() : TriggerSkill("shengxue")
	{
		events << Damage;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		if (damage.from->hasSkill(this) && damage.card && damage.card->isKindOf("Slash") && damage.card->isRed() && damage.from->isWounded()) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *target = damage.to;
		ServerPlayer *flandre = damage.from;
		room->sendCompulsoryTriggerLog(flandre, objectName());
		RecoverStruct recover;
		recover.recover = 1;
		recover.reason = objectName();
		room->recover(flandre, recover, false);
		
		return false;
	}
};

class Wosui : public TriggerSkill
{
	
public:
	Wosui() : TriggerSkill("wosui")
	{
		events << CardUsed << CardResponded;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		const Card *card;
		ServerPlayer *flandre;
		if (event == CardUsed) {
			CardUseStruct use = data.value<CardUseStruct>();
			card = use.card;
			flandre = use.from;
		}
		else if (event == CardResponded) {
			CardResponseStruct response = data.value<CardResponseStruct>();
			card = response.m_card;
			flandre = response.m_from;
		}
		ServerPlayer *current = room->getCurrent();
		if (flandre->hasSkill(this) && card->isKindOf("BasicCard") && flandre->getPhase() == Player::NotActive
				&& current->getHp() >= flandre->getHp() && !current->isNude()) {
			QList<ServerPlayer *> targets;
			targets << current;
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, flandre, flandre, targets, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
		ServerPlayer *flandre;
		if (event == CardUsed)
			flandre = data.value<CardUseStruct>().from;
		else if (event == CardResponded)
			flandre = data.value<CardResponseStruct>().m_from;
		return room->askForSkillInvoke(flandre, objectName(), data);
	}
	
	bool effect(TriggerEvent triggerevent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
		ServerPlayer *flandre;
		if (triggerevent == CardUsed)
			flandre = data.value<CardUseStruct>().from;
		else if (triggerevent == CardResponded)
			flandre = data.value<CardResponseStruct>().m_from;
		ServerPlayer *current = room->getCurrent();
		int id1 = room->askForCardChosen(flandre, current, "he", objectName());
		room->throwCard(id1, current, flandre);
		if (!current->isNude()) {
			int id2 = room->askForCardChosen(flandre, current, "he", objectName());
			room->throwCard(id2, current, flandre);
		}
		
		return false;
	}
};

HeiyanCard::HeiyanCard()
{
}

bool HeiyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < getSubcards().length() && to_select != Self;
}

void HeiyanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	if (targets.length() == 0)
		return;
	
	room->doSuperLightbox("flandre:heiyan", "heiyan");
	room->removePlayerMark(source, "@darkflame");
	foreach (ServerPlayer *target, targets) {
		if (!target->isNude()) {
			int id = room->askForCardChosen(source, target, "he", "heiyan");
			const Card *c = Sanguosha->getCard(id);
            source->obtainCard(c, (room->getCardPlace(id) != Player::PlaceHand));
		}
		room->damage(DamageStruct("heiyan", source, target, 1, DamageStruct::Fire));
	}
}

class Heiyan : public ViewAsSkill
{
public:
    Heiyan() : ViewAsSkill("heiyan")
    {
        frequency = Limited;
        limit_mark = "@darkflame";
    }
	
	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return selected.length() < 3 && to_select->isBlack() && !to_select->isEquipped();
	}

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() == 0)
			return NULL;
		
		HeiyanCard *card = new HeiyanCard();
		card->addSubcards(cards);
		card->setSkillName("heiyan");
		return card;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("@darkflame") >= 1;
    }
};

class Huiyin : public TriggerSkill
{
	
public:
	Huiyin() : TriggerSkill("huiyin")
	{
		events << CardsMoveOneTime;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *kyouko = room->findPlayerBySkillName(objectName());
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		if (kyouko && kyouko->isAlive() && !kyouko->isNude() && move.from && move.from->isAlive()
				&& (move.reason.m_reason ^ 3) % 16 == 0 && move.from != kyouko && (move.from_places.contains(Player::PlaceHand)
				|| move.from_places.contains(Player::PlaceEquip))) {
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kyouko, kyouko, NULL, false, qobject_cast<ServerPlayer *>(move.from));
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *kyouko = invoke->invoker;
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		QStringList suit_list;
		foreach(int id, move.card_ids) {
			const Card *c = Sanguosha->getCard(id);
			if (!suit_list.contains(c->getSuitString()))
				suit_list << c->getSuitString();
		}
		QString suits = suit_list.join(",");
		return room->askForCard(kyouko, ".|" + suits, "@huiyin-discard", data);
	}
	
    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *target = invoke->preferredTarget;
		target->drawCards(1);
		
		return false;
	}
};

class Kuopin : public TriggerSkill
{
	
public:
	Kuopin() : TriggerSkill("kuopin")
	{
		events << DrawNCards << EventPhaseEnd;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		ServerPlayer *kyouko = room->getCurrent();
		if (kyouko->hasSkill(this)) {
			if (event == DrawNCards) {
				foreach (ServerPlayer *player, room->getOtherPlayers(kyouko)) {
					if (player->getHp() < kyouko->getHp())
						return QList<SkillInvokeDetail>();
				}
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kyouko, kyouko, NULL, true);
			}
			else if (event == EventPhaseEnd) {
				if (kyouko->getPhase() == Player::Finish && kyouko->hasFlag("kuopin"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kyouko, kyouko, NULL, true);
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent triggerevent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
	{
		ServerPlayer *kyouko = room->getCurrent();
		if (triggerevent == DrawNCards) {
			DrawNCardsStruct qnum = data.value<DrawNCardsStruct>();
			qnum.n++;
			data = QVariant::fromValue(qnum);
			room->setPlayerFlag(kyouko, "kuopin");
		}
        else if (triggerevent == EventPhaseEnd) {
			room->setPlayerFlag(kyouko, "-kuopin");
		}
		
		return false;
	}
};

class KuopinMaxCardsSkill : public MaxCardsSkill
{
	
public:
    KuopinMaxCardsSkill() : MaxCardsSkill("#kuopin-maxcardsskill")
	{
	}
	
	int getExtra(const Player *target) const
	{
		if (target->hasFlag("kuopin"))
			return 2;
		else
			return 0;
	}
};

class Shuimu : public TriggerSkill
{
	
public:
	Shuimu() : TriggerSkill("shuimu")
	{
		events << ConfirmDamage << Damaged;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *nitori = damage.to;
		if (nitori && nitori->isAlive() && nitori->hasSkill(this)) {
			if (event == ConfirmDamage) {
				if (damage.nature == DamageStruct::Thunder) {
					QList<ServerPlayer *> targets;
					foreach(ServerPlayer *p, room->getOtherPlayers(nitori)) {
						if (!p->isNude())
							targets << p;
					}
					if (!targets.isEmpty())
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nitori, nitori, targets, false);
				}
				else if (damage.nature == DamageStruct::Fire) {
					if (!nitori->isKongcheng() && damage.damage > 0)
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nitori, nitori, NULL, false);
				}
			}
			else if (event == Damaged) {
				if (damage.nature == DamageStruct::Thunder && nitori->hasFlag("CauseThunder")) {
					QList<ServerPlayer *> targets;
					foreach(ServerPlayer *p, room->getOtherPlayers(nitori)) {
						if (!p->isNude())
							targets << p;
					}
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nitori, nitori, targets, true);
				}
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *nitori = invoke->invoker;
		if (event == ConfirmDamage) {
			if (damage.nature == DamageStruct::Thunder)
				return room->askForSkillInvoke(nitori, objectName(), data);
			else if (damage.nature == DamageStruct::Fire)
				return room->askForCard(nitori, ".|.|.|hand", "@shuimu-discard", data, objectName());
		}
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *nitori = invoke->invoker;
		if (event == ConfirmDamage) {
			if (damage.nature == DamageStruct::Thunder) {
				room->setPlayerFlag(nitori, "CauseThunder");
				damage.damage++;
				data = QVariant::fromValue(damage);
			}
			else if (damage.nature == DamageStruct::Fire) {
				damage.damage--;
				data = QVariant::fromValue(damage);
			}
		}
		else if (event == Damaged) {
			foreach (ServerPlayer *p, room->getOtherPlayers(nitori)) {
				if (!p->isNude()) {
					int id = room->askForCardChosen(nitori, p, "he", objectName());
					const Card *card = Sanguosha->getCard(id);
					room->obtainCard(nitori, card, (room->getCardPlace(id) != Player::PlaceHand));
				}
			}
		}
		
		return false;
	}
};

class Yaoshi : public MasochismSkill
{
	
public:
	Yaoshi() : MasochismSkill("yaoshi")
	{
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(const Room *, const DamageStruct &damage) const
    {
        if (damage.to->isAlive() && damage.to->hasSkill(this) && damage.nature != DamageStruct::Normal) {
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to, NULL, true);
        }
        return QList<SkillInvokeDetail>();
    }

	void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
	{
		ServerPlayer *target = damage.to;
		room->sendCompulsoryTriggerLog(target, objectName());
        //room->sendCompulsoryTriggerLog(target, objectName());
		target->drawCards(2 * damage.damage);
	}
};

HuangyanCard::HuangyanCard()
{
}

bool HuangyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() == 0 && to_select != Self && !to_select->isKongcheng();
}

void HuangyanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	bool success = source->pindian(targets.first(), "huangyan", NULL);
	if (success) {
		room->damage(DamageStruct(objectName(), source, targets.first(), 1, DamageStruct::Fire));
	} else {
		FireSlash *fslash = new FireSlash(Card::NoSuit, 0);
		fslash->setSkillName("huangyan");
		room->useCard(CardUseStruct(fslash, targets.first(), source));
	}
}

class Huangyan : public ZeroCardViewAsSkill
{
	
public:
	Huangyan() : ZeroCardViewAsSkill("huangyan")
	{
	}
	
	bool isEnabledAtPlay(const Player *player) const
	{
		return player->canDiscard(player, "h") && !player->hasUsed("HuangyanCard");
	}
	
	bool isEnabledAtResponse(const Player *, const QString &) const
	{
		return false;
	}
	
	const Card *viewAs() const
	{
		return new HuangyanCard;
	}
};

class Jingyue : public OneCardViewAsSkill
{
	
public:
	Jingyue() : OneCardViewAsSkill("jingyue")
	{
	}
	
	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return selected.length() == 0 && !to_select->isKindOf("BasicCard") && !to_select->isEquipped();
	}
	
	bool isEnabledAtPlay(const Player *) const
	{
		return false;
	}
	
	bool isEnabledAtResponse(const Player *, const QString &pattern) const
	{
		return pattern == "jink";
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		Jink *jink = new Jink(originalCard->getSuit(), originalCard->getNumber());
		jink->setSkillName(objectName());
		jink->addSubcard(originalCard);
		return jink;
	}
};

DiaoouCard::DiaoouCard()
{
}

bool DiaoouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() == 0 && to_select != Self;
}

void DiaoouCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	room->addPlayerMark(effect.to, "@ningyou");
}

class DiaoouViewAsSkill : public ZeroCardViewAsSkill
{
	
public:
	DiaoouViewAsSkill() : ZeroCardViewAsSkill("diaoou")
	{
	}
	
	bool isEnabledAtPlay(const Player *) const
	{
		return false;
	}
	
	bool isEnabledAtResponse(const Player *, const QString &pattern) const
	{
		return pattern == "@@diaoou";
	}
	
	const Card *viewAs() const
	{
		return new DiaoouCard;
	}
};

class Diaoou : public TriggerSkill
{
	
public:
	Diaoou() : TriggerSkill("diaoou")
	{
		events << EventPhaseStart << Damaged;
		view_as_skill = new DiaoouViewAsSkill;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		ServerPlayer *alice = room->findPlayerBySkillName(objectName());
		if (alice && alice->isAlive() && alice->hasSkill(this)) {
			if (event == EventPhaseStart) {
				if (alice->getPhase() == Player::Finish) 
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, alice, alice, NULL, false);
				else if (alice->getPhase() == Player::RoundStart)
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, alice, alice, NULL, true);
			}
			else if (event == Damaged) {
				DamageStruct damage = data.value<DamageStruct>();
				if (alice == damage.to && !alice->isKongcheng()) {
					foreach(ServerPlayer *p, room->getOtherPlayers(alice)) {
						if (p->getMark("@ningyou") > 0)
							return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, alice, alice, NULL, false, p);
					}
				}
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *alice = invoke->invoker;
		if (event == EventPhaseStart && alice->getPhase() == Player::Finish)
			return room->askForUseCard(alice, "@@diaoou", "diaoou-choose-target");
		else if (event == Damaged)
			return room->askForCard(alice, ".", "@diaoou-ask", data, objectName());
		return true;
	}
	
	bool effect(TriggerEvent triggerevent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *alice = invoke->invoker;
		if (triggerevent == EventPhaseStart) {
			if (alice->getPhase() == Player::RoundStart) {
				foreach (ServerPlayer *p, room->getOtherPlayers(alice)) {
					if (p->getMark("@ningyou") > 0) {
						room->removePlayerMark(p, "@ningyou", p->getMark("@ningyou"));
					}
				}
			}
		}
		else if (triggerevent == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
            foreach (ServerPlayer *p, room->getOtherPlayers(alice)) {
				if (p->getMark("@ningyou") > 0) {
					room->damage(DamageStruct(objectName(), damage.from, p));
					break;
				}
			}
		}
		
		return false;
	}
};

AnjiCard::AnjiCard()
{
	will_throw = false;
}

bool AnjiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() == 0 && to_select != Self;
}

void AnjiCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	CardMoveReason reason(CardMoveReason::S_REASON_GIVE, effect.from->objectName(), effect.to->objectName(), "anji", QString());
	room->obtainCard(effect.to, this, reason);
	QList<ServerPlayer *> targets;
	foreach (ServerPlayer *p, room->getOtherPlayers(effect.to)) {
		if (effect.to->canSlash(p)) {
			targets << p;
		}
	}
	QString choice;
    if (targets.isEmpty())
		choice = "KeepCard";
	else
		choice = room->askForChoice(effect.to, objectName(), "KeepCard+UseSlash");
	if (choice == "UseSlash") {
        ServerPlayer *dest = room->askForPlayerChosen(effect.to, targets, "anji", "dummy-slash2:" + effect.to->objectName());
        Slash *slash = new Slash(this->getSuit(), this->getNumber());
		slash->setSkillName("anji");
		slash->addSubcard(getSubcards().first());
		room->useCard(CardUseStruct(slash, effect.to, dest));
	}
	else
		effect.to->drawCards(1);
}

class Anji : public OneCardViewAsSkill
{
	
public:
	Anji() : OneCardViewAsSkill("anji")
	{
		filter_pattern = ".|black|.|hand";
	}
	
	bool isEnabledAtPlay(const Player *player) const
	{
		return !player->hasUsed("AnjiCard");
	}
	
	bool isEnabledAtResponse(const Player *, const QString &) const
	{
		return false;
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		AnjiCard *card = new AnjiCard;
		card->addSubcard(originalCard);
		card->setSkillName("anji");
		return card;
	}
};

class Xuebeng : public MasochismSkill
{
	
public:
	Xuebeng() : MasochismSkill("xuebeng")
	{
		frequency = Frequent;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.to->isAlive() && damage.to->hasSkill(this)) {
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to, NULL, false, NULL);
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        return (invoke->invoker->getPile("snow").length() < 4);
    }
	
	void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
	{
		ServerPlayer *letty = damage.to;
		int x = damage.damage;
		for (int i = 0; i < x; i++) {
			letty->drawCards(1);
			int id = room->getNCards(1).first();
			QList <int> ids;
			ids << id;
			QList <ServerPlayer *> seeable;
			seeable << letty;
			letty->addToPile("snow", ids, false, CardMoveReason(), seeable);
		}
	}
};

class XuebengAdd : public TriggerSkill
{
	
public:
	XuebengAdd() : TriggerSkill("#xuebeng-add")
	{
		events << EventPhaseStart;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *letty = room->findPlayerBySkillName("xuebeng");
		if (letty && letty->isAlive() && letty->getPhase() == Player::Finish && letty->getPile("snow").length() > 0)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, letty, letty, NULL, true);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *letty = invoke->invoker;
		QList<int> snow = letty->getPile("snow");
		room->fillAG(snow, letty);
		int id = room->askForAG(letty, snow, false, "xuebeng");
		room->obtainCard(letty, id, false);
		room->clearAG(letty);
		return false;
	}
};

YaofengCard::YaofengCard()
{
}

bool YaofengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() == 0 && to_select != Self;
}

void YaofengCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
    if (!room->askForCard(effect.to, "BasicCard", "@yaofeng-discard", QVariant()))
		room->damage(DamageStruct("yaofeng", effect.from, effect.to));
}

class YaofengViewAsSkill : public OneCardViewAsSkill
{
	
public:
	YaofengViewAsSkill() : OneCardViewAsSkill("yaofeng")
	{
		expand_pile = "snow";
	}
	
	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        return selected.isEmpty() && Self->getPile("snow").contains(to_select->getEffectiveId());
    }
	
	virtual bool isEnabledAtPlay(const Player *) const
	{
		return false;
	}
	
	virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const
	{
		return pattern == "@@yaofeng";
	}
	
	virtual const Card *viewAs(const Card *originalCard) const
	{
		YaofengCard *card = new YaofengCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Yaofeng : public TriggerSkill
{
	
public:
	Yaofeng() : TriggerSkill("yaofeng")
	{
		events << EventPhaseStart;
		view_as_skill = new YaofengViewAsSkill;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &) const
	{
		ServerPlayer *letty = room->getCurrent();
		if (letty->hasSkill(this) && letty->getPhase() == Player::RoundStart) {
			QList<int> snow = letty->getPile("snow");
			if (!snow.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, letty, letty, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *letty = invoke->invoker;
		return room->askForUseCard(letty, "@@yaofeng", "@yaofeng");
	}
	
	bool trigger(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &) const
	{
		return false;
	}
};

class Jihan : public TriggerSkill
{
	
public:
	Jihan() : TriggerSkill("jihan")
	{
		events << DrawNCards;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &) const
	{
		ServerPlayer *letty = room->getCurrent();
		if (letty->hasSkill(this)) {
			QList<int> snow = letty->getPile("snow");
			if (!snow.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, letty, letty, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *letty = invoke->invoker;
		return room->askForSkillInvoke(letty, objectName(), data);
	}
	
	bool effect(TriggerEvent triggerevent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *letty = invoke->invoker;
		QList<int> snow = letty->getPile("snow");
		DrawNCardsStruct qnum = data.value<DrawNCardsStruct>();
		if (qnum.n > 0) {
			qnum.n--;
			data = QVariant::fromValue(qnum);
		}
		for (int i = 0; i < snow.length(); i++) {
			int id = snow.at(i);
			room->obtainCard(letty, id, false);
		}
		letty->clearOnePrivatePile("snow");
		
		return false;
	}
};

class Xinxing : public TriggerSkill
{
	
public:
	Xinxing() : TriggerSkill("xinxing")
	{
		frequency = Frequent;
		events << CardsMoveOneTime;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *sanae = qobject_cast<ServerPlayer *>(move.from);
		if (sanae && sanae->isAlive() && sanae->hasSkill(this) && move.from_places.contains(Player::PlaceHand)
				&& sanae->getPhase() == Player::NotActive) {
            foreach(ServerPlayer *p, room->getOtherPlayers(sanae)) {
				if (p->getHandcardNum() < sanae->getHandcardNum())
					return QList<SkillInvokeDetail>();
			}
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sanae, sanae, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
        ServerPlayer *sanae = qobject_cast<ServerPlayer *>(data.value<CardsMoveOneTimeStruct>().from);
		sanae->drawCards(1);
		return false;
	}
};

class Qiji : public TriggerSkill
{
	
public:
	Qiji() : TriggerSkill("qiji$")
	{
		events << Damage;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *sanae = room->findPlayerBySkillName(objectName());
		if (sanae && sanae->hasLordSkill(this) && damage.from && damage.from->isAlive() && damage.from->getKingdom() == "moriya"
				&& damage.from != sanae) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sanae, damage.from, NULL, false, sanae);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->invoker;
		return room->askForSkillInvoke(player, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *sanae = invoke->owner;
		ServerPlayer *player = invoke->invoker;
		room->notifySkillInvoked(sanae, objectName());
		QList<ServerPlayer *> to;
		to << sanae;
		room->touhouLogmessage("#InvokeOthersSkill", player, objectName(), to);
		JudgeStruct judge;
		judge.pattern = ".|heart";
		judge.who = sanae;
		judge.reason = objectName();
		judge.good = true;
		room->judge(judge);
		if (judge.isGood()) {
            int n = room->getLieges("moriya", sanae).length() + 1;
			if (n > 3)
				n = 3;
			sanae->drawCards(n);
		}
		return false;
	}
};

YuzhuCard::YuzhuCard()
{
}

bool YuzhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self && !to_select->isNude();
}

void YuzhuCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	int n = 0;
	while (n < 2 && !effect.to->isNude()) {
		n++;
		int id = room->askForCardChosen(effect.from, effect.to, "he", "yuzhu");
		room->throwCard(id, effect.to, effect.from);
		if (Sanguosha->getCard(id)->getSuit() == Card::Club)
			effect.from->drawCards(1);
	}
	effect.from->turnOver();
}

class Yuzhu : public ZeroCardViewAsSkill
{
	
public:
	Yuzhu() : ZeroCardViewAsSkill("yuzhu")
	{
		
	}
	
	virtual bool isEnabledAtPlay(const Player *player) const
	{
		return !player->hasUsed("YuzhuCard");
	}
	
	virtual const Card *viewAs() const
	{
		return new YuzhuCard;
	}
};

class Jinlun : public TriggerSkill
{
	
public:
	Jinlun() : TriggerSkill("jinlun")
	{
		frequency = Compulsory;
		events << ConfirmDamage << BeforeCardsMove;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == ConfirmDamage) {
			DamageStruct damage = data.value<DamageStruct>();
			if (damage.to && damage.to->hasSkill(this) && !damage.to->getArmor() && damage.damage > 1)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to, NULL, true);
			return QList<SkillInvokeDetail>();
		}
		else if (event == BeforeCardsMove) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			ServerPlayer *suwako = qobject_cast<ServerPlayer *>(move.to);
			if (suwako && suwako->isAlive() && suwako->hasSkill(this) && move.to_place == Player::PlaceEquip && suwako->isWounded()
					&& !suwako->getArmor()) {
				foreach(int id, move.card_ids) {
					if (Sanguosha->getCard(id)->isKindOf("Armor"))
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, suwako, suwako, NULL, true);
				}
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *suwako = invoke->invoker;
		room->sendCompulsoryTriggerLog(suwako, objectName());
		if (event == ConfirmDamage) {
			DamageStruct damage = data.value<DamageStruct>();
			damage.damage = 1;
			data = QVariant::fromValue(damage);
		}
		else if (event == BeforeCardsMove) {
			RecoverStruct recover;
			recover.recover = 1;
			recover.reason = objectName();
			room->recover(suwako, recover);
		}
		return false;
	}
};

ChiwaCard::ChiwaCard()
{
	
}

bool ChiwaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->hasFlag("ValidChiwaTarget");
}

void ChiwaCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *from = effect.from;
	ServerPlayer *to = effect.to;
	Room *room = to->getRoom();
	from->tag["ChiwaTarget"] = QVariant::fromValue(to);
}

class ChiwaVS : public OneCardViewAsSkill
{
	
public:
	ChiwaVS() : OneCardViewAsSkill("chiwa")
	{
		filter_pattern = ".";
		response_pattern = "@@chiwa";
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		ChiwaCard *card = new ChiwaCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Chiwa : public TriggerSkill
{
	
public:
	Chiwa() : TriggerSkill("chiwa")
	{
		events << TargetConfirming;
		view_as_skill = new ChiwaVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		if (use.from && use.from->isAlive() && use.from->hasSkill(this) && !use.from->isNude() && use.card) {
			QList<ServerPlayer *> targets;
			if (use.card->isKindOf("Peach")) {
				foreach(ServerPlayer *p, room->getAlivePlayers()) {
					if (!use.to.contains(p) && p->isWounded())
						targets <<p;
				}
			}
			else if (use.card->isKindOf("ExNihilo")) {
				foreach(ServerPlayer *p, room->getAlivePlayers()) {
					if (!use.to.contains(p))
						targets << p;
				}
			}
			else if (use.card->isKindOf("ArcheryAttack") || use.card->isKindOf("AmazingGrace") || use.card->isKindOf("GodSalvation")) {
				foreach(ServerPlayer *p, room->getAlivePlayers()) {
					if (use.to.contains(p))
						targets << p;
				}
			}
			if (!targets.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, use.from, use.from, targets, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *suwako = invoke->invoker;
		suwako->tag["ChiwaUse"] = data;
		CardUseStruct use = data.value<CardUseStruct>();
		if (use.card->isKindOf("Peach")) {
			foreach (ServerPlayer *p, room->getAlivePlayers())
				if (!use.to.contains(p) && p->isWounded())
					room->setPlayerFlag(p, "ValidChiwaTarget");
		} else if (use.card->isKindOf("ExNihilo")) {
			foreach (ServerPlayer *p, room->getAlivePlayers())
				if (!use.to.contains(p))
					room->setPlayerFlag(p, "ValidChiwaTarget");
		} else {
			foreach (ServerPlayer *p, use.to)
				room->setPlayerFlag(p, "ValidChiwaTarget");
		}
		bool yes = room->askForUseCard(suwako, "@@chiwa", "@chiwa");
		foreach (ServerPlayer *p, room->getAlivePlayers())
			if (p->hasFlag("ValidChiwaTarget"))
				room->setPlayerFlag(p, "-ValidChiwaTarget");
		return yes;
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *suwako = invoke->invoker;
		ServerPlayer *target = suwako->tag["ChiwaTarget"].value<ServerPlayer *>();
		CardUseStruct use = data.value<CardUseStruct>();
		QList<ServerPlayer *> to;
		to << target;
		if (use.card->isKindOf("Peach") || use.card->isKindOf("ExNihilo")) {
			room->touhouLogmessage("#AddTarget", suwako, objectName(), to, use.card->objectName());
			use.to.append(target);
		} else if (use.card->isKindOf("ArcheryAttack") || use.card->isKindOf("AmazingGrace") || use.card->isKindOf("GodSalvation")) {
			room->touhouLogmessage("#RemoveTarget", suwako, objectName(), to, use.card->objectName());
			use.to.removeOne(target);
		}
		room->sortByActionOrder(use.to);
		data = QVariant::fromValue(use);
		return false;
	}
};

FangyingCard::FangyingCard()
{
	target_fixed = true;
	will_throw = false;
	handling_method = Card::MethodNone;
}

void FangyingCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    source->addToPile("imitation", this, true);
}

class FangyingVS : public OneCardViewAsSkill
{
	
public:
	FangyingVS() : OneCardViewAsSkill("fangying")
	{
		filter_pattern = ".|.|.|hand";
		response_pattern = "@@fangying";
	}
	
	virtual const Card *viewAs(const Card *originalCard) const
	{
		if (originalCard) {
			FangyingCard *card = new FangyingCard;
			card->addSubcard(originalCard);
			return card;
		}
		return NULL;
	}
};

class Fangying : public TriggerSkill
{
	
public:
	Fangying() : TriggerSkill("fangying")
	{
		events << CardFinished;
		view_as_skill = new FangyingVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *satori = room->findPlayerBySkillName(objectName());
		CardUseStruct use = data.value<CardUseStruct>();
		if (satori && satori->isAlive() && satori->hasSkill(this) && use.card && use.card->isNDTrick() && !satori->isKongcheng()
				&& satori != use.from) {
			QList<int> imitation = satori->getPile("imitation");
			if (imitation.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, satori, satori, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *satori = invoke->invoker;
		room->setTag("fangyingTag", data);
		bool use = room->askForUseCard(satori, "@@fangying", "@fangying");
		room->removeTag("fangyingTag");
		return use;
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *satori = invoke->invoker;
		CardUseStruct use = data.value<CardUseStruct>();
		room->obtainCard(satori, use.card, true);
		return false;
	}
};

class Xinyan : public MasochismSkill
{
	
public:
	Xinyan() : MasochismSkill("xinyan")
	{
		
	}
	
	QList<SkillInvokeDetail> triggerable(const Room *room, const DamageStruct &damage) const
	{
		if (damage.to->isAlive() && damage.to->hasSkill(this)) {
			QList<ServerPlayer *> targets;
			foreach(ServerPlayer *p, room->getOtherPlayers(damage.to)) {
				if (!p->isKongcheng() && (!damage.from || p != damage.from))
					targets << p;
			}
			if (!targets.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to, targets, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *satori = invoke->invoker;
		return room->askForSkillInvoke(satori, objectName(), data);
	}
	
	void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
	{
		ServerPlayer *satori = invoke->invoker;
		ServerPlayer *target = room->askForPlayerChosen(satori, invoke->targets, objectName());
		QList<int> card_ids;
		foreach(const Card *card, target->getHandcards()) {
			card_ids << card->getEffectiveId();
		}
		int card_id = room->doGongxin(satori, target, card_ids, objectName());
		room->throwCard(card_id, target, satori);
		if (damage.from && damage.from->isAlive()) {
			const Card *card = Sanguosha->getCard(card_id);
			QList<int> imitation = satori->getPile("imitation");
            if (room->askForCard(damage.from, ".|" + card->getSuitString() + "|.|hand", "@xinyan-discard")) {
				if (!imitation.isEmpty()) {
					DummyCard *dummy = new DummyCard;
					dummy->addSubcards(imitation);
					room->obtainCard(satori, dummy, true);
					satori->clearOnePrivatePile("imitation");
				}
			}
			else {
				room->loseHp(damage.from);
				if (!imitation.isEmpty()) {
					DummyCard *dummy = new DummyCard;
					dummy->addSubcards(imitation);
					room->throwCard(dummy, satori, damage.from);
					satori->clearOnePrivatePile("imitation");
				}
			}
		}
	}
};

DuannianCard::DuannianCard()
{
	target_fixed = true;
	will_throw = false;
	handling_method = Card::MethodNone;
	m_skillName = "duannian";
}

void DuannianCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	source->addToPile("meditation", this, true);
	source->skip(Player::Play);
}

class DuannianVS : public ViewAsSkill
{
	
public:
	DuannianVS() : ViewAsSkill("duannian")
	{
		response_pattern = "@@duannian";
		//filter_pattern = "BasicCard";
	}
	
	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return to_select->isKindOf("BasicCard");
	}
	
	const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() >= 2) {
			DuannianCard *card = new DuannianCard;
			card->addSubcards(cards);
			return card;
		}
		else
			return NULL;
	}
	
	/*virtual const Card *viewAs(const Card *originalCard) const
	{
		DuannianCard *card = new DuannianCard;
		card->addSubcard(originalCard);
		return card;
	}*/
};

class Duannian : public TriggerSkill
{
	
public:
	Duannian() : TriggerSkill("duannian")
	{
		events << EventPhaseChanging << EventPhaseStart;
        view_as_skill = new DuannianVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		ServerPlayer *koishi = room->getCurrent();
		if (koishi->hasSkill(this)) {
			if (event == EventPhaseChanging) {
				PhaseChangeStruct change = data.value<PhaseChangeStruct>();
				if (change.to == Player::Play && !koishi->hasFlag("CannotDuannian"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, koishi, koishi, NULL, false);
				else if (change.to == Player::NotActive && koishi->hasFlag("CannotDuannian"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, koishi, koishi, NULL, true);
			}
			else if (event == EventPhaseStart) {
				if (koishi->getPhase() == Player::RoundStart) {
					QList<int> meditation = koishi->getPile("meditation");
					if (!meditation.isEmpty())
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, koishi, koishi, NULL, true);
				}
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *koishi = invoke->invoker;
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			if (change.to == Player::Play)
				return room->askForUseCard(koishi, "@@duannian", "@duannian");
		}
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *koishi = invoke->invoker;
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			if (change.to == Player::NotActive)
				room->setPlayerFlag(koishi, "-CannotDuannian");
		}
		else if (event == EventPhaseStart) {
			QList<int> meditation = koishi->getPile("meditation");
			DummyCard *dummy = new DummyCard;
			dummy->addSubcards(meditation);
			room->obtainCard(koishi, dummy, true);
			koishi->clearOnePrivatePile("meditation");
			room->setPlayerFlag(koishi, "CannotDuannian");
		}
		return false;
	}
};

class DuannianSlash : public TargetModSkill
{
	
public:
	DuannianSlash() : TargetModSkill("#duannian-slash")
	{
		pattern = "Slash";
	}
	
	int getResidueNum(const Player *koishi, const Card *) const
	{
		if (koishi->hasFlag("CannotDuannian"))
			return 1000;
		else
			return 0;
	}
	
	int getExtraTargetNum(const Player *koishi, const Card *) const
	{
		if (koishi->hasFlag("CannotDuannian"))
			return 1;
		else
			return 0;
	}
};

class Xinping : public ProhibitSkill
{
	
public:
	Xinping() : ProhibitSkill("xinping")
	{
		
	}
	
	bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &) const
	{
		return to->getPile("meditation").length() > 0 && (card->isKindOf("SavageAssault") || card->isKindOf("MindReading") || card->isKindOf("Haze"));
	}
};

HunquCard::HunquCard()
{
	
}

bool HunquCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && (to_select->isWounded() || to_select != Self);
}

void HunquCard::onEffect(const CardEffectStruct &effect) const
{
	effect.from->turnOver();
	Room *room = effect.from->getRoom();
	effect.from->tag["HunquTarget"] = QVariant::fromValue(effect.to);
	QString choice;
	if (!effect.to->isWounded())
		choice = "HunquDamage";
    else if (effect.to == effect.from)
		choice = "HunquRecover";
	else
		choice = room->askForChoice(effect.from, "hunqu", "HunquDamage+HunquRecover");
	if (choice == "HunquRecover") {
		effect.to->drawCards(1);
		RecoverStruct recover;
		recover.recover = 1;
		recover.who = effect.from;
		recover.reason = "hunqu";
		room->recover(effect.to, recover);
	}
	else if (choice == "HunquDamage") {
		if (!effect.to->isKongcheng()) {
			int id = room->askForCardChosen(effect.from, effect.to, "h", "hunqu");
			room->throwCard(id, effect.to, effect.from);
		}
		room->damage(DamageStruct("hunqu", effect.from, effect.to));
	}
}

class HunquVS : public ZeroCardViewAsSkill
{
	
public:
    HunquVS() : ZeroCardViewAsSkill("hunqu")
	{
		response_pattern = "@@hunqu";
	}
	
	/*virtual bool isEnabledAtPlay(const Player *player) const
	{
		return !player->hasUsed("HunquCard");
	}*/
	
	virtual const Card *viewAs() const
	{
		return new HunquCard;
	}
};

class Hunqu : public TriggerSkill
{
	
public:
	Hunqu() : TriggerSkill("hunqu")
	{
		events << EventPhaseStart;
		view_as_skill = new HunquVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *mystia = room->getCurrent();
		if (mystia->hasSkill(this) && mystia->getPhase() == Player::Play) {
			if (!mystia->isKongcheng())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, mystia, mystia, NULL, false);
			else {
				foreach(ServerPlayer *p, room->getAlivePlayers()) {
					if (p->isWounded())
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, mystia, mystia, NULL, false);
				}
			}
		}
			
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *mystia = invoke->invoker;
		return room->askForUseCard(mystia, "@@hunqu", "@hunqu");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
	{
		return false;
	}
};

class Yemang : public ProhibitSkill
{
	
public:
	Yemang() : ProhibitSkill("yemang")
	{
		
	}
	
	bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &) const
	{
		return to->hasSkill(this) && card->isKindOf("TrickCard") && card->isBlack();
	}
};

ZhangqiCard::ZhangqiCard()
{
	
}

bool ZhangqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void ZhangqiCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	int id = room->askForCardChosen(effect.from, effect.to, "h", "zhangqi");
	const Card *card = Sanguosha->getCard(id);
    room->showCard(effect.to, id);
	if (card->getSuit() == Card::Club)
		room->setPlayerFlag(effect.to, "ZhangqiFlag");
	else if (card->getSuit() == Card::Heart)
		room->setFixedDistance(effect.from, effect.to, 1);
}

class ZhangqiVS : public ZeroCardViewAsSkill
{
	
public:
	ZhangqiVS() : ZeroCardViewAsSkill("zhangqi")
	{
		response_pattern = "@@zhangqi";
	}
	
	virtual const Card *viewAs() const
	{
		return new ZhangqiCard;
	}
};

class Zhangqi : public TriggerSkill
{
	
public:
	Zhangqi() : TriggerSkill("zhangqi")
	{
		events << EventPhaseStart << ConfirmDamage;
		view_as_skill = new ZhangqiVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *yamame = room->getCurrent();
			if (yamame && yamame->isAlive() && yamame->hasSkill(this)) {
				if (yamame->getPhase() == Player::RoundStart) {
					QList<ServerPlayer *> targets;
					foreach (ServerPlayer *p, room->getOtherPlayers(yamame)) {
						if (!p->isKongcheng())
							targets << p;
					}
					if (!targets.isEmpty())
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yamame, yamame, targets, false);
				}
				else if (yamame->getPhase() == Player::Finish) {
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yamame, yamame, NULL, true);
				}
			}
		}
		else if (event == ConfirmDamage) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *yamame = damage.from;
			if (yamame && yamame->isAlive() && yamame->hasSkill(this) && damage.card && damage.card->isKindOf("Slash")
					&& damage.to->isAlive() && damage.to->hasFlag("ZhangqiFlag"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yamame, yamame, NULL, true, damage.to);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yamame = invoke->invoker;
		if (event == EventPhaseStart && yamame->getPhase() == Player::RoundStart)
			return room->askForUseCard(yamame, "@@zhangqi", "@zhangqi");
		else
			return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yamame = invoke->invoker;
		if (event == EventPhaseStart && yamame->getPhase() == Player::Finish) {
			foreach(ServerPlayer *p, room->getOtherPlayers(yamame)) {
				room->setPlayerFlag(p, "-ZhangqiFlag");
				room->setFixedDistance(yamame, p, -1);
			}
		}
		else if (event == ConfirmDamage) {
			DamageStruct damage = data.value<DamageStruct>();
			damage.damage++;
			data = QVariant::fromValue(damage);
		}
		return false;
	}
};

class Zhusi : public TriggerSkill
{
	
public:
	Zhusi() : TriggerSkill("zhusi")
	{
		frequency = Frequent;
		events << SlashMissed;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		SlashEffectStruct effect = data.value<SlashEffectStruct>();
		if (effect.from && effect.from->isAlive() && effect.from->hasSkill(this))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, effect.from, effect.from, NULL, false);
		else if (effect.to && effect.to->isAlive() && effect.to->hasSkill(this))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, effect.to, effect.to, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yamame = invoke->invoker;
		yamame->drawCards(1);
		return false;
	}
};

class Canye : public TriggerSkill
{
	
public:
	Canye() : TriggerSkill("canye")
	{
		frequency = Compulsory;
		events << TargetSpecified;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *shizuha = use.from;
		if (shizuha && shizuha->isAlive() && shizuha->hasSkill(this) && use.card->isKindOf("Slash")) {
			foreach(ServerPlayer *p, use.to) {
				if (p->getHp() <= 1)
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, shizuha, shizuha, NULL, true, p);
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *shizuha = invoke->invoker;
		CardUseStruct use = data.value<CardUseStruct>();
		room->sendCompulsoryTriggerLog(shizuha, objectName());
		QVariantList jink_list = shizuha->tag["Jink_" + use.card->toString()].toList();
		for (int i = 0; i < use.to.length(); i++) {
			if (jink_list.at(i).toInt() == 1)
				jink_list.replace(i, QVariant(2));
		}
        shizuha->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
		return false;
	}
};

class Diaofeng : public TriggerSkill
{
	
public:
	Diaofeng() : TriggerSkill("diaofeng")
	{
		events << TargetSpecified;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *shizuha = room->findPlayerBySkillName(objectName());
		if (shizuha && shizuha->isAlive() && use.to.contains(shizuha) && use.card->isNDTrick() && shizuha != use.from) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, shizuha, shizuha, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *shizuha = invoke->invoker;
		return room->askForSkillInvoke(shizuha, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *shizuha = invoke->invoker;
		JudgeStruct judge;
		judge.who = shizuha;
		judge.pattern = ".|black";
		judge.reason = objectName();
		judge.good = true;
		room->judge(judge);
        if (judge.isGood()) {
			CardUseStruct use = data.value<CardUseStruct>();
			QString choice;
			if (use.from->isKongcheng())
				choice = "DFDraw";
			else
				choice = room->askForChoice(shizuha, objectName(), "DFDraw+DFDiscard", data);
			if (choice == "DFDraw")
				shizuha->drawCards(1);
			else if (choice == "DFDiscard") {
				int id = room->askForCardChosen(shizuha, use.from, "h", objectName());
				room->throwCard(id, use.from, shizuha);
			}
		}
		return false;
	}
};

class Tuji : public OneCardViewAsSkill
{
	
public:
	Tuji() : OneCardViewAsSkill("tuji")
	{
		filter_pattern = ".|.|.|hand";
	}
	
	virtual bool isEnabledAtPlay(const Player *player) const
	{
		return !player->hasUsed("AmazingGrace");
	}
	
	virtual const Card *viewAs(const Card *originalCard) const
	{
		AmazingGrace *card = new AmazingGrace(originalCard->getSuit(), originalCard->getNumber());
		card->addSubcard(originalCard);
		return card;
	}
};

class TujiAdd : public TriggerSkill
{
	
public:
	TujiAdd() : TriggerSkill("#tuji-add")
	{
		events << TargetConfirmed;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *minoriko = room->findPlayerBySkillName("tuji");
		if (minoriko && minoriko->isAlive() && use.card->isKindOf("AmazingGrace"))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, minoriko, minoriko, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *minoriko = invoke->invoker;
		return room->askForSkillInvoke(minoriko, "tuji", data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *minoriko = invoke->invoker;
		minoriko->drawCards(1);
		return false;
	}
};

class Ganli : public FilterSkill
{
	
public:
	Ganli() : FilterSkill("ganli")
	{
		
	}
	
	bool viewFilter(const Card *to_select) const
	{
		return to_select->isKindOf("Analeptic");
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		Peach *peach = new Peach(originalCard->getSuit(), originalCard->getNumber());
		peach->setSkillName(objectName());
		WrappedCard *card = Sanguosha->getWrappedCard(originalCard->getId());
		card->takeOver(peach);
		return card;
	}
};

class Qiangyun : public TriggerSkill
{
public:
    Qiangyun() : TriggerSkill("qiangyun")
    {
		events << AskForRetrial;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        JudgeStruct *judge = data.value<JudgeStruct *>();
		ServerPlayer *tewi = room->findPlayerBySkillName(objectName());
		if (tewi && tewi->isAlive())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tewi, tewi, NULL, true);
		return QList<SkillInvokeDetail>();
    }
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *tewi = invoke->invoker;
		JudgeStruct *judge = data.value<JudgeStruct *>();
        const Card *card = room->askForCard(tewi, ".|.|.|hand", "@qiangyun-retrial", QVariant::fromValue(judge), Card::MethodResponse,
            judge->who, true, objectName());
		if (card != NULL) {
			room->retrial(card, tewi, judge, objectName(), true);
		}
		return false;
	}
};

class Jiahu : public TriggerSkill
{
	
public:
	Jiahu() : TriggerSkill("jiahu")
	{
		events << CardAsked;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardAskedStruct ask = data.value<CardAskedStruct>();
		ServerPlayer *tewi = room->findPlayerBySkillName(objectName());
		if (tewi && tewi->isAlive() && ask.pattern == "jink" && ask.player->isWounded() && ask.player->getArmor() == NULL
				&& tewi->distanceTo(ask.player) <= 1)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tewi, tewi, NULL, false, ask.player);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *tewi = invoke->invoker;
		tewi->tag["JiahuTarget"] = QVariant::fromValue(invoke->preferredTarget);
        return room->askForSkillInvoke(tewi, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *tewi = invoke->invoker;
		ServerPlayer *target = invoke->preferredTarget;
		JudgeStruct judge;
		judge.who = target;
		judge.reason = objectName();
		judge.pattern = ".|red";
		judge.good = true;
		room->judge(judge);
		if (judge.isGood()) {
			Jink *jink = new Jink(Card::NoSuit, 0);
			jink->setSkillName(objectName());
			room->provide(jink, tewi);
		}
		return false;
	}
};

XianboCard::XianboCard()
{
	target_fixed = true;
	will_throw = false;
}

void XianboCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	room->moveCardTo(this, NULL, Player::DrawPile, false);
	if (source->getHandcardNum() < source->getMaxHp()) {
		QList<ServerPlayer *> targets;
		foreach(ServerPlayer *p, room->getOtherPlayers(source)) {
			if (!p->isKongcheng())
				targets << p;
		}
		if (!targets.isEmpty()) {
			ServerPlayer *target = room->askForPlayerChosen(source, targets, "xianbo", "@xianbo-target", true);
			if (target != NULL) {
				int id = room->askForCardChosen(source, target, "h", "xianbo");
				room->obtainCard(source, Sanguosha->getCard(id), false);
			}
		}
	}
};

class Xianbo : public OneCardViewAsSkill
{
	
public:
	Xianbo() : OneCardViewAsSkill("xianbo")
	{
		filter_pattern = ".|.|.|hand";
	}
	
	virtual bool isEnabledAtPlay(const Player *player) const
	{
		return !player->hasUsed("XianboCard");
	}
	
	virtual const Card *viewAs(const Card *originalCard) const
	{
		XianboCard *card = new XianboCard;
		card->addSubcard(originalCard);
		return card;
	}
};

HuanniCard::HuanniCard()
{
	target_fixed = true;
}

void HuanniCard::use(Room *, ServerPlayer *, QList<ServerPlayer *> &) const
{
	
}

class HuanniVS : public ViewAsSkill
{
	
public:
	HuanniVS() : ViewAsSkill("huanni")
	{
		response_pattern = "@@huanni";
	}
	
	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return selected.length() < 2 && !to_select->isEquipped() && to_select->isRed();
	}
	
	virtual const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() < 2)
			return NULL;
		HuanniCard *card = new HuanniCard;
		card->addSubcards(cards);
		return card;
	}
};

class Huanni : public MasochismSkill
{
	
public:
	Huanni() : MasochismSkill("huanni")
	{
        view_as_skill = new HuanniVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *reisen = damage.to;
        if (reisen && reisen->isAlive() && reisen->hasSkill(this) && damage.from && damage.from->isAlive() && !reisen->isKongcheng()) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, reisen, reisen, NULL, true, damage.from);
		}
		return QList<SkillInvokeDetail>();
	}
	
    void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
	{
		ServerPlayer *reisen = invoke->invoker;
		ServerPlayer *target = invoke->preferredTarget;
		const Card *card = room->askForCard(reisen, ".|red|.|hand", "@huanni-show", QVariant::fromValue(damage), Card::MethodNone,
			target, false, objectName());
		if (card != NULL) {
			room->notifySkillInvoked(reisen, objectName());
			room->showCard(reisen, card->getEffectiveId());
			if (!room->askForUseCard(target, "@@huanni", "@huanni-throw"))
				reisen->drawCards(2);
		}
	}
};

CitanCard::CitanCard()
{
	
}

bool CitanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self && qAbs(Self->getHandcardNum() - to_select->getHandcardNum() <= Self->getLostHp())
		&& Self->getHandcardNum() + to_select->getHandcardNum() >= 1;
}

void CitanCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	int n1 = effect.from->getHandcardNum();
	int n2 = effect.to->getHandcardNum();
	foreach(ServerPlayer *p, room->getOtherPlayers(effect.from)) {
		if (p != effect.to) {
			JsonArray arr;
			arr << effect.from->objectName() << effect.to->objectName();
			room->doNotify(p, QSanProtocol::S_COMMAND_EXCHANGE_KNOWN_CARDS, arr);
		}
	}
	QList<CardsMoveStruct> exchangeMove;
	CardsMoveStruct move1(effect.from->handCards(), effect.to, Player::PlaceHand,
		CardMoveReason(CardMoveReason::S_REASON_SWAP, effect.from->objectName(), effect.to->objectName(), "citan", QString()));
	CardsMoveStruct move2(effect.to->handCards(), effect.from, Player::PlaceHand,
		CardMoveReason(CardMoveReason::S_REASON_SWAP, effect.to->objectName(), effect.from->objectName(), "citan", QString()));
	exchangeMove.push_back(move1);
	exchangeMove.push_back(move2);
	room->moveCardsAtomic(exchangeMove, false);
	LogMessage log;
	log.type = "#Citan";
	log.from = effect.from;
	log.to << effect.to;
	log.arg = QString::number(n1);
	log.arg2 = QString::number(n2);
	room->sendLog(log);
	if (effect.from->getHandcardNum() <= effect.to->getHandcardNum())
		room->damage(DamageStruct("citan", effect.from, effect.to, 1, DamageStruct::Thunder));
}

class Citan : public ZeroCardViewAsSkill
{
	
public:
	Citan() : ZeroCardViewAsSkill("citan")
	{
		
	}
	
	virtual bool isEnabledAtPlay(const Player *player) const
	{
		return !player->hasUsed("CitanCard");
	}
	
	virtual const Card *viewAs() const
	{
		return new CitanCard;
	}
};

THStandardPackage::THStandardPackage()
	: Package("thstandard")
{
	General *reimu = new General(this, "reimu$", "hakurei", 4, false);
	reimu->addSkill(new Fengmo);
	reimu->addSkill(new FengmoAdd);
	reimu->addSkill(new FengmoClear);
	reimu->addSkill(new Guayu);
	reimu->addSkill(new GuayuAdd);
	reimu->addSkill(new GuayuClear);
	related_skills.insertMulti("fengmo", "#fengmoadd");
	related_skills.insertMulti("fengmo", "#fengmo-clear");
	related_skills.insertMulti("guayu$", "#guayu-add");
	related_skills.insertMulti("guayu$", "#guayu-clear");
	
	General *marisa = new General(this, "marisa", "hakurei", 3, false);
	marisa->addSkill(new Xingchen);
	marisa->addSkill(new Sheyue);
	marisa->addSkill(new SheyueAdd);
	related_skills.insertMulti("sheyue", "#sheyueadd");
	
	General *suika = new General(this, "suika", "hakurei", 4, false);
	suika->addSkill(new Shantou);
	
	General *aya = new General(this, "aya", "hakurei", 3, false);
	aya->addSkill(new Daoshe);
	aya->addSkill(new Fengmi);
	aya->addSkill(new FengmiMax);
	related_skills.insertMulti("fengmi", "#fengmi-max");
	
	General *cirno = new General(this, "cirno", "hakurei", 4, false);
	cirno->addSkill(new Bingpu);
	
	General *remilia = new General(this, "remilia", "hakurei", 3, false);
	remilia->addSkill(new Shengqiang);
	remilia->addSkill(new ShengqiangMaso);
	related_skills.insertMulti("shengqiang", "#shengqiang-maso");
	remilia->addSkill(new Hongye);
	
	General *flandre = new General(this, "flandre", "hakurei", 3, false);
	flandre->addSkill(new Shengxue);
	flandre->addSkill(new Wosui);
	flandre->addSkill(new Heiyan);
	
	General *kyouko = new General(this, "kyouko", "hakurei", 3, false);
	kyouko->addSkill(new Huiyin);
	kyouko->addSkill(new Kuopin);
	kyouko->addSkill(new KuopinMaxCardsSkill);
	related_skills.insertMulti("kuopin", "#kuopin-maxcardsskill");
	
	General *nitori = new General(this, "nitori", "hakurei", 4, false);
	nitori->addSkill(new Shuimu);
	
	General *pachouli = new General(this, "pachouli", "hakurei", 3, false);
	pachouli->addSkill(new Yaoshi);
	pachouli->addSkill(new Huangyan);
	pachouli->addSkill(new Jingyue);
	
	General *alice = new General(this, "alice", "hakurei", 3, false);
	alice->addSkill(new Diaoou);
	alice->addSkill(new Anji);
	
	General *letty = new General(this, "letty", "hakurei", 3, false);
	letty->addSkill(new Xuebeng);
	letty->addSkill(new XuebengAdd);
	letty->addSkill(new Yaofeng);
	letty->addSkill(new Jihan);
	related_skills.insertMulti("xuebeng", "#xuebeng-add");
	
	General *sanae = new General(this, "sanae$", "moriya", 4, false);
	sanae->addSkill(new Xinxing);
	sanae->addSkill(new Qiji);
	
	General *kanako = new General(this, "kanako", "moriya", 4, false);
	kanako->addSkill(new Yuzhu);
	
	General *suwako = new General(this, "suwako", "moriya", 3, false);
	suwako->addSkill(new Jinlun);
	suwako->addSkill(new Chiwa);
	
	General *satori = new General(this, "satori", "moriya", 3, false);
	satori->addSkill(new Fangying);
	satori->addSkill(new Xinyan);
	
	General *koishi = new General(this, "koishi", "moriya", 4, false);
	koishi->addSkill(new Duannian);
	koishi->addSkill(new DuannianSlash);
	related_skills.insertMulti("duannian", "#duannian-slash");
	koishi->addSkill(new Xinping);
	
	General *mystia = new General(this, "mystia", "moriya", 3, false);
	mystia->addSkill(new Hunqu);
	mystia->addSkill(new Yemang);
	
	General *yamame = new General(this, "yamame", "moriya", 4, false);
	yamame->addSkill(new Zhangqi);
	yamame->addSkill(new Zhusi);
	
	General *shizuha = new General(this, "shizuha", "moriya", 4, false);
	shizuha->addSkill(new Canye);
	shizuha->addSkill(new Diaofeng);
	
	General *minoriko = new General(this, "minoriko", "moriya", 3, false);
	minoriko->addSkill(new Tuji);
	minoriko->addSkill(new TujiAdd);
	related_skills.insertMulti("tuji", "#tuji-add");
	minoriko->addSkill(new Ganli);
	
	General *tewi = new General(this, "tewi", "moriya", 3, false);
	tewi->addSkill(new Qiangyun);
	tewi->addSkill(new Jiahu);
	
	General *reisen = new General(this, "reisen", "moriya", 3, false);
	reisen->addSkill(new Xianbo);
	reisen->addSkill(new Huanni);
	
	General *iku = new General(this, "iku", "moriya", 4, false);
	iku->addSkill(new Citan);
	
	addMetaObject<FengmoCard>();
	addMetaObject<GuayuCard>();
	addMetaObject<ShantouCard>();
	addMetaObject<DaosheCard>();
	addMetaObject<HeiyanCard>();
	addMetaObject<HuangyanCard>();
	addMetaObject<DiaoouCard>();
	addMetaObject<AnjiCard>();
	addMetaObject<YaofengCard>();
	addMetaObject<YuzhuCard>();
	addMetaObject<ChiwaCard>();
	addMetaObject<FangyingCard>();
	addMetaObject<DuannianCard>();
	addMetaObject<HunquCard>();
	addMetaObject<ZhangqiCard>();
    addMetaObject<XianboCard>();
	addMetaObject<HuanniCard>();
	addMetaObject<CitanCard>();
}

ADD_PACKAGE(THStandard)
