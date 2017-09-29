#include "thweave.h"
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

JiejieCard::JiejieCard()
{
	will_throw = false;
}

bool JiejieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self && to_select->getPhase() == Player::Play;
}

void JiejieCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	room->obtainCard(effect.to, this, true);
	QString choice = room->askForChoice(effect.from, "jiejie", "JJBuff+JJDebuff");
	if (choice == "JJBuff") {
		foreach(ServerPlayer *p, room->getOtherPlayers(effect.to)) {
			room->setFixedDistance(effect.to, p, 1);
		}
		room->addPlayerMark(effect.to, "@enchant_buff", 1);
	}
	else
		room->addPlayerMark(effect.to, "@enchant_debuff", 1);
}

class JiejieVS : public OneCardViewAsSkill
{
	
public:
	JiejieVS() : OneCardViewAsSkill("jiejie")
	{
		filter_pattern = "BasicCard";
		response_pattern = "@@jiejie";
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		JiejieCard *card = new JiejieCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Jiejie : public TriggerSkill
{
	
public:
	Jiejie() : TriggerSkill("jiejie")
	{
		events << EventPhaseStart;
		view_as_skill = new JiejieVS;
	}
	
    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *yukari = room->findPlayerBySkillName(objectName());
		ServerPlayer *player = room->getCurrent();
		if (yukari && yukari->isAlive() && player != yukari) {
			if (player->getPhase() == Player::Play && !yukari->isKongcheng())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yukari, yukari, NULL, false, player);
			else if (player->getPhase() == Player::Finish && player->getMark("@enchant_buff") + player->getMark("@enchant_debuff") > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yukari, yukari, NULL, true, player);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->preferredTarget;
		if (player->getPhase() == Player::Play) {
			ServerPlayer *yukari = invoke->invoker;
			return room->askForUseCard(yukari, "@@jiejie", "@jiejie");
		}
		else if (player->getPhase() == Player::Finish)
			return true;
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->preferredTarget;
		if (player->getPhase() == Player::Finish) {
			room->setPlayerMark(player, "@enchant_buff", 0);
			room->setPlayerMark(player, "@enchant_debuff", 0);
			foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
				room->setFixedDistance(player, p, -1);
			}
		}
		return false;
	}
};

class JiejieTM : public TargetModSkill
{
	
public:
	JiejieTM() : TargetModSkill("#jiejie-targetmod")
	{
		pattern = "Slash";
	}
	
	int getResidueNum(const Player *player, const Card *) const
	{
		if (player->getMark("@enchant_buff") > 0)
			return 1;
		else
			return 0;
	}
	
	int getExtraTargetNum(const Player *player, const Card *) const
	{
		if (player->getMark("@enchant_buff") > 0)
			return 1;
		else
			return 0;
	}
};

class JiejieProhibit : public ProhibitSkill
{
	
public:
	JiejieProhibit() : ProhibitSkill("#jiejie-prohibit")
	{
	}
	
	bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &) const
	{
		return from->getMark("@enchant_debuff") > 0 && from->distanceTo(to) > 1;
	}
};

class Xianhu : public TriggerSkill
{
	
public:
	Xianhu() : TriggerSkill("xianhu")
	{
		frequency = Compulsory;
		events << CardsMoveOneTime;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		if (move.from && move.from->isAlive() && move.from->hasSkill(this) && move.from_places.contains(Player::PlaceHand)
				&& move.from->getPhase() == Player::NotActive && move.from->isKongcheng()) {
            ServerPlayer *ran = qobject_cast<ServerPlayer *>(move.from);
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ran, ran, NULL, true, room->getCurrent());
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->preferredTarget;
		if (!room->askForCard(player, "BasicCard", "@xianhu-discard", data))
			room->loseHp(player, 1);
		return false;
	}
};

ShishenCard::ShishenCard()
{
	
}

bool ShishenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->isWounded() && !to_select->isKongcheng();
}

void ShishenCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	const Card *card = room->askForCard(effect.to, ".|.|.|hand", "@shishen-discard");
	if (card) {
		if (card->isKindOf("BasicCard"))
			effect.to->drawCards(2);
		else if (card->isKindOf("TrickCard")) {
			RecoverStruct recover;
			recover.who = effect.from;
			recover.recover = 1;
			recover.reason = "shishen";
			room->recover(effect.to, recover);
		}
		else if (card->isKindOf("EquipCard")) {
			QList<ServerPlayer *> targets;
			foreach(ServerPlayer *p, room->getOtherPlayers(effect.to)) {
				if (!p->isKongcheng())
					targets << p;
			}
			ServerPlayer *target = room->askForPlayerChosen(effect.to, targets, "shishen");
			int id = room->askForCardChosen(effect.to, target, "h", "shishen");
			room->obtainCard(effect.to, Sanguosha->getCard(id), false);
		}
	}
	else
		room->showAllCards(effect.to, effect.from);
}

class ShishenVS : public ZeroCardViewAsSkill
{
	
public:
	ShishenVS() : ZeroCardViewAsSkill("shishen")
	{
		response_pattern = "@@shishen";
	}
	
	virtual const Card *viewAs() const
	{
		return new ShishenCard;
	}
};

