#include "thvenus.h"
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
#include "thsoul.h"
#include "thglimpse.h"

FanniCard::FanniCard()
{

}

bool FanniCard::targetFilter(const QList<const Player *> &selected, const Player *to_select, const Player *Self) const
{
	return selected.length() < qMax(1, Self->getHandcardNum()) && to_select->getHandcardNum() > Self->getHandcardNum();
}

void FanniCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets) {
		int id = room->askForCardChosen(source, p, "he", "fanni");
		const Card *c = Sanguosha->getCard(id);
		source->obtainCard(c, (room->getCardPlace(id) != Player::PlaceHand));
	}
	if (targets.length() > 1)
		source->turnOver();
}

class FanniVS : public ZeroCardViewAsSkill
{
	
public:
	FanniVS() : ZeroCardViewAsSkill("fanni")
	{
		response_pattern = "@@fanni";
	}
	
	const Card *viewAs() const
	{
		return new FanniCard;
	}
};

class Fanni : public TriggerSkill
{
	
public:
	Fanni() : TriggerSkill("fanni")
	{
		events << EventPhaseStart;
		view_as_skill = new FanniVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *seijya = data.value<ServerPlayer *>();
		if (seijya && seijya->isAlive() && seijya->hasSkill(this) && seijya->getPhase() == Player::Finish)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, seijya, seijya, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@fanni", "@fanni");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

XiaoanCard::XiaoanCard()
{

}

bool XiaoanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < Self->getMark("xiaoan_targets") && to_select->hasFlag("Xiaoanable");
}

void XiaoanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets)
		room->setPlayerFlag(p, "XiaoanTarget");
}

class XiaoanVS : public ZeroCardViewAsSkill
{

public:
	XiaoanVS() : ZeroCardViewAsSkill("xiaoan")
	{
		response_pattern = "@@xiaoan";
	}

	const Card *viewAs() const
	{
		return new XiaoanCard;
	}
};

class Xiaoan : public TriggerSkill
{
	
public:
	Xiaoan() : TriggerSkill("xiaoan")
	{
		events << CardsMoveOneTime;
		view_as_skill = new XiaoanVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *rumia = qobject_cast<ServerPlayer *>(move.from);
		if (rumia && rumia->isAlive() && rumia->hasSkill(this) && move.reason.m_reason == 0x33) {
			DoomNight *doom_night = new DoomNight(Card::SuitToBeDecided, 0);
			foreach (int id, move.card_ids) {
				const Card *c = Sanguosha->getCard(id);
				if (c->isBlack())
					doom_night->addSubcard(c);
			}
			if (doom_night->getSubcards().isEmpty())
				return QList<SkillInvokeDetail>();
			QList<ServerPlayer *> targets;
			foreach (ServerPlayer *p, room->getOtherPlayers(rumia)) {
                if (doom_night->targetFilter(QList<const Player *>(), p, rumia) && !room->isProhibited(rumia, p, doom_night))
					targets << p;
			}
			if (!targets.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, rumia, rumia, targets, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *rumia = invoke->invoker;
		QList<ServerPlayer *> targets = invoke->targets;
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		DoomNight *doom_night = new DoomNight(Card::SuitToBeDecided, 0);
		foreach(int id, move.card_ids) {
			const Card *c = Sanguosha->getCard(id);
			if (c->isBlack()) {
				doom_night->addSubcard(c);
			}
		}
		foreach (ServerPlayer *p, targets)
			room->setPlayerFlag(p, "Xiaoanable");
		room->setPlayerMark(rumia, "xiaoan_targets", 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, rumia, doom_night));
		rumia->tag["XiaoanTag"] = data;
		bool yes = room->askForUseCard(invoke->invoker, "@@xiaoan", "@xiaoan");
		foreach (ServerPlayer *p, room->getAlivePlayers()) {
			if (p->hasFlag("Xiaoanable"))
				room->setPlayerFlag(p, "-Xiaoanable");
		}
		room->setPlayerMark(rumia, "xiaoan_targets", 0);
		return yes;
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *rumia = invoke->invoker;
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		DoomNight *doom_night = new DoomNight(Card::SuitToBeDecided, 0);
		foreach (int id, move.card_ids) {
			const Card *c = Sanguosha->getCard(id);
			if (c->isBlack())
				doom_night->addSubcard(c);
		}
		CardUseStruct use;
		use.card = doom_night;
		use.from = rumia;
		foreach (ServerPlayer *p, room->getAlivePlayers()) {
			if (p->hasFlag("XiaoanTarget")) {
				room->setPlayerFlag(p, "-XiaoanTarget");
				use.to << p;
			}
		}
		room->useCard(use);
		return false;
	}
};

class Yueshi : public TriggerSkill
{
	
public:
	Yueshi() : TriggerSkill("yueshi")
	{
		events << DamageInflicted << EventPhaseChanging;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == DamageInflicted) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *rumia = damage.to;
			if (rumia && rumia->isAlive() && rumia->hasSkill(this) && rumia->getCards("he").length() >= damage.damage
					&& damage.from && damage.from->isAlive() && damage.from != rumia && !rumia->hasFlag("YueshiInvoked"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, rumia, rumia, NULL, false, damage.from);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *rumia = room->findPlayerBySkillName(objectName());
			if (rumia && rumia->isAlive() && rumia->hasFlag("YueshiInvoked") && change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, rumia, rumia, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == DamageInflicted)
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == DamageInflicted) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *from = invoke->preferredTarget;
			ServerPlayer *rumia = invoke->invoker;
			for (int i = 0; i < damage.damage; i++) {
				if (rumia->isNude())
					break;
				int id = room->askForCardChosen(from, rumia, "he", objectName());
				room->throwCard(id, rumia, from);
			}
			room->setPlayerFlag(rumia, "YueshiInvoked");
			return true;
		} else if (event == EventPhaseChanging) {
			ServerPlayer *rumia = invoke->invoker;
			room->setPlayerFlag(rumia, "-YueshiInvoked");
		}
		return false;
	}
};

class Tannang : public TriggerSkill
{
	
public:
	Tannang() : TriggerSkill("tannang")
	{
		events << DrawNCards;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DrawNCardsStruct q = data.value<DrawNCardsStruct>();
		ServerPlayer *nazrin = q.player;
		if (nazrin && nazrin->isAlive() && nazrin->hasSkill(this))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nazrin, nazrin, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		DrawNCardsStruct q = data.value<DrawNCardsStruct>();
		ServerPlayer *nazrin = invoke->invoker;
		q.n = 0;
		data = QVariant::fromValue(q);
		QList<int> card_ids = room->getNCards(5, false);
		QList<int> enabled = card_ids;
		QList<int> disabled;
		int card_id = 0;
		DummyCard *dummy = new DummyCard;
		bool has_trick = false;
		while (card_id != -1 && !enabled.isEmpty()) {
			room->fillAG(card_ids, nazrin, disabled);
			card_id = room->askForAG(nazrin, enabled, true, objectName());
			if (card_id == -1) {
				room->clearAG(nazrin);
				break;
			}
			enabled.removeOne(card_id);
			disabled.append(card_id);
			dummy->addSubcard(card_id);
			const Card *card = Sanguosha->getCard(card_id);
			if (card->isKindOf("TrickCard"))
				has_trick = true;
			foreach (int id, enabled) {
				const Card *c = Sanguosha->getCard(id);
				if (c->getType() != card->getType()) {
					enabled.removeOne(id);
					disabled.append(id);
				}
			}
			room->clearAG(nazrin);
		}
		if (!dummy->getSubcards().isEmpty()) {
			nazrin->obtainCard(dummy, true);
			if (has_trick)
				room->loseHp(nazrin, 1);
		}
		return false;
	}
};

