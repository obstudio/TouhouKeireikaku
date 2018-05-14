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
	target_fixed = false;
}

bool FengmoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select != Self && !to_select->hasFlag("FengmoTargeted")
        && (to_select->getHandcardNum() >= 2 || to_select->getMark("@spell") > 0);
}

void FengmoCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *from = effect.from;
	ServerPlayer *to = effect.to;
	Room *room = from->getRoom();
	room->setPlayerFlag(to, "FengmoTargeted");

	QString self_choice;
	if (getSubcards().isEmpty()) {
		room->removePlayerMark(from, "@spell", 1);
		self_choice = "LoseSpell";
	} else {
		self_choice = "Discard";
	}

	to->tag["FengmoReimuChoice"] = QVariant(self_choice);

	QString choice;
	if (to->getMark("@spell") <= 0) {
		choice = "Discard";
		room->askForDiscard(to, "fengmo1", 2, 2, false, false, "@fengmo-compulsory-discard");
	} else if (!room->askForDiscard(to, "fengmo2", 2, 2, true, false, "@fengmo-optional-discard")) {
		choice = "LoseSpell";
		room->removePlayerMark(to, "@spell", 1);
	} else {
		choice = "Discard";
	}

	if (self_choice != choice) {
		room->setPlayerFlag(from, "FengmoProhibited");
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
		if (player->getHandcardNum() < 2 && player->getMark("@spell") <= 0)
			return false;
		
		if (player->hasFlag("FengmoProhibited"))
			return false;
		
		foreach (const Player *p, player->getAliveSiblings()) {
			if (!p->hasFlag("FengmoTargeted") && (p->getHandcardNum() >= 2 || p->getMark("@spell") > 0)) {
				return true;
			}
		}
		return false;
	}
	
	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return selected.length() < 2 && !to_select->isEquipped();
	}
	
	const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() != 0 && cards.length() != 2)
			return NULL;
		
		if (cards.isEmpty() && Self->getMark("@spell") == 0)
			return NULL;
		
		FengmoCard *card = new FengmoCard();
		if (cards.length() > 0)
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
		events << EventPhaseChanging;
	}
	
	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *reimu = change.player;
			if (reimu && reimu->isAlive() && reimu->hasSkill("fengmo") && change.to == Player::NotActive) {
				if (reimu->hasFlag("FengmoProhibited"))
					room->setPlayerFlag(reimu, "-FengmoProhibited");
				foreach (ServerPlayer *p, room->getOtherPlayers(reimu)) {
					if (p->hasFlag("FengmoTargeted"))
						room->setPlayerFlag(p, "-FengmoTargeted");
				}
			}
		}
	}
};

/* GuayuCard::GuayuCard()
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
}; */

class Guayu : public TriggerSkill
{
public:
	Guayu() : TriggerSkill("guayu$")
	{
		events << EventPhaseStart;
		// view_as_skill = new GuayuViewAsSkill;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *player = room->getCurrent();
		ServerPlayer *reimu = room->findPlayerBySkillName(objectName());
		if (reimu && reimu->isAlive() && reimu->hasLordSkill(objectName()) && player->getPhase() == Player::Play
				&& player != reimu && player->getKingdom() == "hakurei") {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, reimu, player, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        return room->askForSkillInvoke(invoke->invoker, objectName(), data);
    }
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
        ServerPlayer *reimu = invoke->owner;
		ServerPlayer *player = invoke->invoker;
		room->notifySkillInvoked(reimu, objectName());
		QList<ServerPlayer *> to;
		to << reimu;
		room->touhouLogmessage("#InvokeOthersSkill", player, objectName(), to);
		QString choice;
		if (player->getMark("@spell") <= 0) {
			choice = "GYAdd";
		} else {
			choice = room->askForChoice(player, objectName(), "GYAdd+GYGive");
		}
		if (choice == "GYAdd") {
			LogMessage log;
			log.type = "#GuayuAddSpell";
			log.from = player;
			log.to << reimu;
			log.arg = QString::number(1);
			room->addPlayerMark(reimu, "@spell", 1);
			room->sendLog(log);
		} else if (choice == "GYGive") {
			LogMessage log;
			log.type = "#GuayuGiveSpell";
			log.from = player;
			log.to << reimu;
			log.arg = QString::number(1);
			room->removePlayerMark(player, "@spell", 1);
			room->addPlayerMark(reimu, "@spell", 1);
			room->sendLog(log);
			player->drawCards(1);
		}
		
		return false;
	}
};

class Xingchen : public TriggerSkill
{
public:
	Xingchen() : TriggerSkill("xingchen")
	{
		events << Damage << TargetConfirmed << EventPhaseChanging;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == TargetConfirmed) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *marisa = room->findPlayerBySkillName(objectName());
			if (marisa && marisa->isAlive() && use.card->isKindOf("Slash") && marisa->getPhase() != Player::NotActive) {
				foreach (ServerPlayer *to, use.to) {
					room->setPlayerFlag(to, "XingchenSlashed");
				}
			}
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *marisa = change.player;
			if (marisa && marisa->isAlive() && marisa->hasSkill(this) && change.to == Player::NotActive) {
				foreach (ServerPlayer *p, room->getAlivePlayers()) {
					if (p->hasFlag("XingchenSlashed"))
						room->setPlayerFlag(p, "-XingchenSlashed");
				}
			}
		}
	}
	
    QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == Damage) {
			DamageStruct damage = data.value<DamageStruct>();
			if (damage.from->hasSkill(objectName()) && damage.card && damage.card->isKindOf("Slash")
					&& damage.to && damage.to->isAlive()) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *target, room->getOtherPlayers(damage.to)) {
					if (damage.to->inMyAttackRange(target) && !target->hasFlag("XingchenSlashed")) {
						targets << target;
					}
				}
				if (!targets.empty()) {
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, targets, false);
				}
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
			/*QList<ServerPlayer *> targets;
            foreach (ServerPlayer *target, room->getOtherPlayers(damage.to)) {
				if (damage.to->inMyAttackRange(target)) {
					targets << target;
				}
			}*/
            ServerPlayer *victim = room->askForPlayerChosen(damage.from, invoke->targets, objectName(), "@xingchen-invoke");
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
		events << CardsMoveOneTime << EventPhaseStart << EventPhaseChanging;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *aya = change.player;
			if (aya && aya->isAlive() && aya->hasSkill(this) && change.from == Player::Finish) {
				if (aya->hasFlag("FengmiAdjustMaxCards"))
					room->setPlayerFlag(aya, "-FengmiAdjustMaxCards");
				room->setPlayerMark(aya, "@fengmi", 0);
			}
		}
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardsMoveOneTime) {
			ServerPlayer *aya = room->findPlayerBySkillName(objectName());
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			if (aya && aya->isAlive() && move.from && ((move.reason.m_reason ^ 3) % 16 == 0) && aya->getPhase() == Player::Play)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, aya, aya, NULL, true);
		}
		else if (event == EventPhaseStart) {
			ServerPlayer *aya = data.value<ServerPlayer *>();
			if (aya && aya->isAlive() && aya->hasSkill(this) && aya->getPhase() == Player::Discard)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, aya, aya, NULL, false);
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