class Shishen : public TriggerSkill
{
	
public:
	Shishen() : TriggerSkill("shishen")
	{
		events << EventPhaseStart;
		view_as_skill = new ShishenVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *ran = room->getCurrent();
		if (ran && ran->hasSkill(this) && ran->getPhase() == Player::Finish) {
			QList<ServerPlayer *> targets;
			foreach(ServerPlayer *p, room->getAlivePlayers()) {
				if (p->isWounded())
					targets << p;
			}
			if (!targets.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ran, ran, targets, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ran = invoke->invoker;
		return room->askForUseCard(ran, "@@shishen", "@shishen");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

class Dunjia : public TriggerSkill
{
	
public:
	Dunjia() : TriggerSkill("dunjia")
	{
		events << TargetConfirming << Damaged;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
        ServerPlayer *chen = room->findPlayerBySkillName(objectName());
        if (!chen || !chen->isAlive())
            return QList<SkillInvokeDetail>();
		if (event == TargetConfirming) {
			CardUseStruct use = data.value<CardUseStruct>();
			if ((!use.from || !use.from->isAlive() || use.from != chen) && !use.to.contains(chen) && use.card->isKindOf("Slash")
					&& use.card->isRed()) {
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, chen, chen, NULL, false);
			}
		}
		else if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			if (damage.to && damage.to->isAlive() && damage.card && damage.card->hasFlag("DunjiaFlag"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, chen, chen, NULL, true, damage.to);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == TargetConfirming) {
			ServerPlayer *chen = invoke->invoker;
			return room->askForSkillInvoke(chen, objectName(), data);
		}
		else
			return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == TargetConfirming) {
			CardUseStruct use = data.value<CardUseStruct>();
			room->setCardFlag(use.card, "DunjiaFlag");
			use.to.append(invoke->invoker);
			room->sortByActionOrder(use.to);
			data = QVariant::fromValue(use);
		}
		else if (event == Damaged) {
			ServerPlayer *player = invoke->preferredTarget;
			player->drawCards(3);
		}
		return false;
	}
};

class Fengshi : public TriggerSkill
{
	
public:
	Fengshi() : TriggerSkill("fengshi")
	{
        events << EventPhaseStart;
		frequency = Wake;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *chen = data.value<ServerPlayer *>();
		if (chen && chen->hasSkill(this) && chen->getPhase() ==  Player::RoundStart && (chen->isKongcheng() || chen->getHandcardNum() >= 5)
				&& chen->getMark(objectName()) == 0) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, chen, chen, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->doSuperLightbox("chen:fengshi", objectName());
		ServerPlayer *chen = invoke->invoker;
		room->addPlayerMark(chen, objectName());
		if (room->changeMaxHpForAwakenSkill(chen))
            room->acquireSkill(chen, "shishen");
		return false;
	}
};

YuanqiCard::YuanqiCard()
{
	will_throw = false;
	handling_method = Card::MethodNone;
}

bool YuanqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	if (!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void YuanqiCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
	ServerPlayer *keine = effect.from;
    keine->getRoom()->moveCardTo(this, keine, effect.to, Player::PlaceEquip,
        CardMoveReason(CardMoveReason::S_REASON_TRANSFER,
        keine->objectName(), "yuanqi", QString()));
	
	room->setPlayerFlag(effect.to, "YuanqiTarget");
}

class Yuanqi : public OneCardViewAsSkill
{
	
public:
	Yuanqi() : OneCardViewAsSkill("yuanqi")
	{
		filter_pattern = "EquipCard|.|.|equipped";
	}
	
	virtual bool isEnabledAtPlay(const Player *player) const
	{
		return !player->hasUsed("YuanqiCard");
	}
	
	virtual const Card *viewAs(const Card *originalCard) const
	{
		YuanqiCard *card = new YuanqiCard;
		card->addSubcard(originalCard);
		card->setSkillName("yuanqi");
		return card;
	}
};

class YuanqiAdd : public TriggerSkill
{
	
public:
	YuanqiAdd() : TriggerSkill("#yuanqi-add")
	{
		events << EventPhaseStart;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *keine = data.value<ServerPlayer *>();
		foreach (ServerPlayer *p, room->getOtherPlayers(keine)) {
			if (p->hasFlag("YuanqiTarget") && !p->getEquips().isEmpty() && keine->getEquips().length() < 5
				&& keine->getPhase() == Player::Finish) {
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, keine, keine, NULL, true, p);
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *keine = invoke->invoker;
		ServerPlayer *player = invoke->preferredTarget;
		room->setPlayerFlag(player, "-YuanqiTarget");
		QList<int> equips;
		QList<int> disabled_equips;
		QList<int> enabled_equips;
		foreach(const Card *card, player->getEquips()) {
			equips << card->getEffectiveId();
			const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
			int equip_index = static_cast<int>(equip->location());
			if (keine->getEquip(equip_index) != NULL)
				disabled_equips << card->getEffectiveId();
			else
				enabled_equips << card->getEffectiveId();
		}
		if (enabled_equips.isEmpty())
			return false;
		room->sendCompulsoryTriggerLog(keine, "yuanqi");
		room->fillAG(equips, keine, disabled_equips);
		int eq_id = room->askForAG(keine, enabled_equips, false, "yuanqi");
		const Card *eq = Sanguosha->getCard(eq_id);
		room->moveCardTo(eq, player, keine, Player::PlaceEquip,
			CardMoveReason(CardMoveReason::S_REASON_TRANSFER,
			keine->objectName(), "yuanqi", QString()));
		room->clearAG(keine);
		return false;
	}
};

/*class YuanqiClear : public TriggerSkill
{
	
public:
	YuanqiClear() : TriggerSkill("#yuanqi-clear")
	{
		events << EventPhaseStart;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *keine = room->getCurrent();
		if (keine->hasSkill("yuanqi") && keine->getPhase() == Player::Finish) {
			foreach(ServerPlayer *p, room->getAlivePlayers()) {
				if (p->hasFlag("YuanqiFlag"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, keine, keine, NULL, true, p);
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->preferredTarget;
		room->setPlayerFlag(player, "-YuanqiFlag");
		return false;
	}
};*/

MishiCard::MishiCard()
{
	
}

bool MishiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return to_select->hasFlag("MishiTarget");
}

void MishiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets)
		p->drawCards(1);
}