LingbaiCard::LingbaiCard()
{
	
}

bool LingbaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < Self->getMark("lingbai");
}

void LingbaiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets) {
		if (p == source && p->isWounded()) {
			QString choice = room->askForChoice(p, "lingbai", "LBDraw+LBRecover");
			if (choice == "LBDraw")
				p->drawCards(1);
			else {
				RecoverStruct recover;
				recover.who = p;
				recover.recover = 1;
				recover.reason = "lingbai";
				room->recover(p, recover);
			}
		} else
			p->drawCards(1);
	}
}

class LingbaiVS : public ZeroCardViewAsSkill
{
	
public:
	LingbaiVS() : ZeroCardViewAsSkill("lingbai")
	{
		response_pattern = "@@lingbai";
	}
	
	const Card *viewAs() const
	{
		return new LingbaiCard;
	}
};

class Lingbai : public TriggerSkill
{
	
public:
	Lingbai() : TriggerSkill("lingbai")
	{
		events << CardsMoveOneTime << EventPhaseEnd << EventPhaseChanging;
		view_as_skill = new LingbaiVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardsMoveOneTime) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			ServerPlayer *nazrin = qobject_cast<ServerPlayer *>(move.from);
			if (nazrin && nazrin->isAlive() && nazrin->hasSkill(this) && move.reason.m_reason == 0x13)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nazrin, nazrin, NULL, true);
		} else if (event == EventPhaseEnd) {
			ServerPlayer *nazrin = data.value<ServerPlayer *>();
			if (nazrin && nazrin->isAlive() && nazrin->getPhase() == Player::Discard && nazrin->getMark("lingbai") > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nazrin, nazrin, NULL, false);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *nazrin = change.player;
			if (nazrin && nazrin->isAlive() && change.from == Player::Finish && nazrin->getMark("lingbai") > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nazrin, nazrin, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseEnd)
			return room->askForUseCard(invoke->invoker, "@@lingbai", "@lingbai");
		
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *nazrin = invoke->invoker;
		if (event == CardsMoveOneTime) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			int n = 0;
			foreach (int id, move.card_ids) {
				const Card *c = Sanguosha->getCard(id);
				if (!c->isKindOf("BasicCard"))
					n++;
			}
			if (n > 0)
				room->setPlayerMark(nazrin, "lingbai", n);
		} else if (event == EventPhaseChanging) {
			room->setPlayerMark(nazrin, "lingbai", 0);
		}
		return false;
	}
};

JinghunCard::JinghunCard()
{
	
}

bool JinghunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return to_select->hasFlag("ValidJinghunTarget");
}

void JinghunCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets)
		room->setPlayerFlag(p, "JinghunTarget");
}

class JinghunVS : public ZeroCardViewAsSkill
{
	
public:
	JinghunVS() : ZeroCardViewAsSkill("jinghun")
	{
		response_pattern = "@@jinghun";
	}
	
	const Card *viewAs() const
	{
		return new JinghunCard;
	}
};

class Jinghun : public TriggerSkill
{
	
public:
	Jinghun() : TriggerSkill("jinghun")
	{
		events << TargetSpecified;
		view_as_skill = new JinghunVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *shyou = use.from;
		if (shyou && shyou->isAlive() && shyou->hasSkill(this) && (use.card->isKindOf("Slash") || use.card->isNDTrick()))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, shyou, shyou, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		invoke->invoker->tag["JinghunUse"] = data;
		CardUseStruct use = data.value<CardUseStruct>();
		foreach (ServerPlayer *p, use.to)
			room->setPlayerFlag(p, "ValidJinghunTarget");
		bool yes = room->askForUseCard(invoke->invoker, "@@jinghun", "@jinghun");
		foreach (ServerPlayer *p, room->getAlivePlayers())
			if (p->hasFlag("ValidJinghunTarget"))
				room->setPlayerFlag(p, "-ValidJinghunTarget");
		return yes;
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *shyou = invoke->invoker;
		CardUseStruct use = data.value<CardUseStruct>();
		foreach (ServerPlayer *p, room->getAlivePlayers()) {
			if (p->hasFlag("JinghunTarget") && use.to.contains(p)) {
				use.nullified_list << p->objectName();
				room->setPlayerFlag(p, "-JinghunTarget");
				room->addPlayerMark(p, "@purified", 1);
				if (p->getMark("@purified") >= 4) {
					room->removePlayerMark(p, "@purified", 4);
					p->throwAllEquips();
					int discard_num = qMin(p->getHandcardNum(), 4);
					if (discard_num > 0)
						room->askForDiscard(p, objectName(), discard_num, discard_num, false, false);
					if (p->isWounded()) {
						shyou->tag["JinghunRecoverTarget"] = QVariant::fromValue(p);
						if (room->askForSkillInvoke(shyou, objectName(), QVariant("recover"))) {
							RecoverStruct recover;
							recover.who = shyou;
							recover.recover = 1;
							recover.reason = objectName();
							room->recover(p, recover);
						}
					}
				}
			}
		}
		data = QVariant::fromValue(use);
		return false;
	}
};

CaiyuanCard::CaiyuanCard()
{
	
}

bool CaiyuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->getMark("@purified") > 0;
}

void CaiyuanCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *to = effect.to;
	Room *room = to->getRoom();
	int n = to->getMark("@purified");
	to->drawCards(n);
	room->askForDiscard(to, "caiyuan", n, n, false, true);
}

class CaiyuanVS : public ZeroCardViewAsSkill
{
	
public:
	CaiyuanVS() : ZeroCardViewAsSkill("caiyuan")
	{
		response_pattern = "@@caiyuan";
	}
	
	const Card *viewAs() const
	{
		return new CaiyuanCard;
	}
};

