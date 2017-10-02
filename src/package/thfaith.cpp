#include "thfaith.h"
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

class Zhouneng : public TriggerSkill
{

public:
	Zhouneng() : TriggerSkill("zhouneng")
	{
		events << EventPhaseStart;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *byakuren = data.value<ServerPlayer *>();
		if (byakuren && byakuren->isAlive() && byakuren->hasSkill(this) && byakuren->getPhase() == Player::Finish)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, byakuren, byakuren, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "Slash", "@zhouneng-use", -1, Card::MethodUse, true, objectName());
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

class Shuzui : public TriggerSkill
{
	
public:
	Shuzui() : TriggerSkill("shuzui")
	{
		events << TargetSpecified;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *byakuren = use.from;
		if (byakuren && byakuren->isAlive() && byakuren->hasSkill(this) && use.card->isKindOf("Slash")) {
			foreach (ServerPlayer *p, use.to)
				if (p && p->isAlive())
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, byakuren, byakuren, NULL, false, p);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *byakuren = invoke->invoker;
		ServerPlayer *target = invoke->preferredTarget;
		byakuren->drawCards(2);
		JudgeStruct judge;
		judge.who = target;
		judge.pattern = ".|heart,diamond";
		judge.good = true;
		judge.reason = objectName();
		room->judge(judge);
		if (judge.isGood()) {
			RecoverStruct recover;
			recover.who = byakuren;
			recover.recover = 1;
			recover.reason = objectName();
			room->recover(target, recover);
		}
		return false;
	}
};

class Shengnian : public TriggerSkill
{
	
public:
	Shengnian() : TriggerSkill("shengnian$")
	{
		events << CardsMoveOneTime;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		if (room->getTag("FirstRound").toBool())
			return QList<SkillInvokeDetail>();
		
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *from = qobject_cast<ServerPlayer *>(move.from);
		ServerPlayer *to = qobject_cast<ServerPlayer *>(move.to);
		if (from && from->isAlive() && from->getKingdom() == "hakurei" && move.card_ids.length() > 1) {
            ServerPlayer *byakuren = room->findPlayerBySkillName(objectName());
			if (byakuren && byakuren->isAlive() && byakuren->hasLordSkill(objectName()) && byakuren != from)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, byakuren, from, NULL, false, byakuren);
		} else if (to && to->isAlive() && to->getKingdom() == "hakurei" && move.card_ids.length() > 1) {
            ServerPlayer *byakuren = room->findPlayerBySkillName(objectName());
			if (byakuren && byakuren->isAlive() && byakuren->hasLordSkill(objectName()) && byakuren != to)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, byakuren, to, NULL, false, byakuren);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->invoker;
		ServerPlayer *byakuren = invoke->preferredTarget;
		room->notifySkillInvoked(player, objectName());
		QList<ServerPlayer *> to;
		to << byakuren;
		room->touhouLogmessage("#InvokeOthersSkill", player, objectName(), to);
		byakuren->drawCards(1);
		ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(byakuren), objectName());
		QList<ServerPlayer *> targets;
		targets << target;
		QString prompt = QString("@shengnian-slash:%1").arg(target->objectName());
		if (!room->askForUseSlashTo(byakuren, targets, prompt, false, true, false))
			room->askForDiscard(byakuren, objectName(), 1, 1, false, true);
		return false;
	}
};

class Feidao : public TriggerSkill
{
	
public:
	Feidao() : TriggerSkill("feidao")
	{
		events << TargetConfirming;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *sakuya = use.from;
		if (sakuya && sakuya->isAlive() && sakuya->hasSkill(this) && use.card->isKindOf("Slash")) {
			QList<ServerPlayer *> targets;
			foreach(ServerPlayer *p, room->getOtherPlayers(sakuya)) {
				if (!use.to.contains(p) && sakuya->canSlash(p, use.card, true))
					targets << p;
			}
			if (!targets.isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sakuya, sakuya, targets, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		JudgeStruct judge;
		judge.who = invoke->invoker;
		judge.pattern = ".|spade,club";
		judge.good = true;
		judge.reason = objectName();
		room->judge(judge);
		if (judge.isGood()) {
			if (judge.card->getSuitString() == "spade" || invoke->invoker->hasUsed("YoudaoCard")) {
				ServerPlayer *extra = room->askForPlayerChosen(invoke->invoker, invoke->targets, objectName());
				use.to.append(extra);
				room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, invoke->invoker->objectName(), extra->objectName());
				room->sortByActionOrder(use.to);
			}
			if (judge.card->getSuitString() == "club" || invoke->invoker->hasUsed("YoudaoCard")) {
				if (use.m_addHistory) {
                    room->addPlayerHistory(invoke->invoker, use.card->getClassName(), -1);
                    use.m_addHistory = false;
                }
			}
		}
		data = QVariant::fromValue(use);
		return false;
	}
};

YoudaoCard::YoudaoCard()
{
	target_fixed = true;
}

void YoudaoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	source->drawCards(1);
	room->setPlayerFlag(source, "InfinityAttackRange");
}

class Youdao : public OneCardViewAsSkill
{
	
public:
	Youdao() : OneCardViewAsSkill("youdao")
	{
		filter_pattern = ".|spade,club";
	}
	