class MishiVS : public ZeroCardViewAsSkill
{
	
public:
	MishiVS() : ZeroCardViewAsSkill("mishi")
	{
		response_pattern = "@@mishi";
	}
	
	virtual const Card *viewAs() const
	{
		return new MishiCard;
	}
};

class Mishi : public TriggerSkill
{
	
public:
	Mishi() : TriggerSkill("mishi")
	{
		events << CardsMoveOneTime << EventPhaseChanging;
		view_as_skill = new MishiVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardsMoveOneTime) {
			CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
			ServerPlayer *from = qobject_cast<ServerPlayer *>(move.from);
			ServerPlayer *to  = qobject_cast<ServerPlayer *>(move.to);
			ServerPlayer *keine = room->findPlayerBySkillName(objectName());
			if (keine && keine->isAlive() && from && to && from->isAlive() && !keine->hasFlag("MishiUsed")
					&& to->isAlive() && move.from_places.contains(Player::PlaceEquip) && from != to) {
				QList<ServerPlayer *> targets;
				targets << from << to;
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, keine, keine, targets, false);
			}
		}
		else if (event == EventPhaseChanging) {
			ServerPlayer *keine = room->findPlayerBySkillName(objectName());
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			if (keine && keine->isAlive() && change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, keine, keine, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == CardsMoveOneTime) {
			foreach (ServerPlayer *p, invoke->targets)
				room->setPlayerFlag(p, "MishiTarget");
			ServerPlayer *keine = invoke->invoker;
			return room->askForUseCard(keine, "@@mishi", "@mishi");
		}
		return true;
	}
	
    bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
	{
		if (event == CardsMoveOneTime) {
			ServerPlayer *keine = invoke->invoker;
			room->setPlayerFlag(keine, "MishiUsed");
		}
		if (event == EventPhaseChanging) {
			foreach (ServerPlayer *p, room->getAlivePlayers()) {
				if (p->hasFlag("MishiTarget"))
					room->setPlayerFlag(p, "-MishiTarget");
			}
			ServerPlayer *keine = invoke->invoker;
			if (keine->hasFlag("MishiUsed"))
				room->setPlayerFlag(keine, "-MishiUsed");
		}
		return false;
	}
};

MiezuiCard::MiezuiCard()
{
	target_fixed = true;
}

/*void MiezuiCard::use(Room *, ServerPlayer *, QList<ServerPlayer *> &) const
{
	
}*/

class MiezuiVS : public ViewAsSkill
{
	
public:
	MiezuiVS() : ViewAsSkill("miezui")
	{
		response_pattern = "@@miezui";
	}
	
	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		if (selected.isEmpty())
			return !to_select->isEquipped();
		else if (selected.length() == 1)
			return to_select->getSuit() == selected.at(0)->getSuit() && !to_select->isEquipped();
		else
			return false;
	}
	
	virtual const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() == 2) {
			MiezuiCard *card = new MiezuiCard;
			card->addSubcards(cards);
			return card;
		}
		else
			return NULL;
	}
};

class Miezui : public TriggerSkill
{
	
public:
	Miezui() : TriggerSkill("miezui")
	{
		frequency = Compulsory;
		events << ConfirmDamage;
		view_as_skill = new MiezuiVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *mokou = damage.to;
		if (mokou && mokou->isAlive() && mokou->hasSkill(this) && damage.damage >= mokou->getHp() && damage.from && damage.from->isAlive())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, mokou, mokou, NULL, true, damage.from);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *mokou = invoke->invoker;
		ServerPlayer *from = invoke->preferredTarget;
		room->sendCompulsoryTriggerLog(mokou, objectName());
		if (!room->askForUseCard(from, "@@miezui", "@miezui")) {
			if (!from->getEquips().isEmpty()) {
				ServerPlayer *obtainer = room->askForPlayerChosen(mokou, room->getAlivePlayers(), objectName(), "@miezui-obtainer");
				DummyCard *dummy = new DummyCard;
				dummy->addSubcards(from->getEquips());
				obtainer->obtainCard(dummy, true);
			}
			DamageStruct damage = data.value<DamageStruct>();
			damage.damage--;
			data = QVariant::fromValue(damage);
		}
		return false;
	}
};

KuaiqingCard::KuaiqingCard()
{
	
}

bool KuaiqingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < Self->getHandcardNum();
}

void KuaiqingCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach(ServerPlayer *p, targets)
		p->drawCards(1);
}