class Caiyuan : public TriggerSkill
{
	
public:
    Caiyuan() : TriggerSkill("caiyuan")
	{
		events << EventPhaseStart;
		view_as_skill = new CaiyuanVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *shyou = data.value<ServerPlayer *>();
		if (shyou && shyou->isAlive() && shyou->hasSkill(this) && shyou->getPhase() == Player::Finish)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, shyou, shyou, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@caiyuan", "@caiyuan");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

class Guangjie : public TriggerSkill
{
	
public:
	Guangjie() : TriggerSkill("guangjie")
	{
		events << GameStart << TargetConfirming << CardFinished << EventPhaseChanging;
		//view_as_skill = new GuangjieVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == GameStart) {
			ServerPlayer *momiji = room->findPlayerBySkillName(objectName());
			if (momiji && momiji->isAlive())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, momiji, momiji, NULL, true);
		} else if (event == TargetConfirming) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *momiji = room->findPlayerBySkillName(objectName());
			if (momiji && momiji->isAlive() && momiji->getMark("guangjie") == 0 && (use.card->isKindOf("BasicCard") || use.card->isNDTrick())) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *p, room->getAlivePlayers()) {
					if (p != use.from && !use.to.contains(p) && !room->isProhibited(use.from, p, use.card)) {
						if (use.card->targetFixed()) {
							if (!use.card->isKindOf("Peach") || p->isWounded())
								targets << p;
						} else if (use.card->targetFilter(QList<const Player *>(), p, use.from))
							targets << p;
					}
				}
				if (!targets.isEmpty())
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, momiji, momiji, targets, false);
			}
		} else if (event == CardFinished) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *momiji = room->findPlayerBySkillName(objectName());
			if (!momiji || !momiji->isAlive() || !use.card->hasFlag("GuangjieCard"))
				return QList<SkillInvokeDetail>();
			foreach (ServerPlayer *p, room->getAlivePlayers())
				if (p->hasFlag("GuangjieExtraTarget"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, momiji, momiji, NULL, true, p);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *momiji = room->findPlayerBySkillName(objectName());
			if (momiji && momiji->isAlive() && change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, momiji, momiji, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *momiji = invoke->invoker;
		if (event == TargetConfirming) {
			momiji->tag["GuangjieUse"] = data;
			foreach (ServerPlayer *p, invoke->targets)
				room->setPlayerFlag(p, "GuangjieTarget");
			ServerPlayer *extra = room->askForPlayerChosen(momiji, invoke->targets, objectName(), "@guangjie-extra-target", true, true);
			foreach (ServerPlayer *p, invoke->targets)
				room->setPlayerFlag(p, "-GuangjieTarget");
			if (extra) {
				room->setPlayerFlag(extra, "GuangjieExtraTarget");
				return true;
			}
			return false;
		}
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *momiji = invoke->invoker;
		if (event == GameStart || event == EventPhaseChanging) {
			room->setPlayerMark(momiji, "guangjie", 0);
		} else if (event == TargetConfirming) {
			CardUseStruct use = data.value<CardUseStruct>();
			foreach (ServerPlayer *p, room->getAlivePlayers())
				if (p->hasFlag("GuangjieExtraTarget") && !use.to.contains(p))
					use.to.append(p);
			room->sortByActionOrder(use.to);
			room->setCardFlag(use.card, "GuangjieCard");
			data = QVariant::fromValue(use);
			room->addPlayerMark(momiji, "guangjie", 1);
			//room->touhouLogmessage("#GuangjieLimit", momiji, objectName(), QList<ServerPlayer *>(), QString::number(momiji->getMark("guangjie_max") - momiji->getMark("guangjie")));
		} else if (event == CardFinished) {
			CardUseStruct use = data.value<CardUseStruct>();
			room->clearCardFlag(use.card);
			foreach (ServerPlayer *p, room->getAlivePlayers()) {
				p->tag["GuangjieUse"] = data;
				if (p->hasFlag("GuangjieExtraTarget")) {
					room->setPlayerFlag(p, "-GuangjieExtraTarget");
					if (!use.from || !use.from->isAlive() || !momiji || !momiji->isAlive() || momiji->isNude()) {
						if (!p->isNude())
							room->askForDiscard(p, "guangjie3", 1, 1, false, true, "@guangjie-single-option:" + momiji->objectName());
					} else if (!room->askForDiscard(p, "guangjie1", 1, 1, true, true, "@guangjie-optional-discard:" + momiji->objectName())) {
						if (use.from == momiji)
							room->askForDiscard(momiji, "guangjie2", 1, 1, false, true);
						else {
							int id = room->askForCardChosen(use.from, momiji, "he", objectName());
							room->throwCard(id, momiji, use.from);
						}
					}
				}
			}
		}
		return false;
	}
};

class Langyan : public TriggerSkill
{
	
public:
	Langyan() : TriggerSkill("langyan")
	{
		events << EventPhaseEnd;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *player = data.value<ServerPlayer *>();
		ServerPlayer *momiji = room->findPlayerBySkillName(objectName());
		if (momiji && momiji->isAlive() && player->getPhase() == Player::Finish) {
			foreach (ServerPlayer *p, room->getOtherPlayers(momiji)) {
				if (p->getHandcardNum() < momiji->getHandcardNum())
					return QList<SkillInvokeDetail>();
			}
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, momiji, momiji, NULL, false, momiji);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *momiji = invoke->invoker;
		return room->askForSkillInvoke(momiji, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *current = data.value<ServerPlayer *>();
		ServerPlayer *momiji = invoke->invoker;
		ServerPlayer *player = momiji->getNextAlive();
		int n = 0;
		while (player != momiji) {
			player->tag["LangyanAlreadyNum"] = QVariant::fromValue(n);
			const Card *c = room->askForCard(player, ".|.|.|hand", "@langyan-give", data, Card::MethodNone, momiji);
			if (c) {
				room->showCard(player, c->getEffectiveId());
				momiji->obtainCard(c, true);
				if (c->isKindOf("BasicCard") && c->isRed()) {
					player->drawCards(1);
				}
				n++;
			}
			player = player->getNextAlive();
		}
		if (n == 0)
			room->loseHp(momiji, 1);
		return false;
	}
};

class Juanling : public TriggerSkill
{
	
public:
	Juanling() : TriggerSkill("juanling")
	{
		events << CardUsed << EventPhaseChanging;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardUsed) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *tojiko = use.from;
			if (tojiko && tojiko->isAlive() && tojiko->hasSkill(this) && tojiko->getPhase() == Player::Play) {
				foreach (ServerPlayer *p, use.to)
					if (p != tojiko)
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tojiko, tojiko, NULL, true);
			}
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *tojiko = change.player;
			if (tojiko && tojiko->isAlive() && tojiko->hasSkill(this) && change.from == Player::Play)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tojiko, tojiko, NULL, true);
			if (tojiko && tojiko->isAlive() && tojiko->hasSkill("leixuan") && change.from == Player::Finish
					&& (tojiko->hasFlag("LeixuanUp") || tojiko->hasFlag("LeixuanDown")))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tojiko, tojiko, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *tojiko = invoke->invoker;
		if (event == CardUsed) {
			room->addPlayerMark(tojiko, "juanling", 1);
			if (tojiko->getMark("juanling") == 1) {
				room->sendCompulsoryTriggerLog(tojiko, objectName());
				tojiko->drawCards(2);
			}
			if (tojiko->getMark("juanling") == tojiko->getMark("Global_TurnCount") && tojiko->isWounded()) {
				if (tojiko->getMark("juanling") != 1)
					room->sendCompulsoryTriggerLog(tojiko, objectName());
				RecoverStruct recover;
				recover.who = NULL;
				recover.recover = 1;
				recover.reason = objectName();
				room->recover(tojiko, recover);
			}
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			if (change.from == Player::Play)
				room->setPlayerMark(tojiko, "juanling", 0);
			else if (change.from == Player::Finish) {
				if (tojiko->hasFlag("LeixuanUp"))
					room->setPlayerFlag(tojiko, "-LeixuanUp");
				if (tojiko->hasFlag("LeixuanDown"))
					room->setPlayerFlag(tojiko, "-LeixuanDown");
			}
		}
		return false;
	}
};