	bool isEnabledAtPlay(const Player *sakuya) const
	{
		return !sakuya->hasUsed("YoudaoCard");
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		YoudaoCard *card = new YoudaoCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class THShiting : public TriggerSkill
{
	
public:
	THShiting() : TriggerSkill("thshiting")
	{
		events << EventPhaseChanging;
		frequency = Limited;
		limit_mark = "@timestop";
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		PhaseChangeStruct change = data.value<PhaseChangeStruct>();
		ServerPlayer *sakuya = change.player;
		if (sakuya && sakuya->isAlive() && sakuya->hasSkill(this) && sakuya->getMark("@timestop") > 0 && change.from == Player::Finish)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sakuya, sakuya, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->doSuperLightbox("sakuya:thshiting", "thshiting");
		ServerPlayer *sakuya = invoke->invoker;
		room->setPlayerMark(sakuya, "@timestop", 0);
		sakuya->drawCards(2);
		sakuya->gainAnExtraTurn();
		return false;
	}
};

ShanchouCard::ShanchouCard()
{
}

bool ShanchouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < 2 && !to_select->isKongcheng() && to_select != Self;
}

bool ShanchouCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
	return targets.length() == 2;
}

void ShanchouCard::onUse(Room *room, const CardUseStruct &use) const
{
    CardUseStruct card_use = use;
    ServerPlayer *player = card_use.from;

    bool hidden = (card_use.card->getTypeId() == TypeSkill && !card_use.card->willThrow());
    LogMessage log;
    log.from = player;
    if (!card_use.card->targetFixed() || card_use.to.length() > 1 || !card_use.to.contains(card_use.from))
        log.to = card_use.to;
    log.type = "#UseCard";
    log.card_str = card_use.card->toString(hidden);
    room->sendLog(log);

    QList<int> used_cards;
    QList<CardsMoveStruct> moves;
    if (card_use.card->isVirtualCard())
        used_cards.append(card_use.card->getSubcards());
    else
        used_cards << card_use.card->getEffectiveId();

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();
    Q_ASSERT(thread != NULL);
    thread->trigger(PreCardUsed, room, data);
    card_use = data.value<CardUseStruct>();

	CardMoveReason reason(CardMoveReason::S_REASON_THROW, player->objectName(), QString(), card_use.card->getSkillName(), QString());
	room->moveCardTo(this, player, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, data);
    thread->trigger(CardFinished, room, data);
}

void ShanchouCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	source->throwAllHandCards();
	ServerPlayer *from = targets.at(0);
	ServerPlayer *to = targets.at(1);
	from->pindian(to, "shanchou", NULL);
}

class Shanchou : public ZeroCardViewAsSkill
{
	
public:
	Shanchou() : ZeroCardViewAsSkill("shanchou")
	{
	}
	
	bool isEnabledAtPlay(const Player *parsee) const
	{
		return !parsee->hasUsed("ShanchouCard") && !parsee->isKongcheng();
	}
	
	const Card *viewAs() const
	{
		return new ShanchouCard;
	}
};

class ShanchouAdd : public TriggerSkill
{
	
public:
	ShanchouAdd() : TriggerSkill("#shanchou-add")
	{
		events << Pindian;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		PindianStruct *pindian = data.value<PindianStruct *>();
		ServerPlayer *from = pindian->from, *to = pindian->to, *parsee = room->findPlayerBySkillName("shanchou");
		if (parsee && parsee->isAlive() && pindian->reason == "shanchou")
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, parsee, parsee, NULL, true);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		PindianStruct *pindian = data.value<PindianStruct *>();
		ServerPlayer *from = pindian->from, *to = pindian->to, *parsee = room->findPlayerBySkillName("shanchou");
		QString choice;
		if (pindian->from_card->getNumber() == pindian->to_card->getNumber() || !from->isAlive() || !to->isAlive())
			choice = "SCGet";
		else
			choice = room->askForChoice(parsee, "shanchou", "SCDamage+SCGet", data);
		if (choice == "SCDamage") {
			if (pindian->from_card->getNumber() > pindian->to_card->getNumber())
				room->damage(DamageStruct("shanchou", from, to));
			else
				room->damage(DamageStruct("shanchou", to, from));
		}
		else if (choice == "SCGet") {
			DummyCard *dummy = new DummyCard;
			if (pindian->from_card->getNumber() <= pindian->to_card->getNumber())
				dummy->addSubcard(pindian->from_card);
			if (pindian->to_card->getNumber() <= pindian->from_card->getNumber())
				dummy->addSubcard(pindian->to_card);
			parsee->obtainCard(dummy, true);
		}
		return false;
	}
};

class Zhouyuan : public MasochismSkill
{
	
public:
	Zhouyuan() : MasochismSkill("zhouyuan")
	{
	}
	
	QList<SkillInvokeDetail> triggerable(const Room *room, const DamageStruct &damage) const
	{
		ServerPlayer *parsee = damage.to;
		if (parsee && parsee->isAlive() && parsee->hasSkill(this) && damage.from && damage.from->isAlive() && !damage.from->isNude())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, parsee, parsee, NULL, false, damage.from);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
	{
		ServerPlayer *parsee = invoke->invoker;
		ServerPlayer *from = invoke->preferredTarget;
        const Card *card = room->askForCard(from, "BasicCard", "@zhouyuan-give", QVariant(), Card::MethodNone);
		if (card) {
			parsee->obtainCard(card, true);
		}
		else {
			RecoverStruct recover;
			recover.who = from;
			recover.recover = 1;
			recover.reason = objectName();
			room->recover(parsee, recover);
		}
	}
};

JishaCard::JishaCard()
{
}

bool JishaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self;
}

void JishaCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	room->damage(DamageStruct("jisha", effect.from, effect.to));
}

class JishaVS : public ZeroCardViewAsSkill
{
	
public:
	JishaVS() : ZeroCardViewAsSkill("jisha")
	{
		response_pattern = "@@jisha";
	}
	
	const Card *viewAs() const
	{
		return new JishaCard;
	}
};