class KuaiqingVS : public ZeroCardViewAsSkill
{
	
public:
	KuaiqingVS() : ZeroCardViewAsSkill("kuaiqing")
	{
		response_pattern = "@@kuaiqing";
	}
	
	virtual const Card *viewAs() const
	{
		return new KuaiqingCard;
	}
};

class Kuaiqing : public TriggerSkill
{
	
public:
	Kuaiqing() : TriggerSkill("kuaiqing")
	{
		events << Damaged;
		view_as_skill = new KuaiqingVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *mokou = room->findPlayerBySkillName(objectName());
		if (mokou && mokou->isAlive() && damage.from && damage.from->isAlive() && !damage.from->isNude() && damage.from != mokou
				&& damage.card && damage.card->isKindOf("Slash") && damage.to && damage.to->isAlive())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, mokou, mokou, NULL, false, damage.from);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *mokou = invoke->invoker;
		return room->askForCard(mokou, ".", "@kuaiqing-discard", data, objectName());
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *mokou = invoke->invoker;
		ServerPlayer *from = invoke->preferredTarget;
		QStringList choices;
		choices << "KuaiqingDamage";
		if (!from->getEquips().isEmpty())
			choices << "KuaiqingDisarm";
		QString choice = room->askForChoice(from, objectName(), choices.join("+"));
		if (choice == "KuaiqingDisarm") {
			DummyCard *dummy = new DummyCard;
			dummy->addSubcards(from->getEquips());
			room->obtainCard(mokou, dummy, true);
			room->askForUseCard(mokou, "@@kuaiqing", "@kuaiqing");
		}
        else if (choice == "KuaiqingDamage") {
			int id = room->askForCardChosen(mokou, from, "he", objectName());
			const Card *card = Sanguosha->getCard(id);
			mokou->obtainCard(card, (room->getCardPlace(id) != Player::PlaceHand));
			QList<int> card_ids;
			card_ids << id;
			CardMoveReason reason = CardMoveReason(CardMoveReason::S_REASON_GIVE, "mokou");
			room->askForYiji(mokou, card_ids, objectName(), false, false, true, 1, room->getOtherPlayers(mokou), reason, "@kuaiqing-give");
			room->damage(DamageStruct(objectName(), mokou, from, 1, DamageStruct::Fire));
		}
		return false;
	}
};