class JuanlingPS : public ProhibitSkill
{
	
public:
	JuanlingPS() : ProhibitSkill("#juanling-prohibit")
	{
		
	}
	
	bool isProhibited(const Player *from, const Player *to, const Card *, const QList<const Player *> &) const
	{
		return from && from->getMark("juanling") >= from->getMark("Global_TurnCount") && from->getPhase() != Player::NotActive && from != to;
	}
};

class Leixuan : public TriggerSkill
{
	
public:
	Leixuan() : TriggerSkill("leixuan")
	{
		events << EventPhaseStart << EventPhaseEnd << CardsMoveOneTime;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *tojiko = data.value<ServerPlayer *>();
			if (tojiko && tojiko->isAlive() && tojiko->hasSkill(this) && tojiko->getPhase() == Player::Discard
					&& tojiko->getHandcardNum() > tojiko->getMaxCards())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tojiko, tojiko, NULL, false);
		} else if (event == EventPhaseEnd) {
			ServerPlayer *tojiko = data.value<ServerPlayer *>();
			if (tojiko && tojiko->isAlive() && tojiko->hasSkill(this) && tojiko->getPhase() == Player::Discard
					&& tojiko->hasFlag("LeixuanUp"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tojiko, tojiko, NULL, true);
		} else if (event == CardsMoveOneTime) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			ServerPlayer *tojiko = qobject_cast<ServerPlayer *>(move.from);
			if (tojiko && tojiko->isAlive() && tojiko->hasSkill(this) && move.reason.m_reason == 0x13) {
				if (tojiko->hasFlag("LeixuanDown")) {
					QStringList suits;
					foreach (int id, move.card_ids) {
						const Card *c = Sanguosha->getCard(id);
						if (!suits.contains(c->getSuitString()))
							suits.append(c->getSuitString());
						else
							return QList<SkillInvokeDetail>();
					}
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tojiko, tojiko, NULL, false);
				} else if (tojiko->hasFlag("LeixuanUp"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tojiko, tojiko, NULL, true);
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *tojiko = invoke->invoker;
		if (event == EventPhaseStart)
			return room->askForSkillInvoke(tojiko, objectName(), data);
		else if (event == CardsMoveOneTime) {
			if (tojiko->hasFlag("LeixuanDown")) {
				ServerPlayer *target = room->askForPlayerChosen(tojiko, room->getOtherPlayers(tojiko), objectName(), "@leixuan-damage", true, false);
				if (target) {
					tojiko->tag["LeixuanTarget"] = QVariant::fromValue(target);
					return true;
				}
			}
		}
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *tojiko = invoke->invoker;
		if (event == EventPhaseStart) {
			QString choice = room->askForChoice(tojiko, objectName(), "LeixuanUp+LeixuanDown");
			room->setPlayerFlag(tojiko, choice);
		} else if (event == CardsMoveOneTime) {
			if (tojiko->hasFlag("LeixuanUp")) {
				room->setPlayerFlag(tojiko, "-LeixuanUp");
			} else if (tojiko->hasFlag("LeixuanDown")) {
				ServerPlayer *target = tojiko->tag["LeixuanTarget"].value<ServerPlayer *>();
				if (target && target->isAlive()) {
					room->damage(DamageStruct(objectName(), tojiko, target, 1, DamageStruct::Thunder));
				}
			}
		} else if (event == EventPhaseEnd) {
			room->loseHp(tojiko, 1);
		}
		return false;
	}
};

class LeixuanMaxCards : public MaxCardsSkill
{
	
public:
	LeixuanMaxCards() : MaxCardsSkill("#leixuan-max-cards")
	{
		
	}
	
	int getExtra(const Player *target) const
	{
		if (target->hasFlag("LeixuanUp"))
			return 1;
		if (target->hasFlag("LeixuanDown"))
			return -1;
		return 0;
	}
};

JinggeCard::JinggeCard()
{
	target_fixed = true;
}

void JinggeCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	QStringList suits;
	foreach (int id, getSubcards()) {
		const Card *c = Sanguosha->getCard(id);
		if (!suits.contains(c->getSuitString()))
			suits << c->getSuitString();
	}
	if (suits.contains("heart")) {
		QList<ServerPlayer *> wounds;
		foreach (ServerPlayer *p, room->getOtherPlayers(source)) {
			if (p->isWounded())
				wounds << p;
		}
		ServerPlayer *recover_target = room->askForPlayerChosen(source, wounds, "jinggerecover", "@jingge-recover", true, false);
		if (recover_target) {
			room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, source->objectName(), recover_target->objectName());
			RecoverStruct recover;
			recover.who = source;
			recover.recover = 1;
			recover.reason = "jingge";
			room->recover(recover_target, recover);
		}
	}
	if (suits.contains("diamond")) {
		ServerPlayer *draw_target = room->askForPlayerChosen(source, room->getAlivePlayers(), "jinggedraw", "@jingge-draw", true, false);
		if (draw_target) {
			room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, source->objectName(), draw_target->objectName());
			draw_target->drawCards(1);
		}
	}
	if (suits.contains("spade")) {
		QList<ServerPlayer *> carders;
		foreach (ServerPlayer *p, room->getOtherPlayers(source)) {
			if (!p->isNude())
				carders << p;
		}
		ServerPlayer *limit_target = room->askForPlayerChosen(source, carders, "jinggelimit", "@jingge-discard", true, false);
		if (limit_target) {
			room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, source->objectName(), limit_target->objectName());
			room->askForDiscard(limit_target, "jingge", 1, 1, false, true, "@jingge-throw:" + source->objectName());
		}
	}
	if (suits.contains("club")) {
		room->setPlayerFlag(source, "JinggeNoLimit");
	}
}