class Jisha : public TriggerSkill
{
	
public:
	Jisha() : TriggerSkill("jisha")
	{
		events << CardsMoveOneTime;
		frequency = Compulsory;
		view_as_skill = new JishaVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *yugi = qobject_cast<ServerPlayer *>(move.from);
		if (yugi && yugi->isAlive() && yugi->hasSkill(this) && move.to_place == Player::DiscardPile) {
			bool has_slash = false;
			foreach(int id, move.card_ids) {
				const Card *c = Sanguosha->getCard(id);
				if (c->isKindOf("Slash")) {
					has_slash = true;
					break;
				}
			}
			if (has_slash)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yugi, yugi, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yugi = invoke->invoker;
		room->addPlayerMark(yugi, "@sanpo", 1);
		if (yugi->getMark("@sanpo") == 3) {
			room->setPlayerMark(yugi, "@sanpo", 0);
			room->askForUseCard(yugi, "@@jisha", "@jisha");
		}
		return false;
	}
};

class Jiuyan : public TriggerSkill
{
	
public:
	Jiuyan() : TriggerSkill("jiuyan")
	{
		events << HpRecover;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		RecoverStruct recover = data.value<RecoverStruct>();
		ServerPlayer *yugi = room->findPlayerBySkillName(objectName());
		if (yugi && yugi->isAlive() && yugi != recover.to && yugi->getPhase() != Player::NotActive)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yugi, yugi, NULL, false, recover.to);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yugi = invoke->invoker, *player = invoke->preferredTarget;
		QString choice = room->askForChoice(yugi, objectName(), "JYDraw+JYLetDraw", data);
		if (choice == "JYDraw")
			yugi->drawCards(1);
		else if (choice == "JYLetDraw")
			player->drawCards(2);
		return false;
	}
};

class Huayuan : public TriggerSkill
{
	
public:
	Huayuan() : TriggerSkill("huayuan")
	{
		events << TargetSpecified << EventPhaseChanging;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == TargetSpecified) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *yuuka = room->findPlayerBySkillName(objectName());
			ServerPlayer *user = use.from;
			if (yuuka && yuuka->isAlive() && user && user->isAlive() && use.card->getSuitString() == "club" && !use.to.isEmpty()
					&& !(use.to.contains(user) && use.to.length() == 1) && (use.card->isKindOf("BasicCard") || use.card->isNDTrick())
					&& !yuuka->hasFlag("HuayuanUsed") && user->getPhase() == Player::Play)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yuuka, yuuka, NULL, false, user);
		}
		else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *yuuka = change.player;
			if (yuuka && yuuka->isAlive() && yuuka->hasSkill(this) && yuuka->hasFlag("HuayuanUsed") && change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yuuka, yuuka, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == TargetSpecified)
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yuuka = invoke->invoker;
		if (event == TargetSpecified) {
			ServerPlayer *player = invoke->preferredTarget;
			QString choice = room->askForChoice(player, objectName(), "HYDamage+HYDraw");
			if (choice == "HYDamage") {
				room->damage(DamageStruct(objectName(), yuuka, player));
				room->setPlayerCardLimitation(player, "use", "BasicCard,TrickCard", true);
			}
			else if (choice == "HYDraw") {
				yuuka->drawCards(1);
				CardUseStruct use = data.value<CardUseStruct>();
				QStringList nulls;
				foreach (ServerPlayer *p, use.to) {
					nulls << p->objectName();
				}
				use.nullified_list = nulls;
				data = QVariant::fromValue(use);
			}
			room->setPlayerFlag(yuuka, "HuayuanUsed");
		}
		else if (event == EventPhaseChanging) {
			room->setPlayerFlag(yuuka, "-HuayuanUsed");
		}
		return false;
	}
};

DiwenCard::DiwenCard()
{
}

bool DiwenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < Self->getLostHp() + 1 && to_select->hasFlag("DiwenTarget");
}

void DiwenCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets)
		room->setPlayerFlag(p, "DiwenCancel");
}

class DiwenVS : public ZeroCardViewAsSkill
{
	
public:
	DiwenVS() : ZeroCardViewAsSkill("diwen")
	{
		response_pattern = "@@diwen";
	}
	
	const Card *viewAs() const
	{
		return new DiwenCard;
	}
};

class Diwen : public TriggerSkill
{
	
public:
	Diwen() : TriggerSkill("diwen")
	{
		events << TargetSpecified << EventPhaseChanging;
		view_as_skill = new DiwenVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == TargetSpecified) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *miko = room->findPlayerBySkillName(objectName());
			if (miko && miko->isAlive() && use.to.contains(miko) && (use.to.length() > 1 || miko->getMark("@changed") > 0)
					&& use.from && use.from->isAlive() && use.from != miko && !miko->hasFlag("DiwenUsed") && use.card->isNDTrick())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, miko, miko, NULL, false, use.from);
		}
		else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *miko = room->findPlayerBySkillName(objectName());
			if (miko && miko->isAlive() && miko->hasSkill(this) && miko->hasFlag("DiwenUsed") && change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, miko, miko, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == TargetSpecified)
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == TargetSpecified) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *miko = invoke->invoker;
			ServerPlayer *user = invoke->preferredTarget;
			const Card *card = NULL;
			if (!use.card->isVirtualCard()) {
				int number = use.card->getNumber();
				if (number < 13) {
					QString pattern = ".|.|" + QString::number(number + 1) + "~13|hand";
					card = room->askForCard(miko, pattern, "@diwen-discard", data);
				}
			}
			if (card == NULL)
				user->drawCards(2);
			if (miko->getMark("@changed") == 0) {
				QStringList nulls;
				nulls << miko->objectName();
				use.nullified_list = nulls;
				data = QVariant::fromValue(use);
			}
			else {
				foreach (ServerPlayer *p, use.to)
					room->setPlayerFlag(p, "DiwenTarget");
				miko->tag["diwenCardUse"] = data;
				room->askForUseCard(miko, "@@diwen", "@diwen");
				QStringList nulls;
				foreach (ServerPlayer *p, use.to) {
					if (p->hasFlag("DiwenTarget"))
						room->setPlayerFlag(p, "-DiwenTarget");
					if (p->hasFlag("DiwenCancel")) {
						nulls << p->objectName();
						room->setPlayerFlag(p, "-DiwenCancel");
					}
				}
				use.nullified_list = nulls;
				data = QVariant::fromValue(use);
				miko->drawCards(1);
			}
			room->setPlayerFlag(miko, "DiwenUsed");
		}
		else if (event == EventPhaseChanging) {
			ServerPlayer *miko = invoke->invoker;
			room->setPlayerFlag(miko, "-DiwenUsed");
		}
		return false;
	}
};