class Shiling : public TriggerSkill
{

public:
	Shiling() : TriggerSkill("shiling")
	{
		events << TargetConfirmed << TrickCardCanceling << CardFinished;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == CardFinished) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *remilia = use.from;
			if (remilia && remilia->isAlive() && remilia->hasFlag("ShilingUnavoidable") && use.card->isNDTrick()) {
				room->setPlayerFlag(remilia, "-ShilingUnavoidable");
			}
		}
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == TargetConfirmed) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *remilia = room->findPlayerBySkillName(objectName());
			if (remilia && remilia->isAlive() && (use.card->isKindOf("Slash") || use.card->isNDTrick())) {
				if (remilia == use.from && use.to.length() == 1 && !use.to.contains(remilia) && remilia->faceUp()) {
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, remilia, remilia, NULL, false);
				} else if (use.to.contains(remilia) && use.to.length() == 1 && use.from != remilia && !remilia->faceUp()
					&& use.from && use.from->isAlive()) {
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, remilia, remilia, NULL, false, use.from);
				}
			}
		} else if (event == TrickCardCanceling) {
			CardEffectStruct effect = data.value<CardEffectStruct>();
			ServerPlayer *remilia = effect.from;
			if (remilia && effect.card->hasFlag("ShilingUnavoidable"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, remilia, remilia, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *remilia = invoke->invoker;
		if (event == TargetConfirmed) {
			CardUseStruct use = data.value<CardUseStruct>();
			if (remilia == use.from) {
				return room->askForSkillInvoke(remilia, objectName(), data);
			} else if (use.to.contains(remilia)) {
				return room->askForCard(remilia, ".", "@shiling-discard", data, objectName());
			}
		}
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *remilia = invoke->invoker;
		if (event == TargetConfirmed) {
			CardUseStruct use = data.value<CardUseStruct>();
			if (remilia == use.from) {
				ServerPlayer *target = use.to.at(0);
				remilia->turnOver();
				if (use.card->isKindOf("Slash")) {
					QVariantList jink_list = remilia->tag["Jink_" + use.card->toString()].toList();
					LogMessage log;
					log.type = "#NoJink";
					log.from = target;
					room->sendLog(log);
					jink_list.replace(0, QVariant(0));
					remilia->tag["Jink_" + use.card->toString()] = QVariant::fromValue(jink_list);
				} else if (use.card->isNDTrick()) {
					room->touhouLogmessage("#ShilingUnavoid", remilia);
					use.card->setFlags("ShilingUnavoidable");
				}
			} else if (use.to.contains(remilia)) {
				use.nullified_list << remilia->objectName();
			}
			data = QVariant::fromValue(use);
		} else if (event == TrickCardCanceling) {
			return true;
		}
		return false;
	}
};

class Xuewang : public TriggerSkill
{

public:
	Xuewang() : TriggerSkill("xuewang")
	{
		events << CardsMoveOneTime;
		frequency = Compulsory;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *remilia = qobject_cast<ServerPlayer *>(move.from);
		if (remilia && remilia->isAlive() && remilia->hasSkill(this) && move.from_places.contains(Player::PlaceHand) 
			&& remilia->isKongcheng() && remilia->getPhase() == Player::NotActive)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, remilia, remilia, NULL, true);
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *remilia = invoke->invoker;
		room->sendCompulsoryTriggerLog(remilia, objectName());
		if (remilia->getMaxHp() > remilia->getHandcardNum())
			remilia->drawCards(remilia->getMaxHp() - remilia->getHandcardNum());
		if (!remilia->faceUp()) {
			remilia->turnOver();
			room->loseHp(remilia, 1);
		}
		return false;
	}
};

class XuewangMC : public MaxCardsSkill
{

public:
	XuewangMC() : MaxCardsSkill("#xuewang-maxcards")
	{
	}

	int getFixed(const Player *remilia) const
	{
		if (remilia->hasSkill(this)) {
			int max = 0;
			foreach (const Player *p, remilia->getAliveSiblings()) {
				if (p->getHp() > max)
					max = p->getHp();
			}
			return max;
		}
		return -1;
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
		return room->askForCard(kyouko, ".|" + suits, "@huiyin-discard", data, objectName());
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

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseEnd) {
			ServerPlayer *kyouko = data.value<ServerPlayer *>();
			if (kyouko && kyouko->isAlive() && kyouko->hasSkill(this) && kyouko->getPhase() == Player::Finish
				&& kyouko->hasFlag("kuopin"))
				room->setPlayerFlag(kyouko, "-kuopin");
		}
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

class LinshangVS : public ZeroCardViewAsSkill
{

public:
	LinshangVS() : ZeroCardViewAsSkill("linshang")
	{
		response_pattern = "@@linshang";
	}

	const Card *viewAs() const
	{
		QString card_str = Self->property("linshang").toString();
		Card *card = Sanguosha->cloneCard(card_str, Card::NoSuit, -1);
		card->setSkillName(objectName());
		return card;
	}
};

class Linshang : public TriggerSkill
{

public:
	Linshang() : TriggerSkill("linshang")
	{
		events << DrawNCards << AfterDrawNCards << EventPhaseChanging;
		view_as_skill = new LinshangVS;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *nitori = change.player;
			if (nitori && nitori->isAlive() && nitori->hasFlag("LinshahngUsed") && change.to == Player::NotActive)
				room->setPlayerFlag(nitori, "-LinshangUsed");
		}
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == DrawNCards) {
			DrawNCardsStruct draw = data.value<DrawNCardsStruct>();
			ServerPlayer *nitori = draw.player;
			if (nitori && nitori->isAlive() && nitori->hasSkill(this) && draw.n > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nitori, nitori, NULL, false);
		} else if (event == AfterDrawNCards) {
			DrawNCardsStruct draw = data.value<DrawNCardsStruct>();
			ServerPlayer *nitori = draw.player;
			if (nitori && nitori->isAlive() && nitori->hasSkill(this) && nitori->hasFlag("LinshangUsed"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nitori, nitori, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *nitori = invoke->invoker;
		if (event == DrawNCards) {
			return room->askForSkillInvoke(nitori, objectName(), data);
		}
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *nitori = invoke->invoker;
		if (event == DrawNCards) {
			DrawNCardsStruct draw = data.value<DrawNCardsStruct>();
			QStringList choices;
			for (int i = 1; i <= draw.n; i++) {
				choices << QString::number(i);
			}
			QString choice = room->askForChoice(nitori, objectName(), choices.join("+"), data);
			int num = choice.toInt();
			draw.n -= num;
			data = QVariant::fromValue(draw);
			room->setPlayerFlag(nitori, "LinshangUsed");
		} else if (event == AfterDrawNCards) {
			DrawNCardsStruct draw = data.value<DrawNCardsStruct>();
			int num = draw.n;
			if (num >= 1) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *p, room->getOtherPlayers(nitori)) {
					if (!p->isKongcheng())
						targets << p;
				}
				if (!targets.isEmpty()) {
					ServerPlayer *target = room->askForPlayerChosen(nitori, targets, objectName(), "@linshang-target");
					room->askForDiscard(target, objectName(), 1, 1, false, false, QString("@linshang-discard:%1").arg(nitori->objectName()));
				}
			} else if (num == 0) {
				QStringList basics;
				basics << "slash";
				if (!Sanguosha->getBanPackages().contains("maneuvering")) {
					basics << "thunder_slash" << "fire_slash" << "analeptic";
				}
				basics << "peach";
				QStringList available = basics;
				for (int i = 0; i < basics.length(); i++) {
					QString pattern = basics.at(i);
					Card *card = Sanguosha->cloneCard(pattern, Card::NoSuit, -1);
					if (card && !card->isAvailable(nitori)) {
						available.removeOne(pattern);
						if (pattern == "slash") {
							available.removeOne("thunder_slash");
							available.removeOne("fire_slash");
						}
					}
				}
				if (!available.isEmpty()) {
					QString choice = room->askForChoice(nitori, "linshangbasic", available.join("+"), data);
					room->setPlayerProperty(nitori, "linshang", choice);
					room->askForUseCard(nitori, "@@linshang", QString("@linshang-use:%1").arg(choice));
				}
			}
		}
		return false;
	}
};

class Jiexun : public TriggerSkill
{

public:
	Jiexun() : TriggerSkill("jiexun")
	{
		events << CardsMoveOneTime << EventPhaseStart << EventPhaseChanging;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *nitori = change.player;
			if (nitori && nitori->isAlive() && change.to == Player::NotActive && nitori->getMark(objectName()) > 0)
				room->setPlayerMark(nitori, objectName(), 0);
		}
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardsMoveOneTime) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			ServerPlayer *nitori = qobject_cast<ServerPlayer *>(move.to);
			if (nitori && nitori->isAlive() && nitori->hasSkill(this) && move.from_places.contains(Player::DrawPile)
				&& move.to_place == Player::PlaceHand && nitori->getPhase() != Player::NotActive)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nitori, nitori, NULL, true);
		} else if (event == EventPhaseStart) {
			ServerPlayer *nitori = data.value<ServerPlayer *>();
			if (nitori && nitori->isAlive() && nitori->hasSkill(this) && nitori->getMark("jiexun") < 2
				&& nitori->getPhase() == Player::Discard)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nitori, nitori, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *nitori = invoke->invoker;
		if (event == EventPhaseStart) {
			return room->askForSkillInvoke(nitori, objectName(), data);
		}
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *nitori = invoke->invoker;
		if (event == CardsMoveOneTime) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			room->addPlayerMark(nitori, objectName(), move.card_ids.length());
		} else if (event == EventPhaseStart) {
			nitori->drawCards(1);
			while (nitori->getMark(objectName()) < 2) {
				if (room->askForSkillInvoke(nitori, objectName(), data))
					nitori->drawCards(1);
			}
		}
		return false;
	}
};

RanhuiCard::RanhuiCard()
{
}

bool RanhuiCard::targetFilter(const QList<const Player *> &selected, const Player *to_select, const Player *Self) const
{
	return selected.isEmpty() && to_select->getPhase() == Player::Finish;
}

void RanhuiCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *from = effect.from;
	ServerPlayer *to = effect.to;
	Room *room = from->getRoom();

	room->damage(DamageStruct("ranhui", from, to, 1, DamageStruct::Fire));
	from->loseMark("@fire", 1);
}

class RanhuiVS : public ViewAsSkill
{

public:
	RanhuiVS() : ViewAsSkill("ranhui")
	{
		response_pattern = "@@ranhui";
	}

	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return selected.length() < 3;
	}

	const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() != 3)
			return NULL;
		
		RanhuiCard *card = new RanhuiCard;
		card->addSubcards(cards);
		return card;
	}
};

class Ranhui : public TriggerSkill
{

public:
	Ranhui() : TriggerSkill("ranhui")
	{
		events << EventPhaseStart;
		view_as_skill = new RanhuiVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *player = data.value<ServerPlayer *>();
		ServerPlayer *pachouli = room->findPlayerBySkillName(objectName());
		if (player && player->isAlive() && pachouli && pachouli->isAlive() && player->getPhase() == Player::Finish
			&& player != pachouli)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, false, player);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@ranhui", "@ranhui-discard");
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		//room->damage(DamageStruct(objectName(), invoke->invoker, invoke->preferredTarget, 1, DamageStruct::Fire));
		return false;
	}
};

class Huzang : public TriggerSkill
{

public:
	Huzang() : TriggerSkill("huzang")
	{
		events << Damage;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *pachouli = damage.from;
		ServerPlayer *target = damage.to;
		if (pachouli && pachouli->isAlive() && pachouli->hasSkill(this) && target && target->isAlive() && damage.damage > 0 && !target->isNude())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, false, target);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *pachouli = invoke->invoker;
		ServerPlayer *target = invoke->preferredTarget;
		pachouli->loseMark("@water", 1);
		int card_id = room->askForCardChosen(pachouli, target, "he", objectName());
		room->throwCard(card_id, target, pachouli);
		return false;
	}
};

class Jiaodi : public TriggerSkill
{

public:
	Jiaodi() : TriggerSkill("jiaodi")
	{
		events << CardsMoveOneTime;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *player = qobject_cast<ServerPlayer *>(move.from);
		ServerPlayer *pachouli = room->findPlayerBySkillName(objectName());
		if (pachouli && pachouli->isAlive() && player && player->isAlive() && player->isWounded() && (move.reason.m_reason % 16 == 3)
			&& move.card_ids.length() >= player->getHp())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, false, player);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		invoke->invoker->tag["JiaodiTarget"] = QVariant::fromValue(invoke->preferredTarget);
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *pachouli = invoke->invoker;
		ServerPlayer *target = invoke->preferredTarget;
		pachouli->loseMark("@wood", 1);
		RecoverStruct recover;
		recover.recover = 1;
		recover.who = pachouli;
		recover.reason = objectName();
		room->recover(target, recover);
		return false;
	}
};

DianjinCard::DianjinCard()
{
	handling_method = Card::MethodRecast;
	will_throw = false;
	target_fixed = true;
	can_recast = true;
}

void DianjinCard::onUse(Room *room, const CardUseStruct &use) const
{
	CardUseStruct card_use = use;
	ServerPlayer *source = card_use.from;
	CardMoveReason reason(CardMoveReason::S_REASON_RECAST, source->objectName());
	reason.m_skillName = "dianjin";
	room->moveCardTo(this, source, NULL, Player::DiscardPile, reason);
	source->broadcastSkillInvoke("@recast");

	int id = card_use.card->getSubcards().first();

	LogMessage log;
	log.type = "#UseCard_Recast";
	log.from = source;
	log.card_str = QString::number(id);
	room->sendLog(log);

	source->drawCards(1, "recast");
	source->loseMark("@gold", 1);
	room->addPlayerMark(source, "dianjin", 1);
}

class Dianjin : public OneCardViewAsSkill
{

public:
	Dianjin() : OneCardViewAsSkill("dianjin")
	{
		filter_pattern = ".";
	}

	bool isEnabledAtPlay(const Player *player) const
	{
		return player->getMark("@gold") > 0 && !player->isKongcheng();
	}

	const Card *viewAs(const Card *originalCard) const
	{
		DianjinCard *card = new DianjinCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class DianjinMaxCards : public MaxCardsSkill
{

public:
	DianjinMaxCards() : MaxCardsSkill("#dianjin-maxcards")
	{
	}

	int getExtra(const Player *target) const
	{
		return target->getMark("dianjin");
	}
};

class DianjinClear : public TriggerSkill
{

public:
	DianjinClear() : TriggerSkill("#dianjin-clear")
	{
		events << EventPhaseChanging;
		frequency = Compulsory;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		PhaseChangeStruct change = data.value<PhaseChangeStruct>();
		ServerPlayer *pachouli = change.player;
		if (pachouli && pachouli->isAlive() && pachouli->getMark("dianjin") > 0 && change.from == Player::Finish)
			room->setPlayerMark(pachouli, "dianjin", 0);
	}
};

class Zhenlei : public TriggerSkill
{

public:
	Zhenlei() : TriggerSkill("zhenlei")
	{
		events << EventPhaseStart;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *pachouli = data.value<ServerPlayer *>();
			if (pachouli && pachouli->isAlive() && pachouli->hasSkill(this) && pachouli->getPhase() == Player::Finish) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *p, room->getAlivePlayers()) {
					if (p->getHp() <= pachouli->getHp())
						targets << p;
				}
				if (!targets.isEmpty())
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, targets, false);
			}
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart)
			return room->askForCard(invoke->invoker, ".|.|.|hand", "@zhenlei-discard", data, objectName());
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *pachouli = invoke->owner;
		if (event == EventPhaseStart) {
			pachouli->loseMark("@earth", 1);
			ServerPlayer *target = room->askForPlayerChosen(pachouli, invoke->targets, objectName(), "@zhenlei-target");
			room->addPlayerMark(target, "fort", 1);
			LogMessage log;
			log.type = "#ZhenleiTarget";
			log.from = pachouli;
			log.to << target;
			log.arg = objectName();
            room->doNotify(pachouli, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());
		}
		return false;
	}
};

class ZhenleiAdd : public TriggerSkill
{

public:
	ZhenleiAdd() : TriggerSkill("#zhenlei-add")
	{
		events << DamageCaused << EventPhaseChanging << Death;
		frequency = Compulsory;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *pachouli = change.player;
			if (pachouli && pachouli->isAlive() && pachouli->hasSkill(this) && change.to == Player::RoundStart) {
				foreach (ServerPlayer *p, room->getAlivePlayers()) {
					room->setPlayerMark(p, "fort", 0);
				}
			}
		} else if (event == Death) {
			DeathStruct death = data.value<DeathStruct>();
			ServerPlayer *pachouli = death.who;
			if (pachouli && pachouli->hasSkill(this)) {
				foreach (ServerPlayer *p, room->getAlivePlayers()) {
					room->setPlayerMark(p, "fort", 0);
				}
			}
		}
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *player = damage.to;
		ServerPlayer *pachouli = room->findPlayerBySkillName(objectName());
		if (damage.from && damage.from->isAlive() && player && player->isAlive() && player->getMark("fort") > 0)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, player, NULL, true, damage.from);
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *pachouli = invoke->owner;
		room->sendCompulsoryTriggerLog(pachouli, "zhenlei");
		ServerPlayer *target = invoke->invoker;
		ServerPlayer *from = invoke->preferredTarget;
		if (!from->isKongcheng())
			room->askForDiscard(from, "zhenlei", 1, 1, false, false, "@zhenlei-damage-discard");
		QString choice = room->askForChoice(from, "zhenlei", "ZLTurn+ZLPrevent", data);
		if (choice == "ZLTurn") {
			from->turnOver();
		} else if (choice == "ZLPrevent") {
			return true;
		}
		return false;
	}
};

HuangyanCard::HuangyanCard()
{
	will_throw = false;
}

bool HuangyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() == 0 && to_select != Self && !to_select->isKongcheng();
}

void HuangyanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	source->loseMark("@sun", 1);
	bool success = source->pindian(targets.first(), "huangyan", Sanguosha->getCard(getSubcards().at(0)));
	if (success) {
		room->damage(DamageStruct(objectName(), source, targets.first(), 1, DamageStruct::Fire));
	} else {
		source->drawCards(1);
		targets.first()->drawCards(1);
	}
}