class JinggeVS : public ViewAsSkill
{
	
public:
	JinggeVS() : ViewAsSkill("jingge")
	{
		response_pattern = "@@jingge";
	}
	
	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		if (selected.isEmpty())
			return !to_select->isEquipped();
		else {
			if (to_select->isEquipped())
				return false;
			foreach (const Card *c, selected) {
				if (c->getSuit() == to_select->getSuit())
					return false;
			}
			return true;
		}
	}
	
	const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.isEmpty())
			return NULL;
		
		JinggeCard *card = new JinggeCard;
		card->addSubcards(cards);
		return card;
	}
};

class Jingge : public TriggerSkill
{
	
public:
	Jingge() : TriggerSkill("jingge")
	{
		events << EventPhaseStart << EventPhaseChanging << Death;
		view_as_skill = new JinggeVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *tokiko = data.value<ServerPlayer *>();
			if (tokiko && tokiko->isAlive() && tokiko->hasSkill(this) && tokiko->getPhase() == Player::Play)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tokiko, tokiko, NULL, false);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *tokiko = change.player;
			if (tokiko && tokiko->isAlive() && tokiko->hasSkill(this) && change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tokiko, tokiko, NULL, true);
		} else if (event == Death) {
			DeathStruct death = data.value<DeathStruct>();
			ServerPlayer *tokiko = death.who;
			if (tokiko && tokiko->hasSkill(this))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tokiko, tokiko, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart)
			return room->askForUseCard(invoke->invoker, "@@jingge", "@jingge");
		return false;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseChanging || event == Death) {
			room->setPlayerFlag(invoke->invoker, "-JinggeNoLimit");
			foreach (ServerPlayer *p, room->getAlivePlayers()) {
				room->removePlayerCardLimitation(p, "use,response", ".|diamond,spade,club|.|hand$1");
			}
		}
		return false;
	}
};

class JinggeTM : public TargetModSkill
{
	
public:
	JinggeTM() : TargetModSkill("#jingge-tm")
	{
		pattern = ".";
	}
	
	int getResidueNum(const Player *tokiko, const Card *) const
	{
		if (tokiko->hasFlag("JinggeNoLimit"))
			return 1000;
		return 0;
	}
	
	int getDistanceLimit(const Player *tokiko, const Card *) const
	{
		if (tokiko->hasFlag("JinggeNoLimit"))
			return 1000;
		return 0;
	}
};

class Sujuan : public TriggerSkill
{
	
public:
	Sujuan() : TriggerSkill("sujuan")
	{
		events << EventPhaseStart << TargetSpecified << EventPhaseChanging;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *tokiko = data.value<ServerPlayer *>();
			if (tokiko && tokiko->isAlive() && tokiko->hasSkill(this) && tokiko->getPhase() == Player::RoundStart)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tokiko, tokiko, NULL, false);
		} else if (event == TargetSpecified) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *tokiko = room->findPlayerBySkillName(objectName());
			if (tokiko && tokiko->isAlive() && use.to.contains(tokiko) && use.to.length() == 1 && !use.card->isKindOf("SkillCard")
					&& ((tokiko->getMark("@sujuan_red") > 0 && use.card->isRed()) || (tokiko->getMark("@sujuan_black") > 0
					&& use.card->isBlack())))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tokiko, tokiko, NULL, true);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *tokiko = change.player;
			if (tokiko && tokiko->isAlive() && tokiko->hasSkill(this) && change.to == Player::RoundStart
					&& tokiko->getMark("@sujuan_red") + tokiko->getMark("@sujuan_black") > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tokiko, tokiko, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart) {
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		}
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *tokiko = invoke->invoker;
		if (event == EventPhaseStart) {
			QString choice = room->askForChoice(tokiko, objectName(), "red+black");
			if (choice == "red") {
				room->setPlayerMark(tokiko, "@sujuan_red", 1);
				room->touhouLogmessage("#SujuanRedLog", tokiko);
			} else if (choice == "black") {
				room->setPlayerMark(tokiko, "@sujuan_black", 1);
				room->touhouLogmessage("#SujuanBlackLog", tokiko);
			}
		} else if (event == TargetSpecified) {
			room->sendCompulsoryTriggerLog(tokiko, objectName());
			tokiko->drawCards(1);
		}
		else if (event == EventPhaseChanging) {
			room->setPlayerMark(tokiko, "@sujuan_red", 0);
			room->setPlayerMark(tokiko, "@sujuan_black", 0);
		}
		return false;
	}
};

FeimanCard::FeimanCard()
{
	
}

bool FeimanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self && !to_select->isNude();
}

void FeimanCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *from = effect.from;
	ServerPlayer *to = effect.to;
	Room *room = from->getRoom();
	room->setPlayerFlag(to, "FeimanTarget");
	int id = room->askForCardChosen(from, to, "he", "feiman");
	room->throwCard(id, to, from);
	room->setFixedDistance(from, to, 1);
}

class Feiman : public ZeroCardViewAsSkill
{
	
public:
	Feiman() : ZeroCardViewAsSkill("feiman")
	{
		
	}
	
	bool isEnabledAtPlay(const Player *sekibanki) const
	{
		return !sekibanki->hasUsed("FeimanCard");
	}
	
	const Card *viewAs() const
	{
		return new FeimanCard;
	}
};

class FeimanAdd : public TriggerSkill
{
	
public:
	FeimanAdd() : TriggerSkill("#feiman-add")
	{
		events << Damaged << EventPhaseStart << EventPhaseChanging;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *sekibanki = damage.from;
			if (sekibanki && sekibanki->isAlive() && damage.to->hasFlag("FeimanTarget"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sekibanki, sekibanki, NULL, true, damage.to);
		} else if (event == EventPhaseStart) {
			ServerPlayer *sekibanki = data.value<ServerPlayer *>();
			if (sekibanki && sekibanki->isAlive() && sekibanki->hasSkill(this) && sekibanki->getPhase() == Player::Finish
					&& !sekibanki->isKongcheng()) {
				foreach (ServerPlayer *p, room->getAlivePlayers()) {
					if (p->hasFlag("FeimanTarget"))
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sekibanki, sekibanki, NULL, true, p);
				}
			}
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *sekibanki = change.player;
			if (sekibanki && sekibanki->isAlive() && sekibanki->hasSkill(this) && change.from == Player::Finish) {
				foreach (ServerPlayer *p, room->getAlivePlayers()) {
					if (p->hasFlag("FeimanTarget"))
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sekibanki, sekibanki, NULL, true, p);
				}
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *sekibanki = invoke->invoker;
		ServerPlayer *target = invoke->preferredTarget;
		if (event == Damaged) {
			room->setPlayerFlag(target, "-FeimanTarget");
		} else if (event == EventPhaseStart) {
			int id = room->askForCardChosen(target, sekibanki, "h", objectName());
			room->throwCard(id, sekibanki, target);
		} else if (event == EventPhaseChanging) {
			room->setPlayerFlag(target, "-FeimanTarget");
			room->setFixedDistance(sekibanki, target, -1);
		}
		return false;
	}
};