class Chengzhao : public MasochismSkill
{
	
public:
	Chengzhao() : MasochismSkill("chengzhao")
	{
	}
	
	QList<SkillInvokeDetail> triggerable(const Room *room, const DamageStruct &damage) const
	{
		ServerPlayer *miko = damage.to;
		if (miko && miko->isAlive() && miko->hasSkill(this))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, miko, miko, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
	{
		ServerPlayer *miko = invoke->invoker;
		QString choice = room->askForChoice(miko, objectName(), "CZDraw+CZModify");
		if (choice == "CZDraw") {
			miko->drawCards(2);
			if (damage.from)
				damage.from->drawCards(1);
		}
		else if (choice == "CZModify") {
			room->addPlayerMark(miko, "@changed", 1);
			room->detachSkillFromPlayer(miko, "chengzhao");
		}
	}
};

class Lingmiao : public TriggerSkill
{
	
public:
	Lingmiao() : TriggerSkill("lingmiao$")
	{
		events << TargetConfirming;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *miko = room->findPlayerBySkillName(objectName());
		ServerPlayer *user = use.from;
		if (miko && miko->isAlive() && miko->hasLordSkill(this) && user && user->isAlive() && user != miko && user->getKingdom() == "moriya"
				&& use.card->isNDTrick() && use.to.length() == 1 && !use.to.contains(miko) && !room->isProhibited(user, miko, use.card)
				&& use.card->targetFilter(QList<const Player *>(), miko, user))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, miko, user, NULL, false, miko);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForCard(invoke->invoker, ".|heart,diamond", "@lingmiao-discard", data, objectName());
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *miko = invoke->preferredTarget;
		ServerPlayer *player = invoke->invoker;
		room->notifySkillInvoked(miko, objectName());
		QList<ServerPlayer *> to;
		to << miko;
		room->touhouLogmessage("#InvokeOthersSkill", player, objectName(), to);
		CardUseStruct use = data.value<CardUseStruct>();
		use.to.append(miko);
		room->sortByActionOrder(use.to);
		data = QVariant::fromValue(use);
		return false;
	}
};

class Jiangqu : public TriggerSkill
{
	
public:
	Jiangqu() : TriggerSkill("jiangqu")
	{
		events << EventPhaseStart;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *yoshika = data.value<ServerPlayer *>();
		if (yoshika && yoshika->isAlive() && yoshika->hasSkill(this) && yoshika->getPhase() == Player::Finish && yoshika->getHp() > 1)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yoshika, yoshika, NULL, true);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yoshika = invoke->invoker;
		room->sendCompulsoryTriggerLog(yoshika, objectName());
		room->loseHp(yoshika, 1);
		yoshika->drawCards(1);
		return false;
	}
};

ShehunCard::ShehunCard()
{
}

bool ShehunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return (targets.isEmpty() || (targets.length() < Self->getLostHp() + 1 && Self->getMark("@rotten") == 0)) && !to_select->isNude()
			&& to_select != Self && (to_select->getLostHp() < Self->getLostHp() || Self->getMark("@rotten") == 0);
}

void ShehunCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	int id = room->askForCardChosen(effect.from, effect.to, "he", "shehun");
	const Card *card = Sanguosha->getCard(id);
	effect.from->obtainCard(card, (room->getCardPlace(id) != Player::PlaceHand));
}

class Shehun : public OneCardViewAsSkill
{
	
public:
	Shehun() : OneCardViewAsSkill("shehun")
	{
		filter_pattern = ".|.|.|hand";
	}
	
	bool isEnabledAtPlay(const Player *yoshika) const
	{
		return !yoshika->hasUsed("ShehunCard");
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		ShehunCard *card = new ShehunCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Fuliao : public TriggerSkill
{
	
public:
	Fuliao() : TriggerSkill("fuliao")
	{
		events << Damage;
		frequency = Limited;
		limit_mark = "@rotten";
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *yoshika = room->findPlayerBySkillName(objectName());
		if (yoshika && yoshika->isAlive() && (yoshika == damage.from || yoshika == damage.to)
				&& yoshika->getMark("@rotten") > 0 && yoshika->getHp() == 1)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yoshika, yoshika, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yoshika = invoke->invoker;
		room->doSuperLightbox("yoshika:fuliao", "fuliao");
		room->setPlayerMark(yoshika, "@rotten", 0);
		room->loseMaxHp(yoshika, 3);
		RecoverStruct recover;
		recover.who = yoshika;
		recover.recover = yoshika->getMaxHp() - yoshika->getHp();
		recover.reason = objectName();
		room->recover(yoshika, recover);
		room->detachSkillFromPlayer(yoshika, "jiangqu");
		return false;
	}
};

YuansheCard::YuansheCard()
{
}

bool YuansheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void YuansheCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	int id = room->askForCardChosen(effect.from, effect.to, "h", "yuanshe");
	const Card *card = Sanguosha->getCard(id);
	effect.from->obtainCard(card, true);
	if (card->getType() == Sanguosha->getCard(getSubcards().at(0))->getType())
		effect.from->drawCards(1);
}

class Yuanshe : public ViewAsSkill
{
	
public:
	Yuanshe() : ViewAsSkill("yuanshe")
	{
	}
	