class HuangyanVS : public OneCardViewAsSkill
{
	
public:
	HuangyanVS() : OneCardViewAsSkill("huangyan")
	{
		filter_pattern = ".";
		response_pattern = "@@huangyan";
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		HuangyanCard *card = new HuangyanCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Huangyan : public TriggerSkill
{

public:
	Huangyan() : TriggerSkill("huangyan")
	{
		events << EventPhaseStart;
		view_as_skill = new HuangyanVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *pachouli = data.value<ServerPlayer *>();
		if (pachouli && pachouli->isAlive() && pachouli->hasSkill(this) && pachouli->getPhase() == Player::Play)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, false);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@huangyan", "@huangyan-pindian");
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

JingyueCard::JingyueCard()
{
}

bool JingyueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return to_select->hasFlag("JingyueTarget");
}

void JingyueCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets) {
		room->setPlayerFlag(p, "JingyueCanceled");
	}
}

class JingyueVS : public ZeroCardViewAsSkill
{

public:
	JingyueVS() : ZeroCardViewAsSkill("jingyue")
	{
		response_pattern = "@@jingyue";
	}

	const Card *viewAs() const
	{
		return new JingyueCard;
	}
};

class Jingyue : public TriggerSkill
{
	
public:
	Jingyue() : TriggerSkill("jingyue")
	{
		events << TargetConfirmed;
		view_as_skill = new JingyueVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *pachouli = room->findPlayerBySkillName(objectName());
		ServerPlayer *from = use.from;
		if (pachouli && pachouli->isAlive() && (use.card->isKindOf("Slash") || use.card->isNDTrick()) && use.to.contains(pachouli))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, false, from);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *pachouli = invoke->invoker;
		pachouli->tag["JingyueUse"] = data;
		CardUseStruct use = data.value<CardUseStruct>();
		foreach (ServerPlayer *p, use.to) {
			room->setPlayerFlag(p, "JingyueTarget");
		}
		bool yes = room->askForUseCard(pachouli, "@@jingyue", "@jingyue");
		foreach (ServerPlayer *p, use.to) {
			room->setPlayerFlag(p, "-JingyueTarget");
		}
		return yes;
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *pachouli = invoke->invoker;
		pachouli->loseMark("@moon", 1);
		ServerPlayer *from = invoke->preferredTarget;
		from->drawCards(1);
		CardUseStruct use = data.value<CardUseStruct>();
		foreach (ServerPlayer *p, use.to) {
			if (p->hasFlag("JingyueCanceled")) {
				room->setPlayerFlag(p, "-JingyueCanceled");
				use.nullified_list << p->objectName();
			}
		}
		data = QVariant::fromValue(use);
		return false;
	}
};

class Shengyao : public TriggerSkill
{

public:
	Shengyao() : TriggerSkill("shengyao")
	{
		events << AfterDrawInitialCards << EventPhaseStart << Damaged << MarkChanged;
		frequency = Compulsory;
	}

	int getAllElements(ServerPlayer *pachouli) const
	{
		QStringList marks;
		int sum = 0;
		marks << "@fire" << "@water" << "@wood" << "@gold" << "@earth" << "@sun" << "@moon";
		for (int i = 0; i < marks.length(); i++) {
			sum += pachouli->getMark(marks.at(i));
		}
		return sum;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		ServerPlayer *pachouli;
		if (event == AfterDrawInitialCards) {
			DrawNCardsStruct draw = data.value<DrawNCardsStruct>();
            pachouli = draw.player;
			if (pachouli && pachouli->isAlive() && pachouli->hasSkill(this))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, true);
		} else if (event == EventPhaseStart) {
			pachouli = data.value<ServerPlayer *>();
			if (pachouli && pachouli->isAlive() && pachouli->hasSkill(this) && pachouli->getPhase() == Player::RoundStart
				&& getAllElements(pachouli) < 4)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, true);
		} else if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
            pachouli = damage.to;
			if (pachouli && pachouli->isAlive() && pachouli->hasSkill(this) && damage.damage > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, true);
		} else if (event == MarkChanged) {
			MarkChangeStruct change = data.value<MarkChangeStruct>();
			pachouli = change.player;
			QStringList elements;
			elements << "@fire" << "@water" << "@wood" << "@gold" << "@earth" << "@sun" << "@moon";
			if (pachouli && pachouli->isAlive() && pachouli->hasSkill(this) && elements.contains(change.name) && change.num != 0
				&& (change.num == pachouli->getMark(change.name) || pachouli->getMark(change.name) == 0
				|| (change.num > 0 && change.name == "@gold")))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *pachouli = invoke->invoker;
		if (event == AfterDrawInitialCards) {
			room->sendCompulsoryTriggerLog(pachouli, objectName());
			for (int i = 0; i < 2; i++) {
				QString element = room->askForChoice(pachouli, objectName(), "@fire+@water+@wood+@gold+@earth");
				pachouli->gainMark(element, 1);
			}
		} else if (event == MarkChanged) {
			MarkChangeStruct change = data.value<MarkChangeStruct>();
			QString skill_name;
			if (change.name == "@fire") {
				skill_name = "ranhui";
			} else if (change.name == "@water") {
				skill_name = "huzang";
			} else if (change.name == "@wood") {
				skill_name = "jiaodi";
			} else if (change.name == "@gold") {
				skill_name = "dianjin";
			} else if (change.name == "@earth") {
				skill_name = "zhenlei";
			} else if (change.name == "@sun") {
				skill_name = "huangyan";
			} else if (change.name == "@moon") {
				skill_name = "jingyue";
			} else {
				skill_name = "";
			}
			if (skill_name == "")
				return false;
			
			if (change.num == pachouli->getMark(change.name)) {
				room->acquireSkill(pachouli, skill_name);
			} else if (pachouli->getMark(change.name) == 0)
				room->detachSkillFromPlayer(pachouli, skill_name);
			
			if (change.num > 0 && change.name == "@gold") {
				room->sendCompulsoryTriggerLog(pachouli, "dianjin");
				pachouli->drawCards(change.num);
			}
		} else if (event == Damaged) {
			room->sendCompulsoryTriggerLog(pachouli, objectName());
			DamageStruct damage = data.value<DamageStruct>();
			for (int j = 0; j < damage.damage; j++) {
				QStringList elements;
				elements << "@fire" << "@water" << "@wood" << "@gold" << "@earth" << "@sun" << "@moon";
				QStringList choices;
				for (int i = 0; i < 3; i++) {
					int q = qrand() % (7 - i);
					choices << elements.at(q);
					elements.removeAt(q);
				}
				QString element = room->askForChoice(pachouli, objectName(), choices.join("+"));
				pachouli->gainMark(element, 1);
			}
		} else if (event == EventPhaseStart) {
			room->sendCompulsoryTriggerLog(pachouli, objectName());
			QStringList elements;
			elements << "@fire" << "@water" << "@wood" << "@gold" << "@earth";
			QStringList choices;
			for (int i = 0; i < 3; i++) {
				int q = qrand() % (5 - i);
				choices << elements.at(q);
				elements.removeAt(q);
			}
			QString element = room->askForChoice(pachouli, objectName(), choices.join("+"));
			pachouli->gainMark(element, 1);
		}
		return false;
	}
};

XianshiCard::XianshiCard()
{
}

bool XianshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < 3;
}

void XianshiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets) {
		p->drawCards(2);
	}
}

class XianshiVS : public ZeroCardViewAsSkill
{

public:
	XianshiVS() : ZeroCardViewAsSkill("xianshi")
	{
		response_pattern = "@@xianshi";
	}

	const Card *viewAs() const
	{
		return new XianshiCard;
	}
};

class Xianshi : public TriggerSkill
{

public:
	Xianshi() : TriggerSkill("xianshi")
	{
		frequency = Limited;
		events << EventPhaseChanging;
		view_as_skill = new XianshiVS;
		limit_mark = "@philosopher";
	}