YuniCard::YuniCard()
{

}

bool YuniCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->isWounded();
}

void YuniCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	ServerPlayer *from = effect.from;
	ServerPlayer *to = effect.to;
	int n = qMax(from->getHandcardNum(), to->getLostHp());
	to->tag["YuniAmount"] = QVariant::fromValue(n);
	if (!from->isKongcheng())
		from->throwAllHandCards();
	JudgeStruct judge;
	judge.who = to;
	judge.pattern = ".";
	judge.reason = "yuni";
	room->judge(judge);
	if (judge.card->isRed()) {
		RecoverStruct recover;
		recover.who = from;
		recover.recover = 1;
		recover.reason = "yuni";
		room->recover(to, recover);
	} else if (judge.card->isBlack()) {
		to->drawCards(n);
	}
}

class YuniVS : public ZeroCardViewAsSkill
{

public:
	YuniVS() : ZeroCardViewAsSkill("yuni")
	{
		response_pattern = "@@yuni";
	}

	const Card *viewAs() const
	{
		return new YuniCard;
	}
};

class Yuni : public TriggerSkill
{
	
public:
	Yuni() : TriggerSkill("yuni")
	{
		events << EventPhaseStart;
		view_as_skill = new YuniVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *seiga = data.value<ServerPlayer *>();
		if (seiga && seiga->isAlive() && seiga->hasSkill(this) && seiga->getPhase() == Player::RoundStart) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, seiga, seiga, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@yuni", "@yuni");
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

class Daorang : public TriggerSkill
{

public:
	Daorang() : TriggerSkill("daorang")
	{
		events << AskForRetrial << EventPhaseStart;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == AskForRetrial) {
			ServerPlayer *seiga = room->findPlayerBySkillName(objectName());
			if (seiga && seiga->isAlive()) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *p, room->getAlivePlayers()) {
					if (!p->isNude())
						targets << p;
				}
				if (!targets.isEmpty())
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, seiga, seiga, targets, false);
			}
		} else if (event == EventPhaseStart) {
			ServerPlayer *current = data.value<ServerPlayer *>();
			ServerPlayer *seiga = room->findPlayerBySkillName(objectName());
			if (current && current->isAlive() && current->getPhase() == Player::RoundStart && current->getPile("spirit").length() > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, seiga, current, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == AskForRetrial) {
			invoke->invoker->tag["DaorangJudge"] = data;
			ServerPlayer *target = room->askForPlayerChosen(invoke->invoker, invoke->targets, objectName(), "@daorang-target", true, true);
			if (target) {
				invoke->invoker->tag["DaorangTarget"] = QVariant::fromValue(target);
				return true;
			}
			return false;
		}
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == AskForRetrial) {
			ServerPlayer *seiga = invoke->invoker;
			ServerPlayer *target = seiga->tag["DaorangTarget"].value<ServerPlayer *>();
			int n = qMin(target->getCards("he").length(), seiga->getHandcardNum());
			if (n > 0) {
				for (int i = 0; i < n; i++) {
					int id = room->askForCardChosen(seiga, target, "he", objectName());
					target->addToPile("spirit", id, false);
				}
			}
			JudgeStruct *judge = data.value<JudgeStruct *>();
			const Card *card = room->askForCard(target, ".|.|.|hand", "@daorang-retrial", data, Card::MethodResponse, judge->who, true, objectName());
			if (card != NULL)
				room->retrialWithoutThrow(card, target, judge, objectName());
		} else if (event == EventPhaseStart) {
			ServerPlayer *current = invoke->invoker;
			DummyCard *dummy = new DummyCard;
			dummy->addSubcards(current->getPile("spirit"));
			room->obtainCard(current, dummy);
		}
		return false;
	}
};

class Xianhun : public TriggerSkill
{

public:
	Xianhun() : TriggerSkill("xianhun")
	{
		events << Damaged;
		frequency = Wake;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *seiga = damage.to;
		if (seiga && seiga->isAlive() & seiga->hasSkill(this) && seiga->getMark(objectName()) == 0 && seiga->isKongcheng()) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, seiga, seiga, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->doSuperLightbox("seiga:xianhun", "xianhun");
		ServerPlayer *seiga = invoke->invoker;
		room->addPlayerMark(seiga, objectName(), 1);
		if (room->changeMaxHpForAwakenSkill(seiga)) {
			int n = qMax(0, seiga->getMaxHp() - seiga->getHandcardNum());
			if (n > 0)
				seiga->drawCards(n);
			room->acquireSkill(seiga, "daorang");
		}
		return false;
	}
};

class Caizui : public TriggerSkill
{

public:
	Caizui() : TriggerSkill("caizui")
	{
		events << Damaged;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		ServerPlayer *shikieiki = room->findPlayerBySkillName(objectName());
		ServerPlayer *from;
		if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			from = damage.from;
			if (shikieiki && shikieiki->isAlive() && from && from->isAlive() && shikieiki != from)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, shikieiki, shikieiki, NULL, false, from);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *shikieiki = invoke->invoker;
		ServerPlayer *player = invoke->preferredTarget;
		QString choice;
		if (player->getMark("@felony") > 0)
			choice = room->askForChoice(player, objectName(), "CaizuiGain+CaizuiLose");
		else
			choice = "CaizuiGain";
		if (choice == "CaizuiGain")
			room->addPlayerMark(player, "@felony", 1);
		else if (choice == "CaizuiLose") {
			room->removePlayerMark(player, "@felony", 1);
			room->loseHp(player, 1);
		}
		return false;
	}
};

JiaomianCard::JiaomianCard()
{

}

bool JiaomianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->getMark("@felony") > 0 && to_select != Self && !to_select->hasFlag("JiaomianTargeted");
}

void JiaomianCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *from = effect.from;
	ServerPlayer *to = effect.to;
	Room *room = from->getRoom();
	room->removePlayerMark(to, "@felony", 1);
	room->setFixedDistance(from, to, 1);
	room->setPlayerFlag(to, "JiaomianTargeted");
	QString choice = room->askForChoice(to, "jiaomian", "JiaomianDraw+JiaomianProhibit");
	if (choice == "JiaomianDraw")
		from->drawCards(1);
	else if (choice == "JiaomianProhibit") {
		to->throwAllEquips();
		room->setPlayerCardLimitation(to, "use,response", ".|.|.|hand", true);
		room->setPlayerFlag(from, "JiaomianProhibited");
	}
}

class Jiaomian : public ZeroCardViewAsSkill
{

public:
	Jiaomian() : ZeroCardViewAsSkill("jiaomian")
	{

	}

	bool isEnabledAtPlay(const Player *target) const
	{
		return !target->hasFlag("JiaomianProhibited");
	}

	bool isEnabledAtResponse(const Player *, const QString &) const
	{
		return false;
	}

	const Card *viewAs() const
	{
		return new JiaomianCard;
	}
};