	bool isEnabledAtPlay(const Player *hatate) const
	{
		return !hatate->hasUsed("YuansheCard");
	}
	
	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		if (selected.isEmpty())
			return true;
		else if (selected.length() == 1)
			return to_select->getType() == selected.at(0)->getType();
		return false;
	}
	
	const Card *viewAs(const QList<const Card *> &cards) const
	{
		YuansheCard *card = new YuansheCard;
		card->addSubcards(cards);
		return card;
	}
};

class Nianxie : public TriggerSkill
{
	
public:
	Nianxie() : TriggerSkill("nianxie")
	{
		events << CardsMoveOneTime;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *hatate = qobject_cast<ServerPlayer *>(move.from);
		if (hatate && hatate->isAlive() && hatate->hasSkill(this) && hatate->getPhase() == Player::NotActive
				&& move.to_place == Player::DiscardPile && (move.reason.m_reason ^ 3) % 16 == 0) {
			bool has_basic = false;
			foreach(int id, move.card_ids) {
				const Card *card = Sanguosha->getCard(id);
				if (card->isKindOf("BasicCard")) {
					has_basic = true;
					break;
				}
			}
			if (has_basic)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, hatate, hatate, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *hatate = invoke->invoker;
		QList<int> ids = room->getNCards(3, false);
		QList<int> left = ids, basics, non_basics;
		foreach (int id, ids) {
			const Card *c = Sanguosha->getCard(id);
			if (c->isKindOf("BasicCard"))
				basics << id;
			else
				non_basics << id;
		}
		DummyCard *dummy = new DummyCard;
		int get = 0;
		do {
			room->fillAG(ids, hatate, non_basics);
			int card_id = room->askForAG(hatate, basics, true, objectName());
			if (card_id < 0) {
				room->clearAG(hatate);
				break;
			}
			get++;
			ids.removeOne(card_id);
			left.removeOne(card_id);
			basics.removeOne(card_id);
			dummy->addSubcard(card_id);
			room->clearAG(hatate);
		} while (!basics.isEmpty() && get < 2);
		if (dummy->subcardsLength() > 0) {
			room->doBroadcastNotify(QSanProtocol::S_COMMAND_UPDATE_PILE, QString::number(room->getDrawPile().length() + dummy->subcardsLength()));
			hatate->obtainCard(dummy, true);
		}
		if (!left.isEmpty())
			room->askForGuanxing(hatate, left, Room::GuanxingBothSides);
		return false;
	}
};

HuoyunCard::HuoyunCard()
{
}

bool HuoyunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && !to_select->hasFlag("HuoyunTargeted");
}

void HuoyunCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	room->setPlayerFlag(effect.to, "HuoyunExtra");
}

class HuoyunVS : public ZeroCardViewAsSkill
{
	
public:
	HuoyunVS() : ZeroCardViewAsSkill("huoyun")
	{
		response_pattern = "@@huoyun";
	}
	
	bool isEnabledAtPlay(const Player *nue) const
	{
		return false;
	}
	
	const Card *viewAs() const
	{
		return new HuoyunCard;
	}
};

class Huoyun : public TriggerSkill
{
	
public:
	Huoyun() : TriggerSkill("huoyun")
	{
		events << EventPhaseStart << TargetConfirming << EventPhaseChanging;
		view_as_skill = new HuoyunVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
            ServerPlayer *nue = data.value<ServerPlayer *>();
			if (nue && nue->isAlive() && nue->hasSkill(this) && nue->getPhase() == Player::RoundStart)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nue, nue, NULL, false);
		}
		else if (event == TargetConfirming) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *nue = use.from;
			if (nue && nue->isAlive() && nue->hasSkill(this) && ((nue->hasFlag("HuoyunRed") && use.card->isRed())
					|| (nue->hasFlag("HuoyunBlack") && use.card->isBlack())) && (use.card->isKindOf("Peach") || use.card->isKindOf("ExNihilo")))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nue, nue, NULL, false);
		}
		else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *nue = change.player;
			if (nue && nue->isAlive() && nue->hasSkill(this) && (nue->hasFlag("HuoyunRed") || nue->hasFlag("HuoyunBlack"))
					&& change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, nue, nue, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart)
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		else if (event == TargetConfirming) {
			CardUseStruct use = data.value<CardUseStruct>();
			foreach(ServerPlayer *p, room->getAlivePlayers()) {
				if (use.to.contains(p) || (use.card->isKindOf("Peach") && !p->isWounded())
						|| room->isProhibited(invoke->invoker, p, use.card))
					room->setPlayerFlag(p, "HuoyunTargeted");
			}
			invoke->invoker->tag["HuoyunUse"] = data;
			bool yes = room->askForUseCard(invoke->invoker, "@@huoyun", "@huoyun");
			foreach(ServerPlayer *p, room->getAlivePlayers()) {
				if (p->hasFlag("HuoyunTargeted"))
					room->setPlayerFlag(p, "-HuoyunTargeted");
			}
			return yes;
		}
		else
			return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *nue = invoke->invoker;
		if (event == EventPhaseStart) {
			room->showAllCards(nue);
			JudgeStruct judge;
			judge.pattern = ".";
			judge.who = nue;
			judge.reason = objectName();
			room->judge(judge);
			if (judge.card->isRed())
				room->setPlayerFlag(nue, "HuoyunRed");
			else if (judge.card->isBlack())
				room->setPlayerFlag(nue, "HuoyunBlack");
		}
		else if (event == TargetConfirming) {
			CardUseStruct use = data.value<CardUseStruct>();
			foreach (ServerPlayer *p, room->getAlivePlayers()) {
				if (p->hasFlag("HuoyunExtra")) {
					room->setPlayerFlag(p, "-HuoyunExtra");
					use.to.append(p);
				}
			}
			room->sortByActionOrder(use.to);
			data = QVariant::fromValue(use);
		}
		else {
			if (nue->hasFlag("HuoyunRed"))
				room->setPlayerFlag(nue, "-HuoyunRed");
			if (nue->hasFlag("HuoyunBlack"))
				room->setPlayerFlag(nue, "-HuoyunBlack");
		}
		return false;
	}
};