class Chongsheng : public TriggerSkill
{
	
public:
	Chongsheng() : TriggerSkill("chongsheng")
	{
		events << Dying;
		frequency = Wake;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DyingStruct dying = data.value<DyingStruct>();
		ServerPlayer *mokou = dying.who;
		if (mokou && mokou->hasSkill(this) && mokou->getMark(objectName()) == 0) {
			bool can_respawn = false;
			foreach (ServerPlayer *p, room->getOtherPlayers(mokou)) {
				if (p->getHandcardNum() > mokou->getHandcardNum()) {
					can_respawn = true;
				}
			}
			if (can_respawn)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, mokou, mokou, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->doSuperLightbox("mokou:chongsheng", objectName());
		ServerPlayer *mokou = invoke->invoker;
		room->addPlayerMark(mokou, objectName());
		if (room->changeMaxHpForAwakenSkill(mokou, 1)) {
			RecoverStruct recover;
			recover.recover = 4 - mokou->getHp();
			recover.who = mokou;
			recover.reason = objectName();
			room->recover(mokou, recover);
			if (mokou->getHandcardNum() < 4)
				mokou->drawCards(4 - mokou->getHandcardNum());
			room->detachSkillFromPlayer(mokou, "miezui");
			room->acquireSkill(mokou, "kuaiqing");
		}
		return false;
	}
};

class Lingche : public TriggerSkill
{
	
public:
	Lingche() : TriggerSkill("lingche")
	{
		events << Death;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DeathStruct death = data.value<DeathStruct>();
		ServerPlayer *player = death.who;
		ServerPlayer *rin = room->findPlayerBySkillName(objectName());
		if (rin && rin->isAlive() && !player->isNude())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, rin, rin, NULL, false, player);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *rin = invoke->invoker;
        return room->askForSkillInvoke(rin, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
	{
		ServerPlayer *rin = invoke->invoker;
		ServerPlayer *player = invoke->preferredTarget;
		DummyCard *dummy = new DummyCard;
		foreach(const Card *c, player->getCards("he")) {
			dummy->addSubcard(c);
		}
		rin->obtainCard(dummy, false);
		return false;
	}
};

class Maoyou : public TriggerSkill
{
	
public:
	Maoyou() : TriggerSkill("maoyou")
	{
		events << Dying;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DyingStruct dying = data.value<DyingStruct>();
		ServerPlayer *player = dying.who;
		ServerPlayer *rin = room->findPlayerBySkillName(objectName());
		if (rin && !rin->isDead() && room->getAllPlayers(false).length() > 2) {
			int n = 0;
			foreach(ServerPlayer *p, room->getAllPlayers(true)) {
				if (p->isDead())
					n++;
			}
			if (rin->getMark("@spirit") == 0)
				n = qMin(n, 3);
			if (rin->getCards("he").length() >= n) {
				rin->tag["MaoyouNum"] = QVariant::fromValue(n);
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, rin, rin, NULL, false, player);
			}
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *rin = invoke->invoker;
		int n = rin->tag["MaoyouNum"].toInt();
		if (n == 0)
			return room->askForSkillInvoke(rin, objectName(), data);
		else {
			QString prompt;
			if (rin->getMark("@spirit") > 0)
				prompt = "@maoyou-discard";
			else
				prompt = "@maoyou-discard-new";
			bool yes = room->askForDiscard(rin, objectName(), n, n, true, true, prompt);
			if (yes)
				room->notifySkillInvoked(rin, objectName());
			return yes;
		}
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *rin = invoke->invoker;
		QStringList choices;
		choices << "MaoyouDraw";
		foreach (ServerPlayer *p, room->getAllPlayers(false)) {
			if (!p->isNude()) {
				choices << "MaoyouDiscard";
				break;
			}
		}
		QString choice = room->askForChoice(rin, objectName(), choices.join("+"));
		if (choice == "MaoyouDraw") {
            ServerPlayer *player = room->askForPlayerChosen(rin, room->getAllPlayers(false), "MaoyouDrawTarget");
			player->drawCards(2);
		}
		else if (choice == "MaoyouDiscard") {
			for (int i = 1; i <= 2; i++) {
				QList<ServerPlayer *> targets;
				foreach (ServerPlayer *p, room->getAllPlayers(false)) {
					if (!p->isNude() && rin != p)
						targets << p;
				}
				if (targets.isEmpty()) break;
				ServerPlayer *player = room->askForPlayerChosen(rin, targets, "MaoyouDiscardTarget");
				int id = room->askForCardChosen(rin, player, "he", objectName());
				room->throwCard(id, player, rin);
			}
		}
		if (rin == invoke->preferredTarget) {
			RecoverStruct recover;
			recover.recover = 1;
			recover.who = rin;
			recover.reason = objectName();
			room->recover(rin, recover);
		}
		return false;
	}
};

class Shihun : public TriggerSkill
{
	
public:
	Shihun() : TriggerSkill("shihun")
	{
		events << AskForPeachesDone;
        frequency = Limited;
		limit_mark = "@spirit";
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DyingStruct dying = data.value<DyingStruct>();
		ServerPlayer *player = dying.who;
		ServerPlayer *rin = room->findPlayerBySkillName(objectName());
		if (rin && rin->isAlive() && rin != player && player->getHp() <= 0 && rin->getMark("@spirit") > 0)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, rin, rin, NULL, false, player);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *rin = invoke->invoker;
		return room->askForSkillInvoke(rin, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *rin = invoke->invoker;
		ServerPlayer *player = invoke->preferredTarget;
		room->doSuperLightbox("rin:shihun", "shihun");
        room->removePlayerMark(rin, "@spirit");
		room->setPlayerProperty(player, "maxhp", player->getGeneral()->getMaxHp());
        room->setPlayerProperty(player, "hp", 2);
        room->setEmotion(player, "revive");
        room->touhouLogmessage("#Revive", player);
		room->setPlayerProperty(player, "role_shown", player->isLord() ? true : false);
		room->loseHp(rin, rin->getHp());
		if (rin->isAlive())
			rin->drawCards(1);
		return false;
	}
};

HerongCard::HerongCard()
{
	
}

bool HerongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return Self->inMyAttackRange(to_select) && targets.length() < qMin(Self->getMark("@nuclear") + 1, 4) && to_select != Self;
}

void HerongCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	if (source->getMark("@nuclear") < 4)
		room->addPlayerMark(source, "@nuclear", 1);
	foreach (ServerPlayer *p, targets) {
		room->damage(DamageStruct("herong", source, p, 1, DamageStruct::Fire));
	}
}

class Herong : public ViewAsSkill
{
	
public:
    Herong() : ViewAsSkill("herong")
	{
		
	}
	
	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		bool can_throw;
		if (selected.isEmpty())
			can_throw = true;
		else if (selected.length() == 1)
			can_throw = (to_select->getNumber() + selected.at(0)->getNumber() > 4 * (qMin(Self->getMark("@nuclear") + 1, 4)));
		return selected.length() < 2 && !to_select->isEquipped() && can_throw;
	}
	
	virtual bool isEnabledAtPlay(const Player *utsuho) const
	{
		return !utsuho->hasUsed("HerongCard") && utsuho->getHandcardNum() >= 2;
	}
	
	virtual const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() < 2)
			return NULL;
		HerongCard *card = new HerongCard;
		card->addSubcards(cards);
		return card;
	}
};