class JiaomianAdd : public TriggerSkill
{

public:
	JiaomianAdd() : TriggerSkill("#jiaomian-add")
	{
		events << EventPhaseChanging << Death;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *shikieiki = change.player;
			if (shikieiki && shikieiki->isAlive() && shikieiki->hasSkill("jiaomian") && change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, shikieiki, shikieiki, NULL, true);
		} else if (event == Death) {
			DeathStruct death = data.value<DeathStruct>();
			ServerPlayer *shikieiki = death.who;
			if (shikieiki && shikieiki->hasSkill("jiaomian"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, shikieiki, shikieiki, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *shikieiki = invoke->invoker;
		room->setPlayerFlag(shikieiki, "-JiaomianProhibited");
		foreach (ServerPlayer *p, room->getOtherPlayers(shikieiki)) {
			room->setPlayerFlag(p, "-JiaomianTargeted");
			room->clearPlayerCardLimitation(p, true);
			if (event == EventPhaseChanging)
				room->setFixedDistance(shikieiki, p, -1);
		}
		return false;
	}
};

class Jigong : public TriggerSkill
{

public:
	Jigong() : TriggerSkill("jigong")
	{
		events << DrawInitialCards << EventPhaseChanging;
		frequency = Compulsory;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == DrawInitialCards) {
			DrawNCardsStruct q = data.value<DrawNCardsStruct>();
			ServerPlayer *ex_kaguya = q.player;
			if (ex_kaguya && ex_kaguya->isAlive() && ex_kaguya->hasSkill(this) && q.isInitial)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_kaguya, ex_kaguya, NULL, true);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *ex_kaguya = change.player;
			if (ex_kaguya && ex_kaguya->isAlive() && ex_kaguya->hasSkill(this) && ((change.to == Player::Judge && ex_kaguya->getMark("jigong_judge") == 0)
					|| (change.to == Player::Discard && ex_kaguya->getMark("jigong_discard") == 0)))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_kaguya, ex_kaguya, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_kaguya = invoke->invoker;
		if (event == DrawInitialCards) {
			room->sendCompulsoryTriggerLog(ex_kaguya, objectName());
			DrawNCardsStruct q = data.value<DrawNCardsStruct>();
			q.n = 8;
			data = QVariant::fromValue(q);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			if (change.to == Player::Judge)
				room->setPlayerMark(ex_kaguya, "jigong_judge", 1);
			else if (change.to == Player::Discard)
				room->setPlayerMark(ex_kaguya, "jigong_discard", 1);
			room->sendCompulsoryTriggerLog(ex_kaguya, objectName());
			ex_kaguya->skip(change.to);
		}
		return false;
	}
};

class Zhenling : public TriggerSkill
{

public:
	Zhenling() : TriggerSkill("zhenling")
	{
		events << EventPhaseStart;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		ServerPlayer *lord = data.value<ServerPlayer *>();
		ServerPlayer *ex_kaguya = room->findPlayerBySkillName(objectName());
		if (ex_kaguya && ex_kaguya->isAlive() && lord && lord->getRole() == "lord" && lord->getPhase() == Player::RoundStart)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_kaguya, ex_kaguya, NULL, false);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_kaguya = invoke->invoker;
		QString choice;
		if (ex_kaguya->isWounded())
			choice = room->askForChoice(ex_kaguya, objectName(), "ZhenlingRecover+ZhenlingDraw");
		else
			choice = "ZhenlingDraw";
		if (choice == "ZhenlingRecover") {
			RecoverStruct recover;
			recover.who = ex_kaguya;
			recover.recover = 1;
			recover.reason = objectName();
			room->recover(ex_kaguya, recover);
		} else if (choice == "ZhenlingDraw")
			ex_kaguya->drawCards(2);
		return false;
	}
};

class Yiyue : public TriggerSkill
{

public:
	Yiyue() : TriggerSkill("yiyue")
	{
		events << EventPhaseStart << EventPhaseChanging;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *ex_kaguya = data.value<ServerPlayer *>();
			if (ex_kaguya && ex_kaguya->isAlive() && ex_kaguya->hasSkill(this) && ex_kaguya->getPhase() == Player::Finish && !ex_kaguya->isKongcheng())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_kaguya, ex_kaguya, NULL, false);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *ex_kaguya = change.player;
			if (ex_kaguya && ex_kaguya->isAlive() && ex_kaguya->hasSkill(this) && change.to == Player::RoundStart)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_kaguya, ex_kaguya, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *ex_kaguya = invoke->invoker;
			const Card *card = room->askForCard(ex_kaguya, ".", "@yiyue-show", data, Card::MethodNone, NULL, false, objectName());
			if (!card || card == NULL)
				return false;
			ex_kaguya->tag["YiyueCardId"] = QVariant::fromValue(card->getEffectiveId());
		}
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_kaguya = invoke->invoker;
		if (event == EventPhaseStart) {
			int id = ex_kaguya->tag["YiyueCardId"].toInt();
			const Card *card = Sanguosha->getCard(id);
			room->showCard(ex_kaguya, id);
			if (card->isRed())
				room->acquireSkill(ex_kaguya, "huanni");
			else if (card->isBlack())
				room->acquireSkill(ex_kaguya, "xiaoan");
		} else if (event == EventPhaseChanging) {
			room->detachSkillFromPlayer(ex_kaguya, "huanni");
			room->detachSkillFromPlayer(ex_kaguya, "xiaoan");
		}
		return false;
	}
};

YongyeCard::YongyeCard()
{

}

bool YongyeCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return !to_select->isNude() && to_select != Self && targets.length() < Self->getHandcardNum();
}

void YongyeCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	room->doSuperLightbox("ex_kaguya:yongye", "yongye");
	room->setPlayerMark(source, "@evernight", 0);
	int n = source->getHandcardNum();
	source->throwAllHandCards();
	foreach (ServerPlayer *p, targets) {
		int id = room->askForCardChosen(source, p, "he", "yongye");
		const Card *c = Sanguosha->getCard(id);
		source->obtainCard(c);
	}
	RecoverStruct recover;
	recover.who = source;
	recover.recover = n;
	recover.reason = "yongye";
	room->recover(source, recover);
	room->handleAcquireDetachSkills(source, "yiyue|-zhenling");
}

class Yongye : public ZeroCardViewAsSkill
{

public:
	Yongye() : ZeroCardViewAsSkill("yongye")
	{
		frequency = Limited;
		limit_mark = "@evernight";
	}

	bool isEnabledAtPlay(const Player *target) const
	{
		if (target->isKongcheng())
			return false;

		foreach (const Card *c, target->getHandcards()) {
			if (!c->isBlack())
				return false;
		}
		return true;
	}

	const Card *viewAs() const
	{
		return new YongyeCard;
	}
};

SuozhenCard::SuozhenCard()
{

}

bool SuozhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self;
}

void SuozhenCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *to = effect.to;
	Room *room = to->getRoom();
	room->addPlayerMark(to, "@shuttle", 1);
}