class HuoyunTM : public TargetModSkill
{
	
public:
	HuoyunTM() : TargetModSkill("#huoyun-tm")
	{
		pattern = "Slash,SingleTargetTrick";
	}
	
	int getExtraTargetNum(const Player *nue, const Card *card) const
	{
		if (card->isRed() && nue->hasFlag("HuoyunRed"))
			return 1;
		else if (card->isBlack() && nue->hasFlag("HuoyunBlack"))
			return 1;
		else
			return 0;
	}
};

ShimianCard::ShimianCard()
{
	target_fixed = true;
	will_throw = false;
	handling_method = Card::MethodNone;
}

void ShimianCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	source->addToPile("mask", getSubcards(), true);
}

class ShimianVS : public ViewAsSkill
{
	
public:
	ShimianVS() : ViewAsSkill("shimian")
	{
		response_pattern = "@@shimian";
	}
	
	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		QList<int> mask = Self->getPile("mask");
		if (mask.isEmpty() && selected.isEmpty())
			return true;
		foreach (int id, mask) {
			const Card *c = Sanguosha->getCard(id);
			if (to_select->getSuit() == c->getSuit())
				return false;
		}
		foreach (const Card *c, selected) {
			if (to_select->getSuit() == c->getSuit())
				return false;
		}
		return true;
	}
	
	const Card *viewAs(const QList<const Card *> &cards) const
	{
		ShimianCard *card = new ShimianCard;
		card->addSubcards(cards);
		return card;
	}
};

class Shimian : public TriggerSkill
{
	
public:
	Shimian() : TriggerSkill("shimian")
	{
		events << EventPhaseEnd << Damaged;
		view_as_skill = new ShimianVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseEnd) {
			ServerPlayer *kokoro = data.value<ServerPlayer *>();
			if (kokoro && kokoro->isAlive() && kokoro->hasSkill(this) && kokoro->getPhase() == Player::Play
					&& kokoro->getPile("mask").length() < 4 && !kokoro->isNude())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kokoro, kokoro, NULL, false);
		}
		else if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *kokoro = room->findPlayerBySkillName(objectName());
			if (kokoro && kokoro->isAlive() && damage.to && damage.to->isAlive() && !kokoro->getPile("mask").isEmpty())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kokoro, kokoro, NULL, false, damage.to);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *kokoro = invoke->invoker;
		if (event == EventPhaseEnd)
			return room->askForUseCard(kokoro, "@@shimian", "@shimian");
		else if (event == Damaged)
			return room->askForSkillInvoke(kokoro, objectName(), data);
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *kokoro = invoke->invoker;
		if (event == Damaged) {
			ServerPlayer *player = invoke->preferredTarget;
			QList<int> mask = kokoro->getPile("mask"), disabled;
			room->fillAG(mask, kokoro, disabled);
			kokoro->tag["ShimianDamageTag"] = data;
			int id = room->askForAG(kokoro, mask, false, objectName());
			const Card *card = Sanguosha->getCard(id);
			player->obtainCard(card, true);
			room->clearAG(kokoro);
			kokoro->drawCards(1);
		}
		return false;
	}
};

JiwuCard::JiwuCard()
{
}

bool JiwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	if (targets.isEmpty())
		return !to_select->isKongcheng() && to_select != Self;
	else if (targets.length() < getSubcards().length()) {
		foreach (const Player *p, targets) {
			if (to_select->getHandcardNum() == p->getHandcardNum())
				return false;
		}
		return !to_select->isKongcheng() && to_select != Self;
	}
	return false;
}

bool JiwuCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
	return targets.length() == getSubcards().length();
}

void JiwuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets) {
		int id = room->askForCardChosen(source, p, "h", "jiwu");
		room->throwCard(id, p, source);
		const Card *card = Sanguosha->getCard(id);
		bool diff = true;
		foreach (int i, getSubcards()) {
			const Card *c = Sanguosha->getCard(i);
			if (c->getSuit() == card->getSuit()) {
				diff = false;
				break;
			}
		}
		if (diff)
			source->drawCards(1);
	}
}

class JiwuVS : public ViewAsSkill
{
	
public:
	JiwuVS() : ViewAsSkill("jiwu")
	{
		expand_pile = "mask";
		response_pattern = "@@jiwu";
	}
	
	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        return Self->getPile("mask").contains(to_select->getEffectiveId());
    }
	
	const Card *viewAs(const QList<const Card *> &cards) const
	{
		JiwuCard *card = new JiwuCard;
		card->addSubcards(cards);
		return card;
	}
};

class Jiwu : public TriggerSkill
{
	
public:
	Jiwu() : TriggerSkill("jiwu")
	{
		events << EventPhaseStart;
		view_as_skill = new JiwuVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *kokoro = data.value<ServerPlayer *>();
		if (kokoro && kokoro->isAlive() && kokoro->hasSkill(this) && kokoro->getPhase() == Player::RoundStart
				&& !kokoro->getPile("mask").isEmpty())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kokoro, kokoro, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@jiwu", "@jiwu");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

ShixiCard::ShixiCard()
{
}

bool ShixiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && !to_select->getEquips().isEmpty();
}

void ShixiCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	int id = room->askForCardChosen(effect.from, effect.to, "e", "shixi");
	const Card *c = Sanguosha->getCard(id);
	effect.from->obtainCard(c);
	effect.to->drawCards(1);
	if (c->getSuitString() == "club")
		room->loseHp(effect.from, 1);
}