	bool hasEveryElement(ServerPlayer *pachouli) const
	{
		return pachouli->getMark("@fire") > 0 && pachouli->getMark("@water") > 0 && pachouli->getMark("@wood") > 0
			&& pachouli->getMark("@gold") > 0 && pachouli->getMark("@earth") > 0;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		PhaseChangeStruct change = data.value<PhaseChangeStruct>();
		ServerPlayer *pachouli = change.player;
		if (pachouli && pachouli->isAlive() && pachouli->hasSkill(this) && change.from == Player::Finish
			&& hasEveryElement(pachouli) && pachouli->getMark("@philosopher") > 0)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, pachouli, pachouli, NULL, false);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
        ServerPlayer *pachouli = invoke->invoker;
		room->removePlayerMark(pachouli, "@philosopher", 1);
		room->doSuperLightbox("pachouli:xianshi", "xianshi");
		pachouli->loseMark("@fire", 1);
		pachouli->loseMark("@water", 1);
		pachouli->loseMark("@wood", 1);
		pachouli->loseMark("@gold", 1);
		pachouli->loseMark("@earth", 1);
		pachouli->gainMark("@sun", 2);
		pachouli->gainMark("@moon", 2);
		ServerPlayer *player = room->askForPlayerChosen(pachouli, room->getAlivePlayers(), objectName(), "@xianshi-recover", true, false);
		room->setPlayerProperty(player, "maxhp", player->getMaxHp() + 1);
		RecoverStruct recover;
		recover.recover = 1;
		recover.who = pachouli;
		recover.reason = objectName();
		room->recover(player, recover);
		room->askForUseCard(pachouli, "@@xianshi", "@xianshi-draw");
		pachouli->gainAnExtraTurn();
		return false;
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
		events << EventPhaseStart << Damaged << PostHpLost;
		view_as_skill = new DiaoouViewAsSkill;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *alice;
			alice = data.value<ServerPlayer *>();
			if (alice && alice->isAlive() && alice->hasSkill(this) && alice->getPhase() == Player::RoundStart) {
				foreach (ServerPlayer *p, room->getOtherPlayers(alice)) {
					if (p->getMark("@ningyou") > 0)
						room->setPlayerMark(p, "@ningyou", 0);
				}
			}
		}
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		ServerPlayer *alice = room->findPlayerBySkillName(objectName());
		if (alice && alice->isAlive() && alice->hasSkill(this)) {
			if (event == EventPhaseStart) {
				if (alice->getPhase() == Player::Finish) 
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, alice, alice, NULL, false);
			}
			else if (event == Damaged) {
				DamageStruct damage = data.value<DamageStruct>();
				if (alice == damage.to && !alice->isNude()) {
					foreach(ServerPlayer *p, room->getOtherPlayers(alice)) {
						if (p->getMark("@ningyou") > 0)
							return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, alice, alice, NULL, false, p);
					}
				}
			} else if (event == PostHpLost) {
				HpLostStruct lost = data.value<HpLostStruct>();
				if (alice == lost.player && !alice->isNude()) {
					foreach (ServerPlayer *p, room->getOtherPlayers(alice)) {
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
			return room->askForUseCard(alice, "@@diaoou", "@diaoou-choose-target");
		else if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			return room->askForCard(alice, "..", QString("@diaoou-damage-ask:%1::%2").arg(invoke->preferredTarget->objectName()).arg(QString::number(damage.damage)), data, objectName());
		} else if (event == PostHpLost) {
			HpLostStruct lost = data.value<HpLostStruct>();
			return room->askForCard(alice, "..", QString("@diaoou-hplost-ask:%1::%2").arg(invoke->preferredTarget->objectName()).arg(QString::number(lost.num)), data, objectName());
		}
		return true;
	}
	
	bool effect(TriggerEvent triggerevent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *alice = invoke->invoker;
		if (triggerevent == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
            foreach (ServerPlayer *p, room->getOtherPlayers(alice)) {
				if (p->getMark("@ningyou") > 0) {
					room->damage(DamageStruct(objectName(), alice, p, damage.damage, damage.nature));
					break;
				}
			}
		} else if (triggerevent == PostHpLost) {
			HpLostStruct lost = data.value<HpLostStruct>();
			foreach (ServerPlayer *p, room->getOtherPlayers(alice)) {
				if (p->getMark("@ningyou") > 0) {
					room->loseHp(p, lost.num);
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

KaihaiCard::KaihaiCard()
{
}

bool KaihaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->getHandcardNum() > to_select->getHp();
}

void KaihaiCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *to = effect.to;
	Room *room = to->getRoom();
	int delta = to->getHandcardNum() - to->getHp();
	room->askForDiscard(to, "kaihai", delta, delta, false, false, QString("@kaihai-discard:%1").arg(QString::number(delta)));
}

class KaihaiVS : public ZeroCardViewAsSkill
{

public:
	KaihaiVS() : ZeroCardViewAsSkill("kaihai")
	{
		response_pattern = "@@kaihai";
	}

	const Card *viewAs() const
	{
		return new KaihaiCard;
	}
};

class Kaihai : public TriggerSkill
{
	
public:
	Kaihai() : TriggerSkill("kaihai")
	{
		events << Damaged;
		view_as_skill = new KaihaiVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *sanae = damage.to;
		if (sanae && sanae->isAlive() && sanae->hasSkill(this))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sanae, sanae, NULL, false);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@kaihai", "@kaihai-target-1");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *sanae = invoke->invoker;
		QList<ServerPlayer *> targets;
		foreach (ServerPlayer *p, room->getAlivePlayers()) {
			if (p->getHandcardNum() < p->getLostHp())
				targets << p;
		}
		if (!targets.isEmpty()) {
	        ServerPlayer *target = room->askForPlayerChosen(invoke->invoker, targets, objectName(), "@kaihai-target-2", true, false);
	        if (target) {
	        	room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, sanae->objectName(), target->objectName());
	        	target->drawCards(target->getLostHp() - target->getHandcardNum());
	        }
	    }
	    return false;
	}
};

class Qiji : public TriggerSkill
{
	
public:
	Qiji() : TriggerSkill("qiji$")
	{
		events << FinishJudge;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		JudgeStruct *judge = data.value<JudgeStruct *>();
		ServerPlayer *sanae = room->findPlayerBySkillName(objectName());
		ServerPlayer *player = judge->who;
		const Card *card = judge->card;
		if (sanae && sanae->isAlive() && sanae->hasLordSkill(this) && player && player->isAlive() && player != sanae
			&& player->getKingdom() == "moriya" && card->isRed()) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sanae, player, NULL, false, sanae);
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
		sanae->drawCards(1);
		return false;
	}
};

YuzhuCard::YuzhuCard()
{
	target_fixed = true;
}

void YuzhuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	room->addPlayerMark(source, "yuzhu", 1);
	JudgeStruct judge;
	judge.who = source;
	judge.reason = "yuzhu";
	judge.pattern = ".";
	QList<int> onbashiris = source->getPile("onbashiri");
	QStringList suits;
	suits << "heart" << "diamond" << "spade" << "club";
	foreach (int id, onbashiris) {
		const Card *c = Sanguosha->getCard(id);
		if (suits.contains(c->getSuitString()))
			suits.removeOne(c->getSuitString());
	}
	if (!suits.isEmpty()) {
		judge.pattern += "|" + suits.join(",");
	}
	if (judge.pattern == ".") {
		judge.good = false;
	} else {
		judge.good = true;
	}
	judge.good = true;
	room->judge(judge);
	foreach (int id, onbashiris) {
		const Card *c = Sanguosha->getCard(id);
		if (c->getSuit() == judge.card->getSuit()) return;
	}
	source->addToPile("onbashiri", judge.card, true);
}

class Yuzhu : public ZeroCardViewAsSkill
{
	
public:
	Yuzhu() : ZeroCardViewAsSkill("yuzhu")
	{
		
	}
	
	virtual bool isEnabledAtPlay(const Player *player) const
	{
		return player->getMark("yuzhu") < 2 && player->getPile("onbashiri").length() < 4;
	}
	
	virtual const Card *viewAs() const
	{
		return new YuzhuCard;
	}
};

class YuzhuAdd : public TriggerSkill
{

public:
	YuzhuAdd() : TriggerSkill("#yuzhu-add")
	{
		events << EventPhaseStart << EventPhaseChanging;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *kanako = change.player;
			if (kanako && kanako->isAlive() && kanako->getMark("yuzhu") > 0 && change.to == Player::NotActive)
				room->setPlayerMark(kanako, "yuzhu", 0);
		}
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *kanako = data.value<ServerPlayer *>();
			if (kanako && kanako->isAlive() && kanako->hasSkill(this) && !kanako->getPile("onbashiri").isEmpty()
				&& kanako->getPhase() == Player::Finish) {
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kanako, kanako, NULL, false);
			}
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart)
			return room->askForSkillInvoke(invoke->invoker, "yuzhu", data);
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *kanako = invoke->invoker;
			QList<int> card_ids = kanako->getPile("onbashiri"), enabled = card_ids, disabled;
			int id = 0;
			DummyCard *dummy = new DummyCard;
			while (id >= 0 && !enabled.isEmpty()) {
				room->fillAG(card_ids, kanako, disabled);
				id = room->askForAG(kanako, enabled, true, objectName());
				if (id > 0) {
					dummy->addSubcard(id);
					enabled.removeOne(id);
					disabled << id;
				}
				room->clearAG(kanako);
			}
			int n = dummy->getSubcards().length();
			room->throwCard(dummy, kanako, NULL, false);
			if (n == 1) {
				ServerPlayer *drawer = room->askForPlayerChosen(kanako, room->getAlivePlayers(), "yuzhu-1", "@yuzhu-draw");
				room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, kanako->objectName(), drawer->objectName());
				drawer->drawCards(1);
			} else if (n == 2) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *p, room->getAlivePlayers()) {
					if (p->isWounded())
						targets << p;
				}
				if (!targets.isEmpty()) {
					ServerPlayer *recoverer = room->askForPlayerChosen(kanako, targets, "yuzhu-2", "@yuzhu-recover");
					room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, kanako->objectName(), recoverer->objectName());
					RecoverStruct recover;
					recover.who = kanako;
					recover.reason = "yuzhu";
					recover.recover = 1;
					room->recover(recoverer, recover);
				}
			} else if (n == 3) {
				ServerPlayer *turner = room->askForPlayerChosen(kanako, room->getAlivePlayers(), "yuzhu-3", "@yuzhu-turnover");
				room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, kanako->objectName(), turner->objectName());
				turner->turnOver();
			} else if (n == 4) {
				ServerPlayer *damager = room->askForPlayerChosen(kanako, room->getAlivePlayers(), "yuzhu-4", "@yuzhu-damage");
				room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, kanako->objectName(), damager->objectName());
				room->damage(DamageStruct("yuzhu", kanako, damager, 2, DamageStruct::Thunder));
			}
		}
		return false;
	}
};