class Suozhen : public OneCardViewAsSkill
{

public:
	Suozhen() : OneCardViewAsSkill("suozhen")
	{
		filter_pattern = "TrickCard,EquipCard";
	}

	bool isEnabledAtPlay(const Player *target) const
	{
		return !target->isNude();
	}

	const Card *viewAs(const Card *originalCard) const
	{
		SuozhenCard *card = new SuozhenCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class SuozhenAdd : public MasochismSkill
{

public:
	SuozhenAdd() : MasochismSkill("#suozhen-add")
	{

	}

	QList<SkillInvokeDetail> triggerable(const Room *room, const DamageStruct &damage) const
	{
		ServerPlayer *ex_parsee = damage.to;
		ServerPlayer *from = damage.from;
		if (ex_parsee && ex_parsee->isAlive() && ex_parsee->hasSkill("suozhen") && from && from->isAlive() && from->getMark("@shuttle") > 0)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_parsee, ex_parsee, NULL, false, from);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, "suozhen", data);
	}

	void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
	{
		ServerPlayer *ex_parsee = invoke->invoker;
		ServerPlayer *from = invoke->preferredTarget;
		room->removePlayerMark(from, "@shuttle", 1);
		if (!from->isKongcheng()) {
			int id = room->askForCardChosen(ex_parsee, from, "h", "suozhen");
			room->throwCard(id, from, ex_parsee);
		}
		room->loseHp(from, 1);
	}
};

class Gelong : public TriggerSkill
{

public:
	Gelong() : TriggerSkill("gelong")
	{
		events << EventPhaseStart;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *player = data.value<ServerPlayer *>();
		ServerPlayer *ex_parsee = room->findPlayerBySkillName(objectName());
		if (ex_parsee && ex_parsee->isAlive() && ex_parsee->hasSkill(this) && player && player->isAlive() && player->inMyAttackRange(ex_parsee)
				&& player != ex_parsee && player->getPhase() == Player::Play && player->getHandcardNum() > ex_parsee->getHandcardNum())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_parsee, ex_parsee, NULL, false, player);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_parsee = invoke->invoker;
		ServerPlayer *player = invoke->preferredTarget;
		ex_parsee->drawCards(1);
		if (ex_parsee->getHandcardNum() < player->getHandcardNum())
			if (room->askForSkillInvoke(ex_parsee, "gelong_mark", QVariant("mark")))
				room->addPlayerMark(player, "@shuttle", 1);
		return false;
	}
};

class Queji : public TriggerSkill
{

public:
	Queji() : TriggerSkill("queji")
	{
		events << EventPhaseStart;
		frequency = Wake;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *ex_parsee = data.value<ServerPlayer *>();
		if (ex_parsee && ex_parsee->isAlive() && ex_parsee->hasSkill(this) && ex_parsee->getMark("queji") == 0 && ex_parsee->getPhase() == Player::RoundStart) {
			int n = 0;
			foreach (ServerPlayer *p, room->getOtherPlayers(ex_parsee)) {
				n += p->getMark("@shuttle");
			}
			if (n >= 3)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_parsee, ex_parsee, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_parsee = invoke->invoker;
		room->doSuperLightbox("ex_parsee:queji", "queji");
		room->setPlayerMark(ex_parsee, "queji", 1);
		if (room->changeMaxHpForAwakenSkill(ex_parsee))
			room->acquireSkill(ex_parsee, "gelong");
		return false;
	}
};

THVenusPackage::THVenusPackage()
	: Package("thvenus")
{
	General *seijya = new General(this, "seijya", "hakurei", 4, false);
	seijya->addSkill(new Fanni);
	
	General *rumia = new General(this, "rumia", "hakurei", 3, false);
	rumia->addSkill(new Xiaoan);
	rumia->addSkill(new Yueshi);
	
	General *nazrin = new General(this, "nazrin", "hakurei", 3, false);
	nazrin->addSkill(new Tannang);
	nazrin->addSkill(new Lingbai);
	
	General *shyou = new General(this, "shyou", "hakurei", 4, false);
	shyou->addSkill(new Jinghun);
	shyou->addSkill(new Caiyuan);
	
	General *momiji = new General(this, "momiji", "hakurei", 3, false);
	momiji->addSkill(new Guangjie);
	momiji->addSkill(new Langyan);
	
	General *tojiko = new General(this, "tojiko", "moriya", 3, false);
	tojiko->addSkill(new Juanling);
	tojiko->addSkill(new JuanlingPS);
	related_skills.insertMulti("juanling", "#juanling-prohibit");
	tojiko->addSkill(new Leixuan);
	tojiko->addSkill(new LeixuanMaxCards);
	related_skills.insertMulti("leixuan", "#leixuan-max-cards");
	
	General *tokiko = new General(this, "tokiko", "moriya", 3, false);
	tokiko->addSkill(new Jingge);
	tokiko->addSkill(new JinggeTM);
	related_skills.insertMulti("jingge", "#jingge-tm");
	tokiko->addSkill(new Sujuan);
	
	General *sekibanki = new General(this, "sekibanki", "moriya", 4, false);
	sekibanki->addSkill(new Feiman);
	sekibanki->addSkill(new FeimanAdd);
	related_skills.insertMulti("feiman", "#feiman-add");

	General *seiga = new General(this, "seiga", "moriya", 4, false);
	seiga->addSkill(new Yuni);
	seiga->addSkill(new Xianhun);

	General *shikieiki = new General(this, "shikieiki", "moriya", 3, false);
	shikieiki->addSkill(new Caizui);
	shikieiki->addSkill(new Jiaomian);
	shikieiki->addSkill(new JiaomianAdd);
	related_skills.insertMulti("jiaomian", "#jiaomian-add");

	General *ex_kaguya = new General(this, "ex_kaguya", "god", 3, false);
	ex_kaguya->addSkill(new Jigong);
	ex_kaguya->addSkill(new Zhenling);
	ex_kaguya->addSkill(new Yongye);

	General *ex_parsee = new General(this, "ex_parsee", "god", 4, false);
	ex_parsee->addSkill(new Suozhen);
	ex_parsee->addSkill(new SuozhenAdd);
	related_skills.insertMulti("suozhen", "#suozhen-add");
	ex_parsee->addSkill(new Queji);
	
	addMetaObject<FanniCard>();
	addMetaObject<XiaoanCard>();
	addMetaObject<LingbaiCard>();
	addMetaObject<JinghunCard>();
	addMetaObject<CaiyuanCard>();
	addMetaObject<JinggeCard>();
	addMetaObject<FeimanCard>();
	addMetaObject<YuniCard>();
	addMetaObject<JiaomianCard>();
	addMetaObject<YongyeCard>();
	addMetaObject<SuozhenCard>();

	skills << new Daorang << new Yiyue << new Gelong;
}

ADD_PACKAGE(THVenus)