class Shixi : public ZeroCardViewAsSkill
{
	
public:
	Shixi() : ZeroCardViewAsSkill("shixi")
	{
	}
	
	bool isEnabledAtPlay(const Player *ex_yukari) const
	{
		return !ex_yukari->hasUsed("ShixiCard");
	}
	
	const Card *viewAs() const
	{
		return new ShixiCard;
	}
};

WangjieCard::WangjieCard()
{
}

bool WangjieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty();
}

void WangjieCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	QString choice;
	if (!effect.to->isWounded())
		choice = "WJDamage";
	else {
		effect.from->tag["WangjieTarget"] = QVariant::fromValue(effect.to);
		choice = room->askForChoice(effect.from, "wangjie", "WJDamage+WJRecover");
	}
	if (choice == "WJDamage")
		room->damage(DamageStruct("wangjie", effect.from, effect.to));
	else if (choice == "WJRecover") {
		RecoverStruct recover;
		recover.who = effect.from;
		recover.reason = "wangjie";
		recover.recover = 1;
		room->recover(effect.to, recover);
	}
}

class WangjieVS : public ZeroCardViewAsSkill
{
	
public:
	WangjieVS() : ZeroCardViewAsSkill("wangjie")
	{
		response_pattern = "@@wangjie";
	}
	
	const Card *viewAs() const
	{
		return new WangjieCard;
	}
};

class Wangjie : public TriggerSkill
{
	
public:
	Wangjie() : TriggerSkill("wangjie")
	{
		events << CardUsed << EventPhaseStart << EventPhaseChanging;
		view_as_skill = new WangjieVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardUsed) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *ex_yukari = use.from;
			if (ex_yukari && ex_yukari->isAlive() && ex_yukari->hasSkill(this) && !use.card->isVirtualCard()
					&& ex_yukari->getPhase() == Player::Play)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_yukari, ex_yukari, NULL, true);
		}
		else if (event == EventPhaseStart) {
			ServerPlayer *ex_yukari = data.value<ServerPlayer *>();
			if (ex_yukari && ex_yukari->isAlive() && ex_yukari->hasSkill(this) && ex_yukari->getPhase() == Player::Finish
					&& ex_yukari->getMark("shixi") > ex_yukari->getHp())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_yukari, ex_yukari, NULL, false);
		}
		else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *ex_yukari = change.player;
			if (ex_yukari && ex_yukari->isAlive() && ex_yukari->hasSkill(this) && change.from == Player::Finish
					&& ex_yukari->getMark("shixi") > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_yukari, ex_yukari, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_yukari = data.value<ServerPlayer *>();
		if (event == EventPhaseStart)
			return room->askForUseCard(ex_yukari, "@@wangjie", "@wangjie");
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_yukari = invoke->invoker;
		if (event == CardUsed)
			room->addPlayerMark(ex_yukari, "shixi", 1);
		else if (event == EventPhaseChanging)
			room->setPlayerMark(ex_yukari, "shixi", 0);
		return false;
	}
};

class Shengchun : public TriggerSkill
{
	
public:
	Shengchun() : TriggerSkill("shengchun")
	{
		events << DamageInflicted;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *ex_lily = damage.to;
		if (ex_lily && ex_lily->isAlive() && ex_lily->hasSkill(this) && damage.damage > 1)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_lily, ex_lily, NULL, true);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->sendCompulsoryTriggerLog(invoke->invoker, objectName());
		QList<ServerPlayer *> to;
		to.append(invoke->invoker);
		room->touhouLogmessage("#Shengchun", invoke->invoker, objectName(), to, "");
		return true;
	}
};

class ShengchunMC : public MaxCardsSkill
{
	
public:
	ShengchunMC() : MaxCardsSkill("#shengchun-mc")
	{
	}
	
	int getExtra(const Player *ex_lily) const
	{
		if (ex_lily->hasSkill("shengchun"))
			return 2;
		return 0;
	}
};

/*class Huaying : public OneCardViewAsSkill
{
	
public:
	Huaying() : OneCardViewAsSkill("huaying")
	{
		response_or_use = true;
	}
	
	bool isEnabledAtPlay(const Player *) const
	{
		return false;
	}
	
	bool isEnabledAtResponse(const Player *ex_lily, const QString &pattern) const
	{
		return (pattern == "jink" || pattern == "nullification") && !ex_lily->isKongcheng();
	}
	
	bool isEnabledAtNullification(const ServerPlayer *ex_lily) const
	{
		foreach (const Card *c, ex_lily->getHandcards()) {
			if (c->isBlack())
				return true;
		}
		return false;
	}
	
	bool viewFilter(const QList<const Card *> &, const Card *to_select) const
	{
		if (to_select->isEquipped())
			return false;
		
		QString pattern = Sanguosha->getCurrentCardUsePattern();
		if (pattern == "jink")
			return to_select->isRed();
		else if (pattern == "nullification")
			return to_select->isBlack();
		return false;
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		if (originalCard->isRed()) {
			Jink *jink = new Jink(Card::SuitToBeDecided, 0);
			jink->addSubcard(originalCard);
			jink->setSkillName(objectName());
			return jink;
		}
		else if (originalCard->isBlack()) {
			Nullification *nulli = new Nullification(Card::SuitToBeDecided, 0);
            nulli->addSubcard(originalCard);
            nulli->setSkillName(objectName());
            return nulli;
		}
		return NULL;
	}
};

class Huaying : public TriggerSkill
{
	
public:
	Huaying() : TriggerSkill("huaying")
	{
		events << CardAsked << TrickCardCanceling << EventPhaseChanging;
		view_as_skill = new HuayingVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		QList<SkillInvokeDetail> d;
		if (event == CardAsked) {
			CardAskedStruct ask = data.value<CardAskedStruct>();
			ServerPlayer *ex_lily = ask.player;
			if (ex_lily && ex_lily->isAlive() && ex_lily->hasSkill(this) && matchAvaliablePattern("jink", ask.pattern)
					&& !ex_lily->isKongcheng() && !ex_lily->hasFlag("CanHuaying"))
				return d << SkillInvokeDetail(this, ex_lily, ex_lily, NULL, false);
		}
		else if (event == TrickCardCanceling) {
			CardEffectStruct effect = data.value<CardEffectStruct>();
			ServerPlayer *ex_lily = room->findPlayerBySkillName(objectName());
			if (ex_lily && ex_lily->isAlive() && !ex_lily->hasFlag("CanHuaying"))
				return d << SkillInvokeDetail(this, ex_lily, ex_lily, NULL, false);
		}
		else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *ex_lily = room->findPlayerBySkillName(objectName());
			if (ex_lily && ex_lily->isAlive() && ex_lily->hasFlag("CanHuaying") && change.from == Player::Finish)
				return d << SkillInvokeDetail(this, ex_lily, ex_lily, NULL, true);
		}
		return d;
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == CardAsked || event == TrickCardCanceling)
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_lily = invoke->invoker;
		if (event == CardAsked || event == TrickCardCanceling) {
			room->showAllCards(ex_lily);
			bool has_black = false, has_red = false;
			foreach (const Card *c, ex_lily->getHandcards()) {
				if (!has_black && c->isBlack())
					has_black = true;
				if (!has_red && c->isRed())
					has_red = true;
				if (has_black && has_red)
					break;
			}
			if (has_black && has_red)
				room->setPlayerFlag(ex_lily, "CanHuaying");
		}
		else if (event == EventPhaseChanging)
			room->setPlayerFlag(ex_lily, "-CanHuaying");
		return false;
	}
};*/