class Jinlun : public TriggerSkill
{
	
public:
	Jinlun() : TriggerSkill("jinlun")
	{
		frequency = Compulsory;
		events << DamageInflicted << BeforeCardsMove;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == DamageInflicted) {
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
		if (event == DamageInflicted) {
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

SuiwaCard::SuiwaCard()
{
}

bool SuiwaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < Self->getLostHp();
}

void SuiwaCard::use(Room *, ServerPlayer *, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets) {
		p->drawCards(1);
	}
}

class SuiwaVS : public ZeroCardViewAsSkill
{

public:
	SuiwaVS() : ZeroCardViewAsSkill("suiwa")
	{
		response_pattern = "@@suiwa";
	}

	const Card *viewAs() const
	{
		return new SuiwaCard;
	}
};

class Suiwa : public MasochismSkill
{
	
public:
	Suiwa() : MasochismSkill("suiwa")
	{
		view_as_skill = new SuiwaVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *suwako = damage.to;
        if (suwako && suwako->isAlive() && suwako->hasSkill(this) && damage.damage > 0) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, suwako, suwako, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
    void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
	{
		ServerPlayer *suwako = invoke->invoker;
		JudgeStruct judge;
		judge.who = suwako;
		judge.reason = objectName();
		judge.pattern = ".";
		judge.good = true;
		room->judge(judge);
		if (judge.card->isRed() && suwako->isWounded()) {
			room->askForUseCard(suwako, "@@suiwa", "@suiwa-draw", 1);
		} else if (judge.card->isBlack()) {
			QList<ServerPlayer *> targets, targets2;
			foreach (ServerPlayer *p, room->getAlivePlayers()) {
				QList<const Card *> field_cards = p->getCards("ej");
				if (!field_cards.isEmpty()) {
					foreach (const Card *c, field_cards) {
						foreach (ServerPlayer *p2, room->getOtherPlayers(p)) {
							if (c->isKindOf("Weapon") && !p2->getWeapon()) {
								targets << p;
								break;
							}
							if (c->isKindOf("Armor") && !p2->getArmor()) {
								targets << p;
								break;
							}
							if (c->isKindOf("OffensiveHorse") && !p2->getOffensiveHorse()) {
								targets << p;
								break;
							}
							if (c->isKindOf("DefensiveHorse") && !p2->getDefensiveHorse()) {
								targets << p;
								break;
							}
							if (c->isKindOf("Treasure") && !p2->getTreasure()) {
								targets << p;
								break;
							}
							if (c->isKindOf("DelayedTrick")) {
								QStringList delays;
								foreach (const Card *c2, p2->getCards("j")) {
									delays << c2->objectName();
								}
								if (!delays.contains(c->objectName())) {
									targets << p;
									break;
								}
							}
						}
					}
				}
			}
			if (targets.isEmpty()) return;
			ServerPlayer *target = room->askForPlayerChosen(suwako, targets, "suiwa-1", "@suiwa-target-1", true);
			if (target) {
				QList<const Card *> field_cards = target->getCards("ej");
				QList<int> card_ids, enabled, disabled;
				if (!field_cards.isEmpty()) {
					foreach (const Card *c, field_cards) {
						card_ids << c->getEffectiveId();
						foreach (ServerPlayer *p2, room->getOtherPlayers(target)) {
							if (c->isKindOf("Weapon") && !p2->getWeapon()) {
								enabled << c->getEffectiveId();
								break;
							}
							if (c->isKindOf("Armor") && !p2->getArmor()) {
								enabled << c->getEffectiveId();
								break;
							}
							if (c->isKindOf("OffensiveHorse") && !p2->getOffensiveHorse()) {
								enabled << c->getEffectiveId();
								break;
							}
							if (c->isKindOf("DefensiveHorse") && !p2->getDefensiveHorse()) {
								enabled << c->getEffectiveId();
								break;
							}
							if (c->isKindOf("Treasure") && !p2->getTreasure()) {
								enabled << c->getEffectiveId();
								break;
							}
							if (c->isKindOf("DelayedTrick")) {
								QStringList delays;
								foreach (const Card *c2, p2->getCards("j")) {
									delays << c2->objectName();
								}
								if (!delays.contains(c->objectName())) {
									enabled << c->getEffectiveId();
									break;
								}
							}
						}
					}
					if (!enabled.isEmpty()) {
						room->fillAG(card_ids, suwako, disabled);
						int card_id = room->askForAG(suwako, enabled, true, objectName());
						room->clearAG(suwako);
						if (card_id > 0) {
							QList<ServerPlayer *> targets2;
							const Card *card = Sanguosha->getCard(card_id);
							foreach (ServerPlayer *p2, room->getOtherPlayers(target)) {
								if (card->isKindOf("Weapon") && !p2->getWeapon()) {
									targets2 << p2;
								}
								if (card->isKindOf("Armor") && !p2->getArmor()) {
									targets2 << p2;
								}
								if (card->isKindOf("OffensiveHorse") && !p2->getOffensiveHorse()) {
									targets2 << p2;
								}
								if (card->isKindOf("DefensiveHorse") && !p2->getDefensiveHorse()) {
									targets2 << p2;
								}
								if (card->isKindOf("Treasure") && !p2->getTreasure()) {
									targets2 << p2;
								}
								if (card->isKindOf("DelayedTrick")) {
									QStringList delays;
									foreach (const Card *c2, p2->getCards("j")) {
										delays << c2->objectName();
									}
									if (!delays.contains(card->objectName())) {
										targets2 << p2;
									}
								}
							}
							if (targets2.isEmpty()) return;
							ServerPlayer *target2 = room->askForPlayerChosen(suwako, targets2, "suiwa-2", "@suiwa-target-2", true);
							if (target2) {
								room->moveCardTo(card, target, target2, Player::PlaceEquip,
									CardMoveReason(CardMoveReason::S_REASON_TRANSFER, target->objectName(), objectName(), QString()));
							}
						}
					}
				}
			}
		}
	}
};

class XinyanVS : public ZeroCardViewAsSkill
{

public:
	XinyanVS() : ZeroCardViewAsSkill("xinyan")
	{
		response_pattern = "@@xinyan";
	}

	const Card *viewAs() const
	{
		Card *mind_reading = new MindReading(Card::NoSuit, 0);
		mind_reading->setSkillName(objectName());
		return mind_reading;
	}
};

class Xinyan : public TriggerSkill
{

public:
	Xinyan() : TriggerSkill("xinyan")
	{
		events << EventPhaseStart << Damaged;
		view_as_skill = new XinyanVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		MindReading *mind_reading = new MindReading(Card::NoSuit, 0);
		mind_reading->deleteLater();
		if (event == EventPhaseStart) {
			ServerPlayer *satori = data.value<ServerPlayer *>();
			if (satori && satori->isAlive() && satori->hasSkill(this) && satori->getPhase() == Player::Play) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *p, room->getOtherPlayers(satori)) {
					if (mind_reading->targetFilter(QList<const Player *>(), p, satori) && !room->isProhibited(satori, p, mind_reading))
						targets << p;
				}
				if (!targets.isEmpty())
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, satori, satori, targets, false);
			}
		} else if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *satori = damage.to;
			if (satori && satori->isAlive() && satori->hasSkill(this) && damage.damage > 0) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *p, room->getOtherPlayers(satori)) {
					if (mind_reading->targetFilter(QList<const Player *>(), p, satori) && !room->isProhibited(satori, p, mind_reading))
						targets << p;
				}
				if (!targets.isEmpty())
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, satori, satori, targets, false);
			}
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *satori = invoke->invoker;
		if (event == EventPhaseStart) {
			return room->askForUseCard(satori, "@@xinyan", "@xinyan-use");
		} else if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			for (int i = 0; i < damage.damage; i++) {
				room->askForUseCard(satori, "@@xinyan", "@xinyan-use");
			}
			return true;
		}
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