class Molian : public TriggerSkill
{
	
public:
	Molian() : TriggerSkill("molian")
	{
		events << EventPhaseEnd << CardUsed << EventPhaseChanging;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseEnd) {
			ServerPlayer *komachi = room->findPlayerBySkillName(objectName());
			ServerPlayer *player = data.value<ServerPlayer *>();
			if (player && player->isAlive() && komachi && komachi->isAlive() && player != komachi && player->getPhase() == Player::Play
					&& !komachi->isNude() && (player->hasFlag("MolianUseBlack") || player->getEquips().length() > komachi->getEquips().length()))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, komachi, komachi, NULL, false, player);
		}
		else if (event == CardUsed) {
			ServerPlayer *komachi = room->findPlayerBySkillName(objectName());
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *player = use.from;
			if (player && player->isAlive() && komachi && komachi->isAlive() && player != komachi && player->getPhase() != Player::NotActive
					&& use.card->isBlack() && !player->hasFlag("MolianUseBlack"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, komachi, komachi, NULL, true, player);
		}
		else if (event == EventPhaseChanging) {
			ServerPlayer *komachi = room->findPlayerBySkillName(objectName());
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *player = change.player;
			if (player && player->isAlive() && komachi && komachi->isAlive() && player != komachi && change.from == Player::Finish
					&& player->hasFlag("MolianUseBlack"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, komachi, komachi, NULL, true, player);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *komachi = invoke->invoker;
		ServerPlayer *player = invoke->preferredTarget;
		if (event == EventPhaseEnd) {
			if (komachi->hasFlag("MolianBoth"))
				room->setPlayerFlag(komachi, "-MolianBoth");
			if (player->hasFlag("MolianUseBlack") && player->getCards("e").length() > komachi->getCards("e").length())
				room->setPlayerFlag(komachi, "MolianBoth");
			return room->askForDiscard(komachi, objectName(), 1, 1, true, true, "@molian-discard");
		} else
			return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *komachi = invoke->invoker;
		ServerPlayer *player = invoke->preferredTarget;
		if (event == EventPhaseEnd) {
			Slash *slash = new Slash(Card::NoSuit, 0);
			slash->setSkillName(objectName());
			CardUseStruct use;
			use.card = slash;
			use.from = komachi;
			use.to.append(player);
			room->useCard(use);
			if (komachi->hasFlag("MolianBoth"))
				komachi->drawCards(1);
		}
		else if (event == CardUsed) {
			room->setPlayerFlag(player, "MolianUseBlack");
		}
		else if (event == EventPhaseChanging) {
			room->setPlayerFlag(player, "-MolianUseBlack");
		}
		return false;
	}
};

class Sijian : public TriggerSkill
{
	
public:
	Sijian() : TriggerSkill("sijian")
	{
		events << TargetConfirmed;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *erin = room->findPlayerBySkillName(objectName());
		if (erin && erin->isAlive() && use.to.contains(erin) && use.to.length() == 1 && use.card->isNDTrick())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, erin, erin, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *erin = invoke->invoker;
		return room->askForSkillInvoke(erin, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *erin = invoke->invoker;
		erin->drawCards(1);
		erin->turnOver();
		return false;
	}
};

class Qionghu : public TriggerSkill
{
	
public:
	Qionghu() : TriggerSkill("qionghu")
	{
		events << TargetConfirmed << EventPhaseStart;
	}
	
    QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == TargetConfirmed) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *erin = use.from;
			if (erin && erin->isAlive() && erin->hasSkill(this) && (use.card->isKindOf("Peach") || use.card->isKindOf("Analeptic")))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, erin, erin, NULL, false);
		}
		else if (event == EventPhaseStart) {
			ServerPlayer *erin = data.value<ServerPlayer *>();
			if (erin && erin->isAlive() && erin->hasSkill(this) && erin->getPhase() == Player::Finish && erin->getMark("qionghu") > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, erin, erin, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
    bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == TargetConfirmed) {
			ServerPlayer *erin = invoke->invoker;
			return room->askForSkillInvoke(erin, objectName(), data);
		}
		return true;
	}
	
    bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *erin = invoke->invoker;
		if (event == TargetConfirmed) {
			erin->drawCards(2);
			erin->turnOver();
			if (erin->getPhase() != Player::NotActive)
				room->addPlayerMark(erin, "qionghu", 1);
		}
		else if (event == EventPhaseStart)
			room->removePlayerMark(erin, "qionghu");
		return false;
	}
};

class QionghuMaxCards : public MaxCardsSkill
{
	
public:
	QionghuMaxCards() : MaxCardsSkill("#qionghu-maxcards")
	{
		
	}
	
	int getExtra(const Player *target) const
	{
		return target->getMark("qionghu") * 2;
	}
};

YuzhiCard::YuzhiCard()
{
	
}

bool YuzhiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < Self->getLostHp() && !to_select->isAllNude() && to_select != Self;
}

void YuzhiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach(ServerPlayer *p, targets) {
		int id = room->askForCardChosen(source, p, "hej", "yuzhi");
		const Card *card = Sanguosha->getCard(id);
		room->obtainCard(source, card, (room->getCardPlace(id) != Player::PlaceHand));
	}
}

class YuzhiVS : public ZeroCardViewAsSkill
{
	
public:
	YuzhiVS() : ZeroCardViewAsSkill("yuzhi")
	{
		response_pattern = "@@yuzhi";
	}
	
	virtual const Card *viewAs() const
	{
		return new YuzhiCard;
	}
};