class Huaying : public TriggerSkill
{
	
public:
	Huaying() : TriggerSkill("huaying")
	{
		events << Dying;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DyingStruct dying = data.value<DyingStruct>();
		ServerPlayer *ex_lily = dying.who;
		if (ex_lily && ex_lily->hasSkill(this))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_lily, ex_lily, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		JudgeStruct judge;
		ServerPlayer *ex_lily = invoke->invoker;
		judge.who = ex_lily;
		judge.pattern = ".|spade,club";
		judge.good = true;
		judge.reason = objectName();
		room->judge(judge);
		if (judge.isGood()) {
			RecoverStruct recover;
			recover.who = ex_lily;
			recover.reason = objectName();
			recover.recover = 1;
			room->recover(ex_lily, recover);
		}
		else
			ex_lily->drawCards(3);
		return false;
	}
};

THFaithPackage::THFaithPackage()
	: Package("thfaith")
{
	General *byakuren = new General(this, "byakuren$", "hakurei", 3, false);
	byakuren->addSkill(new Zhouneng);
	byakuren->addSkill(new Shuzui);
	byakuren->addSkill(new Shengnian);
	
	General *sakuya = new General(this, "sakuya", "hakurei", 4, false);
	sakuya->addSkill(new Feidao);
	sakuya->addSkill(new Youdao);
	sakuya->addSkill(new THShiting);
	
	General *parsee = new General(this, "parsee", "hakurei", 3, false);
	parsee->addSkill(new Shanchou);
	parsee->addSkill(new ShanchouAdd);
	related_skills.insertMulti("shanchou", "#shanchou-add");
	parsee->addSkill(new Zhouyuan);
	
	General *yugi = new General(this, "yugi", "hakurei", 4, false);
	yugi->addSkill(new Jisha);
	yugi->addSkill(new Jiuyan);
	
	General *yuuka = new General(this, "yuuka", "hakurei", 4, false);
	yuuka->addSkill(new Huayuan);
	
	General *miko = new General(this, "miko$", "moriya", 3, false);
	miko->addSkill(new Diwen);
	miko->addSkill(new Chengzhao);
	miko->addSkill(new Lingmiao);
	
	General *yoshika = new General(this, "yoshika", "moriya", 6, false);
	yoshika->addSkill(new Jiangqu);
	yoshika->addSkill(new Shehun);
	yoshika->addSkill(new Fuliao);
	
	General *hatate = new General(this, "hatate", "moriya", 3, false);
	hatate->addSkill(new Yuanshe);
	hatate->addSkill(new Nianxie);
	
	General *nue = new General(this, "nue", "moriya", 4, false);
	nue->addSkill(new Huoyun);
	nue->addSkill(new HuoyunTM);
	related_skills.insertMulti("huoyun", "#huoyun-tm");
	nue->addSkill(new Skill("hengong", Skill::Compulsory));
	
	General *kokoro = new General(this, "kokoro", "moriya", 3, false);
	kokoro->addSkill(new Shimian);
	kokoro->addSkill(new Jiwu);
	
	General *ex_yukari = new General(this, "ex_yukari", "god", 4, false);
	ex_yukari->addSkill(new Shixi);
	ex_yukari->addSkill(new Wangjie);
	
	General *ex_lily = new General(this, "ex_lily", "god", 2, false);
	ex_lily->addSkill(new Shengchun);
	ex_lily->addSkill(new ShengchunMC);
	related_skills.insertMulti("shengchun", "#shengchun-mc");
	ex_lily->addSkill(new Huaying);
	
	addMetaObject<YoudaoCard>();
	addMetaObject<ShanchouCard>();
	addMetaObject<JishaCard>();
	addMetaObject<DiwenCard>();
	addMetaObject<ShehunCard>();
	addMetaObject<YuansheCard>();
	addMetaObject<HuoyunCard>();
	addMetaObject<ShimianCard>();
	addMetaObject<JiwuCard>();
	addMetaObject<ShixiCard>();
	addMetaObject<WangjieCard>();
}

ADD_PACKAGE(THFaith)