class FangyingVS : public OneCardViewAsSkill
{

public:
	FangyingVS() : OneCardViewAsSkill("fangying")
	{
		filter_pattern = ".|.|.|hand";
		response_pattern = "@@fangying";
	}

	const Card *viewAs(const Card *originalCard) const
	{
		QString card_str = Self->property("fangying_card").toString();
		Card *card = Sanguosha->cloneCard(card_str);
        card->addSubcard(originalCard);
		card->setSkillName(objectName());
		return card;
	}
};

class Fangying : public TriggerSkill
{

public:
	Fangying() : TriggerSkill("fangying")
	{
		events << EventPhaseEnd << CardUsed << EventPhaseChanging;
		view_as_skill = new FangyingVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardUsed) {
			ServerPlayer *satori = room->findPlayerBySkillName(objectName());
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *from = use.from;
			if (satori && satori->isAlive() && satori != from && from->getPhase() == Player::Play) {
				const Card *card = use.card;
				if (card->isKindOf("Slash") || card->isKindOf("Peach") || card->isKindOf("Analeptic") || (card->isNDTrick()
					&& !card->isKindOf("Nullification"))) {
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, satori, satori, NULL, true);
				}
			}
		} else if (event == EventPhaseEnd) {
			ServerPlayer *player = data.value<ServerPlayer *>();
			ServerPlayer *satori = room->findPlayerBySkillName(objectName());
			if (satori && satori->isAlive() && player && player->isAlive() && player->getPhase() == Player::Play && satori != player
				&& !satori->isKongcheng() && player->getHp() > satori->getHp()) {
				QVariant first_card_data = satori->tag["FangyingFirstCard"];
				QVariant last_card_data = satori->tag["FangyingLastCard"];
				bool can_first = false, can_last = false;
                if (first_card_data != NULL) {
					QString first_card_str = first_card_data.toString();
					if (first_card_str != "") {
						const Card *first_card = Sanguosha->cloneCard(first_card_str, Card::SuitToBeDecided, -1);
						if (first_card->targetFixed()) {
							if (first_card->isAvailable(satori)) {
								can_first = true;
							}
						} else if (first_card->isAvailable(satori)) {
							foreach (ServerPlayer *p, room->getAlivePlayers()) {
								if (first_card->targetFilter(QList<const Player *>(), p, satori) && !room->isProhibited(satori, p, first_card)) {
									can_first = true;
									break;
								}
							}
						}
					}
				}
                if (last_card_data != NULL) {
					QString last_card_str = last_card_data.toString();
					if (last_card_str != "") {
						const Card *last_card = Sanguosha->cloneCard(last_card_str, Card::SuitToBeDecided, -1);
						if (last_card->targetFixed()) {
							if (last_card->isAvailable(satori)) {
								can_last = true;
							}
						} else if (last_card->isAvailable(satori)) {
							foreach (ServerPlayer *p, room->getAlivePlayers()) {
								if (last_card->targetFilter(QList<const Player *>(), p, satori) && !room->isProhibited(satori, p, last_card)) {
									can_last = true;
									break;
								}
							}
						}
					}
				}
				if (can_first || can_last)
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, satori, satori, NULL, true);
			}
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *player = change.player;
			ServerPlayer *satori = room->findPlayerBySkillName(objectName());
			if (satori && satori->isAlive() && player && change.to == Player::NotActive) {
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, satori, satori, NULL, true);
			}
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *satori = invoke->invoker;
		if (event == EventPhaseEnd) {
			QVariant first_card_data = satori->tag["FangyingFirstCard"];
			QVariant last_card_data = satori->tag["FangyingLastCard"];
			QString first_card_str = "", last_card_str = "";
			if (first_card_data != NULL) {
				first_card_str = first_card_data.toString();
			}
			if (last_card_data != NULL) {
				last_card_str = last_card_data.toString();
			}
			QStringList choices;
			if (first_card_str != "") {
				const Card *first_card = Sanguosha->cloneCard(first_card_str, Card::SuitToBeDecided, -1);
				if (first_card->targetFixed()) {
					if (first_card->isAvailable(satori)) {
						choices << "FYFirst";
					}
				} else if (first_card->isAvailable(satori)) {
					foreach (ServerPlayer *p, room->getAlivePlayers()) {
						if (first_card->targetFilter(QList<const Player *>(), p, satori) && !room->isProhibited(satori, p, first_card)) {
							choices << "FYFirst";
							break;
						}
					}
				}
			}
			if (last_card_str != "") {
				const Card *last_card = Sanguosha->cloneCard(last_card_str, Card::SuitToBeDecided, -1);
				if (last_card->targetFixed()) {
					if (last_card->isAvailable(satori)) {
						choices << "FYLast";
					}
				} else if (last_card->isAvailable(satori)) {
					foreach (ServerPlayer *p, room->getAlivePlayers()) {
						if (last_card->targetFilter(QList<const Player *>(), p, satori) && !room->isProhibited(satori, p, last_card)) {
							choices << "FYLast";
							break;
						}
					}
				}
			}
			if (choices.isEmpty())
				return false;
			choices << "Cancel";
			QString choice = room->askForChoice(satori, objectName(), choices.join("+"));
			if (choice == "Cancel") {
				return false;
			} else if (choice == "FYFirst") {
				room->setPlayerProperty(satori, "fangying_card", first_card_str);
				return room->askForUseCard(satori, "@@fangying", QString("@fangying:%1").arg(first_card_str));
			} else if (choice == "FYLast") {
				room->setPlayerProperty(satori, "fangying_card", last_card_str);
				return room->askForUseCard(satori, "@@fangying", QString("@fangying:%1").arg(last_card_str));
			}
		}
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *satori = invoke->invoker;
		if (event == CardUsed) {
			CardUseStruct use = data.value<CardUseStruct>();
			const Card *card = use.card;
			QVariant first_card_data = satori->tag["FangyingFirstCard"];
            if (first_card_data == NULL || first_card_data.toString() == "") {
				satori->tag["FangyingFirstCard"] = QVariant(card->objectName());
			}
			satori->tag["FangyingLastCard"] = QVariant(card->objectName());
		} else if (event == EventPhaseChanging) {
			satori->tag["FangyingFirstCard"] = QVariant("");
			satori->tag["FangyingLastCard"] = QVariant("");
		}
		return false;
	}
};