class Yuzhi : public TriggerSkill
{
	
public:
	Yuzhi() : TriggerSkill("yuzhi")
	{
		events << DrawNCards;
		view_as_skill = new YuzhiVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *kaguya = room->getCurrent();
		if (kaguya && kaguya->isAlive() && kaguya->hasSkill(this) && kaguya->isWounded()) {
			QList<ServerPlayer *> targets;
			foreach(ServerPlayer *p, room->getOtherPlayers(kaguya)) {
                if (!p->isAllNude())
					targets << p;
			}
			if (!targets.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kaguya, kaguya, targets, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *kaguya = invoke->invoker;
		return room->askForUseCard(kaguya, "@@yuzhi", "@yuzhi");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		DrawNCardsStruct qnum = data.value<DrawNCardsStruct>();
		if (qnum.n > 0)
			qnum.n--;
		data = QVariant::fromValue(qnum);
		return false;
	}
};

class Zhuqu : public TriggerSkill
{
	
public:
	Zhuqu() : TriggerSkill("zhuqu")
	{
		events << Death;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DeathStruct death = data.value<DeathStruct>();
		ServerPlayer *kaguya = death.who;
        if (kaguya && kaguya->hasSkill(this) && death.damage != NULL && death.damage->from && death.damage->from->isAlive())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kaguya, kaguya, NULL, true, death.damage->from);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *kaguya = invoke->invoker;
		room->sendCompulsoryTriggerLog(kaguya, objectName());
		ServerPlayer *player = invoke->preferredTarget;
		room->loseHp(player, 2);
		player->throwAllEquips();
		DeathStruct death = data.value<DeathStruct>();
        if (player->isAlive() && death.damage->nature != DamageStruct::Normal)
			player->turnOver();
		return false;
	}
};

class Hongmo : public TriggerSkill
{
	
public:
	Hongmo() : TriggerSkill("hongmo")
	{
		events << DrawNCards << EventPhaseChanging;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == DrawNCards) {
			ServerPlayer *ex_scarlets = room->getCurrent();
			if (ex_scarlets && ex_scarlets->isAlive() && ex_scarlets->hasSkill(this))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_scarlets, ex_scarlets, NULL, false);
		}
		else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *ex_scarlets = change.player;
			if (ex_scarlets && ex_scarlets->isAlive() && ex_scarlets->getMark("hongmo") > 0
					&& change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_scarlets, ex_scarlets, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == DrawNCards) {
			ServerPlayer *ex_scarlets = invoke->invoker;
			return room->askForSkillInvoke(ex_scarlets, objectName(), data);
		}
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == DrawNCards) {
			ServerPlayer *ex_scarlets = invoke->invoker;
			DrawNCardsStruct qnum = data.value<DrawNCardsStruct>();
			qnum.n = 0;
			data = QVariant::fromValue(qnum);
			QList<int> card_ids = room->getNCards(4, false);
			CardsMoveStruct move;
			move.card_ids = card_ids;
			move.to_place = Player::PlaceTable;
			move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, ex_scarlets->objectName(), objectName(), QString());
			room->moveCardsAtomic(move, true);
			QList<int> cards_to_get, cards_to_throw;
			foreach (int id, card_ids) {
				const Card *c = Sanguosha->getCard(id);
				if (c->isRed() || c->isKindOf("Slash")) {
					cards_to_get << id;
				}
				else
					cards_to_throw << id;
			}
			if (!cards_to_get.isEmpty()) {
				DummyCard *dummy = new DummyCard;
				dummy->addSubcards(cards_to_get);
				room->obtainCard(ex_scarlets, dummy, true);
				foreach (int id, cards_to_get) {
					const Card *c = Sanguosha->getCard(id);
					if (c->isRed())
						room->addPlayerMark(ex_scarlets, "hongmo", 1);
				}
			}
			if (!cards_to_throw.isEmpty()) {
				DummyCard *dummy2 = new DummyCard;
				dummy2->addSubcards(cards_to_throw);
				CardMoveReason reason = CardMoveReason(CardMoveReason::S_REASON_NATURAL_ENTER, ex_scarlets->objectName(),
						objectName(), QString());
				room->throwCard(dummy2, reason, NULL);
			}
		}
		else if (event == EventPhaseChanging) {
			ServerPlayer *ex_scarlets = invoke->invoker;
			room->removePlayerMark(ex_scarlets, "hongmo", ex_scarlets->getMark("hongmo"));
		}
		return false;
	}
};

class HongmoTargetMod : public TargetModSkill
{
	
public:
	HongmoTargetMod() : TargetModSkill("#hongmo-targetmod")
	{
		pattern = "Slash";
	}
	
	int getResidueNum(const Player *ex_scarlets, const Card *) const
	{
		return ex_scarlets->getMark("hongmo");
	}
	
	int getExtraTargetNum(const Player *ex_scarlets, const Card *) const
	{
		return ex_scarlets->getMark("hongmo");
	}
};

class Jixue : public TriggerSkill
{
	
public:
	Jixue() : TriggerSkill("jixue")
	{
		events << EventPhaseStart;
		frequency = Wake;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *ex_scarlets = room->getCurrent();
		if (ex_scarlets && ex_scarlets->isAlive() && ex_scarlets->hasSkill(this) && ex_scarlets->getPhase() == Player::RoundStart
				&& ex_scarlets->getHp() <= 1 && ex_scarlets->getMark("jixue") == 0)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_scarlets, ex_scarlets, NULL, true);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->doSuperLightbox("ex_scarlets:jixue", objectName());
		ServerPlayer *ex_scarlets = invoke->invoker;
		room->addPlayerMark(ex_scarlets, objectName());
		if (room->changeMaxHpForAwakenSkill(ex_scarlets)) {
			room->acquireSkill(ex_scarlets, "shengqiang");
			room->acquireSkill(ex_scarlets, "shengxue");
		}
		return false;
	}
};