/* class CeshiVS : public OneCardViewAsSkill
{

public:
	CeshiVS() : OneCardViewAsSkill("ceshi")
	{
		filter_pattern = ".|.|.|hand";
	}

	bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

	bool isEnabledAtResponse(const Player *, const QString &pattern) const
	{
		return pattern.startsWith("@@ceshi-");
	}

	const Card *viewAs(const Card *originalCard) const
	{
		Slash *slash = new Slash(Card::SuitToBeDecided, -1);
		slash->addSubcard(originalCard);
		slash->setSkillName(objectName());
		return slash;
	}
};

class Ceshi : public TriggerSkill
{

public:
	Ceshi() : TriggerSkill("ceshi")
	{
		events << EventPhaseStart;
		view_as_skill = new CeshiVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *player = data.value<ServerPlayer *>();
		if (player && player->isAlive() && player->hasSkill(this) && player->getPhase() == Player::RoundStart && !player->isKongcheng())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player, NULL, true);
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
	{
		ServerPlayer *player = invoke->invoker;
		room->askForUseCard(player, "@@ceshi-2", "@ceshi");
		return false;
	}
}; */

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

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *koishi = change.player;
			if (koishi && koishi->isAlive() && koishi->hasSkill(this) && change.to == Player::NotActive
				&& koishi->hasFlag("CannotDuannian"))
				room->setPlayerFlag(koishi, "-CannotDuannian");
		}
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		ServerPlayer *koishi = room->getCurrent();
		if (koishi->hasSkill(this)) {
			if (event == EventPhaseChanging) {
				PhaseChangeStruct change = data.value<PhaseChangeStruct>();
				if (change.to == Player::Play && !koishi->hasFlag("CannotDuannian"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, koishi, koishi, NULL, false);
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
		if (event == EventPhaseStart) {
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
	
	bool noLimit(const Player *koishi, const Card *) const
	{
		return koishi->hasFlag("CannotDuannian");
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

class Zhangqi : public TriggerSkill
{
	
public:
	Zhangqi() : TriggerSkill("zhangqi")
	{
		events << DamageCaused << ConfirmDamage << EventPhaseChanging;
	}

	void record(TriggerEvent event, Room *room, QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *yamame = change.player;
			if (yamame && yamame->isAlive() && yamame->hasSkill(this) && change.to == Player::RoundStart) {
				foreach (ServerPlayer *p, room->getOtherPlayers(yamame)) {
					room->setPlayerFlag(p, "-ZhangqiFlag");
				}
			}
		}
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == DamageCaused) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *yamame = damage.from;
			ServerPlayer *to = damage.to;
			if (yamame && yamame->isAlive() && yamame->hasSkill(this) && to && to->isAlive() && !to->hasFlag("ZhangqiFlag")) {
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yamame, yamame, NULL, false, to);
			}
		} else if (event == ConfirmDamage) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *yamame = room->findPlayerBySkillName(objectName());
			ServerPlayer *to = damage.to;
			if (yamame && yamame->isAlive() && to && to->isAlive() && to->hasFlag("ZhangqiFlag"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yamame, yamame, NULL, true, to);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yamame = invoke->invoker;
		if (event == DamageCaused)
			return room->askForSkillInvoke(yamame, objectName(), data);
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yamame = invoke->invoker;
		ServerPlayer *to = invoke->preferredTarget;
		if (event == DamageCaused) {
			room->setPlayerFlag(to, "ZhangqiFlag");
			return true;
		} else if (event == ConfirmDamage) {
			DamageStruct damage = data.value<DamageStruct>();
			damage.damage++;
			damage.nature = DamageStruct::Fire;
			data = QVariant::fromValue(damage);
			room->touhouLogmessage("#ZhangqiDamage", yamame, objectName(), QList<ServerPlayer *>() << to, QString::number(damage.damage));
		}
		return false;
	}
};

THZhusiCard::THZhusiCard()
{
}

bool THZhusiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self;
}

void THZhusiCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *yamame = effect.from;
	ServerPlayer *to = effect.to;
	Room *room = yamame->getRoom();
	if (!to->isChained()) {
		room->setPlayerProperty(to, "chained", true);
		to->drawCards(1);
	} else if (to->isChained()) {
		room->setPlayerProperty(to, "chained", false);
		int n = 0;
		while (n < 2 && !to->isNude()) {
			int id = room->askForCardChosen(yamame, to, "he", "thzhusi");
			room->throwCard(id, to, yamame);
			n++;
		}
	}
}

class THZhusiVS : public ZeroCardViewAsSkill
{

public:
	THZhusiVS() : ZeroCardViewAsSkill("thzhusi")
	{
		response_pattern = "@@thzhusi";
	}

	const Card *viewAs() const
	{
		return new THZhusiCard;
	}
};

class THZhusi : public TriggerSkill
{
	
public:
	THZhusi() : TriggerSkill("thzhusi")
	{
		events << HpRecover << CardsMoveOneTime;
		view_as_skill = new THZhusiVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == HpRecover) {
			RecoverStruct recover = data.value<RecoverStruct>();
			ServerPlayer *yamame = recover.to;
			if (yamame && yamame->isAlive() && yamame->hasSkill(this))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yamame, yamame, NULL, false);
		} else if (event == CardsMoveOneTime) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			ServerPlayer *yamame = qobject_cast<ServerPlayer *>(move.to);
			if (yamame && yamame->isAlive() && yamame->hasSkill(this) && move.from_places.contains(Player::DrawPile)
				&& move.to_place == Player::PlaceHand && !move.card_ids.isEmpty() && yamame->getPhase() != Player::Draw
				&& !room->getTag("FirstRound").toBool())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yamame, yamame, NULL, false);
		}
		/*ServerPlayer *yamame = data.value<ServerPlayer *>();
		if (yamame && yamame->isAlive() && yamame->hasSkill(this) && yamame->getPhase() == Player::Finish)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yamame, yamame, NULL, false);*/
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
	{
		return room->askForUseCard(invoke->invoker, "@@thzhusi", "@thzhusi");
	}
	
	bool effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &) const
	{
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
			QList<ServerPlayer *> targets;
			foreach(ServerPlayer *p, use.to) {
				if (p->getHp() <= shizuha->getHp() && !p->isNude())
					targets << p;
			}
			if (!targets.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, shizuha, shizuha, targets, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *shizuha = invoke->invoker;
		QList<ServerPlayer *> targets = invoke->targets;
		room->sendCompulsoryTriggerLog(shizuha, objectName());
		foreach (ServerPlayer *p, targets) {
			room->askForDiscard(p, objectName(), 1, 1, false, true, QString("@canye-discard:%1").arg(shizuha->objectName()));
		}
		return false;
	}
};

class Diaofeng : public TriggerSkill
{
	
public:
	Diaofeng() : TriggerSkill("diaofeng")
	{
		events << TargetConfirmed;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *shizuha = room->findPlayerBySkillName(objectName());
		if (shizuha && shizuha->isAlive() && use.to.length() > 1 && use.from && use.from->isAlive()
			&& !use.card->isKindOf("SkillCard")) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, shizuha, shizuha, NULL, false, use.from);
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
			ServerPlayer *from = invoke->preferredTarget;
			QString choice;
			if (from->isKongcheng())
				choice = "DFDraw";
			else
				choice = room->askForChoice(shizuha, objectName(), "DFDraw+DFDiscard", data);
			if (choice == "DFDraw")
				from->drawCards(1);
			else if (choice == "DFDiscard") {
				room->askForDiscard(from, objectName(), 1, 1, false, false, QString("@diaofeng-discard:%1").arg(shizuha->objectName()));
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
	reimu->addSkill(new Guayu);
	related_skills.insertMulti("fengmo", "#fengmoadd");
	
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
	remilia->addSkill(new Shiling);
	remilia->addSkill(new Xuewang);
	remilia->addSkill(new XuewangMC);
	related_skills.insertMulti("xuewang", "#xuewang-maxcards");
	
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
	nitori->addSkill(new Linshang);
	nitori->addSkill(new Jiexun);
	
	General *pachouli = new General(this, "pachouli", "hakurei", 3, false);
	pachouli->addSkill(new Shengyao);
	pachouli->addSkill(new Xianshi);
	pachouli->addSkill(new DianjinMaxCards);
	pachouli->addSkill(new DianjinClear);
	pachouli->addSkill(new ZhenleiAdd);
	related_skills.insertMulti("dianjin", "#dianjin-maxcards");
	related_skills.insertMulti("dianjin", "#dianjin-clear");
	related_skills.insertMulti("zhenlei", "#zhenlei-add");
	
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
	sanae->addSkill(new Kaihai);
	sanae->addSkill(new Qiji);
	
	General *kanako = new General(this, "kanako", "moriya", 4, false);
	kanako->addSkill(new Yuzhu);
	kanako->addSkill(new YuzhuAdd);
	related_skills.insertMulti("yuzhu", "#yuzhu-add");
	
	General *suwako = new General(this, "suwako", "moriya", 3, false);
	suwako->addSkill(new Jinlun);
	suwako->addSkill(new Suiwa);
	
	General *satori = new General(this, "satori", "moriya", 3, false);
	satori->addSkill(new Xinyan);
	satori->addSkill(new Fangying);
	
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
	yamame->addSkill(new THZhusi);
	
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
	addMetaObject<ShantouCard>();
	addMetaObject<DaosheCard>();
	addMetaObject<HeiyanCard>();
	addMetaObject<RanhuiCard>();
	addMetaObject<DianjinCard>();
	addMetaObject<HuangyanCard>();
	addMetaObject<JingyueCard>();
	addMetaObject<XianshiCard>();
	addMetaObject<DiaoouCard>();
	addMetaObject<AnjiCard>();
	addMetaObject<YaofengCard>();
	addMetaObject<KaihaiCard>();
	addMetaObject<YuzhuCard>();
	addMetaObject<SuiwaCard>();
	addMetaObject<DuannianCard>();
	addMetaObject<HunquCard>();
	addMetaObject<THZhusiCard>();
    addMetaObject<XianboCard>();
	addMetaObject<HuanniCard>();
	addMetaObject<CitanCard>();

	skills << new Ranhui << new Huzang << new Jiaodi << new Dianjin << new Zhenlei << new Huangyan << new Jingyue;
}

ADD_PACKAGE(THStandard)