class Cizhang : public TriggerSkill
{
	
public:
	Cizhang() : TriggerSkill("cizhang")
	{
		events << ConfirmDamage;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		if (damage.to && damage.to->isAlive() && damage.to->hasSkill(this) && damage.nature == DamageStruct::Thunder)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to, NULL, true);
		else if (damage.from && damage.from->isAlive() && damage.from->hasSkill(this) && damage.nature == DamageStruct::Normal)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, NULL, true);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *ex_iku = invoke->invoker;
		if (damage.from == ex_iku) {
			room->sendCompulsoryTriggerLog(ex_iku, objectName());
			damage.nature = DamageStruct::Thunder;
			data = QVariant::fromValue(damage);
		}
		if (damage.to == ex_iku) {
			room->sendCompulsoryTriggerLog(ex_iku, objectName());
			damage.damage = 0;
			data = QVariant::fromValue(damage);
		}
		return false;
	}
};

LongzuanCard::LongzuanCard()
{
	
}

bool LongzuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void LongzuanCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	bool success = effect.from->pindian(effect.to, "longzuan", NULL);
	if (success)
		room->damage(DamageStruct("longzuan", effect.from, effect.to, 2));
	else {
		effect.from->tag["LongzuanTarget"] = QVariant::fromValue(effect.to);
		QString choice = room->askForChoice(effect.from, "longzuan", "LongzuanDraw+Cancel");
		if (choice == "LongzuanDraw")
			effect.to->drawCards(2);
	}
}

class LongzuanVS : public ZeroCardViewAsSkill
{
	
public:
	LongzuanVS() : ZeroCardViewAsSkill("longzuan")
	{
		response_pattern = "@@longzuan";
	}
	
	virtual const Card *viewAs() const
	{
		return new LongzuanCard;
	}
};

class Longzuan : public TriggerSkill
{
	
public:
	Longzuan() : TriggerSkill("longzuan")
	{
		events << EventPhaseStart;
		view_as_skill = new LongzuanVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *ex_iku = data.value<ServerPlayer *>();
		if (ex_iku && ex_iku->isAlive() && ex_iku->hasSkill(this) && ex_iku->getPhase() == Player::Finish && !ex_iku->isKongcheng())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_iku, ex_iku, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_iku = invoke->invoker;
		return room->askForUseCard(ex_iku, "@@longzuan", "@longzuan");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

THWeavePackage::THWeavePackage()
	: Package("thweave")
{
	General *yukari = new General(this, "yukari", "hakurei", 4, false);
	yukari->addSkill(new Jiejie);
	yukari->addSkill(new JiejieTM);
	yukari->addSkill(new JiejieProhibit);
	related_skills.insertMulti("jiejie", "#jiejie-targetmod");
	related_skills.insertMulti("jiejie", "#jiejie-prohibit");
	
	General *ran = new General(this, "ran", "hakurei", 3, false);
	ran->addSkill(new Xianhu);
	ran->addSkill(new Shishen);
	
	General *chen = new General(this, "chen", "hakurei", 4, false);
	chen->addSkill(new Dunjia);
	chen->addSkill(new Fengshi);
	
	General *keine = new General(this, "keine", "hakurei", 3, false);
	keine->addSkill(new Yuanqi);
	keine->addSkill(new YuanqiAdd);
	related_skills.insertMulti("yuanqi", "#yuanqi-add");
	keine->addSkill(new Mishi);
	
	General *mokou = new General(this, "mokou", "hakurei", 3, false);
	mokou->addSkill(new Miezui);
	mokou->addSkill(new Chongsheng);
	
	General *rin = new General(this, "rin", "moriya", 3, false);
	rin->addSkill(new Lingche);
	rin->addSkill(new Maoyou);
	rin->addSkill(new Shihun);
	
	General *utsuho = new General(this, "utsuho", "moriya", 4, false);
	utsuho->addSkill(new Herong);
	
	General *komachi = new General(this, "komachi", "moriya", 4, false);
	komachi->addSkill(new Molian);
	
	General *erin = new General(this, "erin", "moriya", 3, false);
	erin->addSkill(new Sijian);
	erin->addSkill(new Qionghu);
	erin->addSkill(new QionghuMaxCards);
	related_skills.insertMulti("qionghu", "#qionghu-maxcards");
	
	General *kaguya = new General(this, "kaguya", "moriya", 3, false);
	kaguya->addSkill(new Yuzhi);
	kaguya->addSkill(new Zhuqu);
	
	General *ex_scarlets = new General(this, "ex_scarlets", "god", 4, false);
	ex_scarlets->addSkill(new Hongmo);
	ex_scarlets->addSkill(new HongmoTargetMod);
	related_skills.insertMulti("hongmo", "#hongmo-targetmod");
	ex_scarlets->addSkill(new Jixue);
	
	General *ex_iku = new General(this, "ex_iku", "god", 3, false);
	ex_iku->addSkill(new Cizhang);
	ex_iku->addSkill(new Longzuan);
	
	addMetaObject<JiejieCard>();
	addMetaObject<ShishenCard>();
	addMetaObject<YuanqiCard>();
	addMetaObject<MishiCard>();
	addMetaObject<MiezuiCard>();
	addMetaObject<KuaiqingCard>();
	addMetaObject<HerongCard>();
	addMetaObject<YuzhiCard>();
	addMetaObject<LongzuanCard>();
	
	skills << new Kuaiqing;
}

ADD_PACKAGE(THWeave)
