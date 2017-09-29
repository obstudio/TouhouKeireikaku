#include "thshadow.h"
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

LiuyueCard::LiuyueCard()
{
}

bool LiuyueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty())
        return false;

    if (to_select->hasFlag("LiuyueSlashSource") || to_select == Self)
        return false;

    const Player *from = NULL;
    foreach (const Player *p, Self->getAliveSiblings()) {
        if (p->hasFlag("LiuyueSlashSource")) {
            from = p;
            break;
        }
    }

    const Card *slash = Card::Parse(Self->property("liuyue").toString());
    if (from && !from->canSlash(to_select, slash, false))
        return false;

    return Self->inMyAttackRange(to_select);
}

void LiuyueCard::onEffect(const CardEffectStruct &effect) const
{
    effect.to->setFlags("LiuyueTarget");
}

class LiuyueVS : public OneCardViewAsSkill
{
public:
    LiuyueVS() : OneCardViewAsSkill("liuyue")
    {
        filter_pattern = ".!";
        response_pattern = "@@liuyue";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        LiuyueCard *card = new LiuyueCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Liuyue : public TriggerSkill
{
public:
    Liuyue() : TriggerSkill("liuyue")
    {
        events << TargetConfirming;
        view_as_skill = new LiuyueVS;
    }

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *daiyousei = room->findPlayerBySkillName(objectName());
		if (daiyousei && daiyousei->isAlive() && use.to.contains(daiyousei) && use.card->isKindOf("Slash")
				&& daiyousei->canDiscard(daiyousei, "he")) {
            QList<ServerPlayer *> players = room->getOtherPlayers(daiyousei);
            players.removeOne(use.from);
			bool can_invoke = false;
            foreach (ServerPlayer *p, players) {
                if (use.from->canSlash(p, use.card, false) && daiyousei->inMyAttackRange(p)) {
                    can_invoke = true;
                    break;
                }
            }
			if (can_invoke)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, daiyousei, daiyousei, NULL, false, use.from);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
        CardUseStruct use = data.value<CardUseStruct>();
		QString prompt = "@liuyue:" + use.from->objectName();
		ServerPlayer *daiyousei = invoke->invoker;
		daiyousei->tag["liuyue-card"] = QVariant::fromValue(use.card);
		ServerPlayer *source = invoke->preferredTarget;
		if (source)
			room->setPlayerFlag(source, "LiuyueSlashSource");
		bool yes = room->askForUseCard(daiyousei, "@@liuyue", prompt);
		if (source)
			room->setPlayerFlag(source, "-LiuyueSlashSource");
		daiyousei->tag.remove("liuyue-card");
		return yes;
	}
	
    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
		ServerPlayer *daiyousei = invoke->invoker;
        CardUseStruct use = data.value<CardUseStruct>();
        QList<ServerPlayer *> players = room->getOtherPlayers(daiyousei);
        players.removeOne(use.from);
		foreach (ServerPlayer *p, players) {
			if (p->hasFlag("LiuyueTarget")) {
				p->setFlags("-LiuyueTarget");
				if (!use.from->canSlash(p, false))
					return false;
				use.to.removeOne(daiyousei);
				use.to.append(p);
				room->sortByActionOrder(use.to);
                data = QVariant::fromValue(use);
				return false;
			}
		}
        return false;
    }
};

class Fangmu : public TriggerSkill
{

public:
	Fangmu() : TriggerSkill("fangmu")
	{
		events << DamageCaused;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *daiyousei = room->findPlayerBySkillName(objectName());
		if (daiyousei && daiyousei->isAlive() && daiyousei != damage.to && damage.damage > 0)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, daiyousei, daiyousei, NULL, false, damage.to);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *daiyousei = invoke->invoker;
		return room->askForSkillInvoke(daiyousei, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *daiyousei = invoke->invoker;
		room->loseHp(daiyousei, 1);
		return true;
	}
};

class Yaodao : public OneCardViewAsSkill
{
	
public:
	Yaodao() : OneCardViewAsSkill("yaodao")
	{
		filter_pattern = "BasicCard";
	}
	
	bool isEnabledAtPlay(const Player *target) const
	{
		return !target->hasFlag("YaodaoUsed");
	}
	
	const Card *viewAs(const Card *originalCard) const
	{
		Card *slash = new SoulSlash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard->getEffectiveId());
		slash->setSkillName("yaodao");
		return slash;
	}
};

class YaodaoAdd : public TriggerSkill
{
	
public:
	YaodaoAdd() : TriggerSkill("#yaodao-add")
	{
		events << CardFinished << EventPhaseEnd;
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardFinished) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *youmu = use.from;
			if (youmu && youmu->isAlive() && youmu->hasSkill(this)) {
				if (use.card->getSkillName() == "yaodao" || use.card->isKindOf("SoulSlash"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, youmu, youmu, NULL, true);
			}
		}
		else if (event == EventPhaseEnd) {
			ServerPlayer *youmu = data.value<ServerPlayer *>();
			if (youmu && youmu->isAlive() && youmu->hasFlag("YaodaoUsed") && youmu->getPhase() == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, youmu, youmu, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == CardFinished) {
			ServerPlayer *youmu = invoke->invoker;
			CardUseStruct use = data.value<CardUseStruct>();
			if (use.card->getSkillName() == "yaodao")
				room->setPlayerFlag(youmu, "YaodaoUsed");
			if (use.card->isKindOf("SoulSlash")) {
				if (use.m_addHistory) {
					room->addPlayerHistory(youmu, use.card->getClassName(), -1);
					use.m_addHistory = false;
					data = QVariant::fromValue(use);
				}
			}
		}
		else {
			ServerPlayer *youmu = invoke->invoker;
			room->setPlayerFlag(youmu, "-YaodaoUsed");
		}
		return false;
	}
};

class THBanling : public TriggerSkill
{
	
public:
	THBanling() : TriggerSkill("thbanling")
	{
		events << Damaged;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *youmu = damage.to;
		if (youmu && youmu->isAlive() && youmu->hasSkill(this))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, youmu, youmu, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *youmu = invoke->invoker;
		return room->askForSkillInvoke(youmu, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *youmu = invoke->invoker;
		QList<int> card_ids = room->getNCards(4, false);
		QList<int> enabled, disabled;
		foreach (int id, card_ids) {
			enabled << id;
		}
		DummyCard *dummy = new DummyCard;
		int id = 0, get = 0;
		while (id >= 0 && get < youmu->getLostHp()) {
			room->fillAG(card_ids, youmu, disabled);
			id = room->askForAG(youmu, enabled, true, objectName());
			if (id == -1) {
				room->clearAG(youmu);
				break;
			}
			get++;
			dummy->addSubcard(id);
			enabled.removeOne(id);
			disabled.append(id);
			room->clearAG(youmu);
		}
		if (!dummy->getSubcards().isEmpty()) {
			ServerPlayer *target = room->askForPlayerChosen(youmu, room->getAlivePlayers(), objectName());
			target->obtainCard(dummy, false);
        }
		return false;
    }
};

class Huadie : public TriggerSkill
{
	
public:
	Huadie() : TriggerSkill("huadie")
	{
		events << Damaged << EventPhaseChanging;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *yuyuko = room->findPlayerBySkillName(objectName());
			if (yuyuko && yuyuko->isAlive() && damage.from && damage.from->isAlive() && damage.from != yuyuko
					&& damage.nature != DamageStruct::Normal && yuyuko->getMark("huadie") < 2)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yuyuko, yuyuko, NULL, false, damage.from);
		}
		else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *yuyuko = room->findPlayerBySkillName(objectName());
			if (yuyuko && yuyuko->isAlive() && change.from == Player::Finish && yuyuko->getMark("huadie") > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yuyuko, yuyuko, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant&data) const
	{
		if (event == Damaged)
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == Damaged) {
			ServerPlayer *yuyuko = invoke->invoker;
			ServerPlayer *source = invoke->preferredTarget;
			yuyuko->tag["HuadieTarget"] = QVariant::fromValue(source);
			QStringList choices;
			choices << "HuadieDraw";
			if (!source->isKongcheng())
				choices << "HuadieThrow";
			QString choice = room->askForChoice(yuyuko, objectName(), choices.join("+"));
			if (choice == "HuadieDraw")
				source->drawCards(1);
			else if (choice == "HuadieThrow") {
				int id = room->askForCardChosen(yuyuko, source, "h", objectName());
				room->throwCard(id, source, yuyuko);
			}
			DamageStruct damage = data.value<DamageStruct>();
			if (damage.nature == DamageStruct::Soul)
				yuyuko->drawCards(1);
			room->addPlayerMark(yuyuko, "huadie", 1);
		}
		else
			room->setPlayerMark(invoke->invoker, "huadie", 0);
		return false;
	}
};

LieCard::LieCard()
{
	will_throw = false;
}

bool LieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select->hasFlag("LieTarget");
}

void LieCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	effect.to->obtainCard(this, false);
	if (effect.to->getMark("lie") == 0)
		room->addPlayerMark(effect.to, "lie", 1);
}

class LieVS : public OneCardViewAsSkill
{
	
public:
	LieVS() : OneCardViewAsSkill("lie")
	{
		filter_pattern = ".|.|.|hand";
		response_pattern = "@@lie";
	}
	
	virtual const Card *viewAs(const Card *originalCard) const
	{
		LieCard *card = new LieCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Lie : public TriggerSkill
{
	
public:
	Lie() : TriggerSkill("lie")
	{
		events << Damaged;
		view_as_skill = new LieVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *hina = room->findPlayerBySkillName(objectName());
		if (hina && hina->isAlive() && damage.to && damage.to->isAlive() && hina != damage.to && !hina->isKongcheng())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, hina, hina, NULL, false, damage.to);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *hina = invoke->invoker;
		room->setPlayerFlag(invoke->preferredTarget, "LieTarget");
		QString prompt = QString("@lie:%1").arg(invoke->preferredTarget->objectName());
		bool yes = room->askForUseCard(hina, "@@lie", prompt);
		room->setPlayerFlag(invoke->preferredTarget, "-LieTarget");
		return yes;
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *hina = invoke->invoker;
		if (hina->isKongcheng()) {
			RecoverStruct recover;
			recover.recover = 1;
			recover.who = hina;
			recover.reason = objectName();
			room->recover(hina, recover);
		}
		return false;
	}
};

class Anyun : public TriggerSkill
{
	
public:
	Anyun() : TriggerSkill("anyun")
	{
		events << TargetSpecified;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *hina = room->findPlayerBySkillName(objectName());
		if (hina && hina->isAlive() && use.to.contains(hina) && use.card->isBlack() && !use.card->isVirtualCard()) {
			foreach (ServerPlayer *p, room->getOtherPlayers(hina)) {
				if (p->getHandcardNum() < hina->getHandcardNum())
					return QList<SkillInvokeDetail>();
			}
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, hina, hina, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *hina = invoke->invoker;
		return room->askForSkillInvoke(hina, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *hina = invoke->invoker;
		CardUseStruct use = data.value<CardUseStruct>();
		int x;
		if (!use.from || use.from->getMark("lie") == 0)
			x = 1;
		else
			x = 2;
		hina->drawCards(x);
		return false;
	}
};

JulangCard::JulangCard()
{
}

bool JulangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && !to_select->getEquips().isEmpty() && to_select != Self;
}

void JulangCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	QString choice = room->askForChoice(effect.to, "julang", "JulangThrow+JulangDamage");
	if (choice == "JulangThrow")
		effect.to->throwAllEquips();
	else if (choice == "JulangDamage")
		room->damage(DamageStruct("julang", effect.from, effect.to));
}

class Julang : public ViewAsSkill
{
	
public:
	Julang() : ViewAsSkill("julang")
	{
	}
	
	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return selected.length() < 2;
	}
	
	virtual bool isEnabledAtPlay(const Player *target) const
	{
		return !target->hasUsed("JulangCard");
	}
	
	virtual const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() != 2)
			return NULL;
		JulangCard *card = new JulangCard;
		card->addSubcards(cards);
		return card;
	}
};

BamaoCard::BamaoCard()
{
	
}

bool BamaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void BamaoCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	room->doSuperLightbox("suimitsu:bamao", "bamao");
	room->removePlayerMark(effect.from, "@anchor", 1);
	DummyCard *dummy = new DummyCard;
	foreach (const Card *card, effect.to->getHandcards()) {
		dummy->addSubcard(card);
	}
    effect.from->obtainCard(dummy, false);
	room->setPlayerFlag(effect.to, "BamaoTarget");
}

class BamaoVS : public ZeroCardViewAsSkill
{
	
public:
	BamaoVS() : ZeroCardViewAsSkill("bamao")
	{
		response_pattern = "@@bamao";
	}
	
	virtual const Card *viewAs() const
	{
		return new BamaoCard;
	}
};

class Bamao : public TriggerSkill
{
	
public:
	Bamao() : TriggerSkill("bamao")
	{
		events << DrawNCards << DamageInflicted << EventPhaseChanging << Death;
		view_as_skill = new BamaoVS;
		frequency = Limited;
		limit_mark = "@anchor";
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == DrawNCards) {
			ServerPlayer *suimitsu = room->getCurrent();
			if (suimitsu && suimitsu->isAlive() && suimitsu->hasSkill(this) && suimitsu->getMark("@anchor") > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, suimitsu, suimitsu, NULL, false);
		}
		else if (event == DamageInflicted) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *suimitsu = room->findPlayerBySkillName(objectName());
			if (damage.to && damage.to->isAlive() && damage.to->hasFlag("BamaoTarget"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, suimitsu, damage.to, NULL, true);
		}
		else if (event == EventPhaseChanging) {
			ServerPlayer *suimitsu = data.value<ServerPlayer *>();
			if (suimitsu && suimitsu->isAlive() && suimitsu->hasSkill(this) && suimitsu->getPhase() == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, suimitsu, suimitsu, NULL, true);
		}
		else if (event == Death) {
            DeathStruct death = data.value<DeathStruct>();
			ServerPlayer *suimitsu = death.who;
			if (suimitsu && suimitsu->hasSkill(this))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, suimitsu, suimitsu, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == DrawNCards)
			return room->askForUseCard(invoke->invoker, "@@bamao", "@bamao");
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == DrawNCards) {
			ServerPlayer *suimitsu = invoke->invoker;
			DrawNCardsStruct qnum = data.value<DrawNCardsStruct>();
			qnum.n = 0;
			data = QVariant::fromValue(qnum);
		}
		else if (event == DamageInflicted) {
			QList<ServerPlayer *> to;
			to << invoke->invoker;
			room->touhouLogmessage("#Bamao", invoke->owner, objectName(), to, "");
			return true;
		}
		else if (event == EventPhaseChanging || event == Death) {
			foreach (ServerPlayer *p, room->getAlivePlayers()) {
				if (p->hasFlag("BamaoTarget"))
					room->setPlayerFlag(p, "-BamaoTarget");
			}
		}
		return false;
	}
};

class Mianxian : public TriggerSkill
{
	
public:
	Mianxian() : TriggerSkill("mianxian")
	{
		events << EventPhaseStart;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *player = data.value<ServerPlayer *>();
		ServerPlayer *prismriver = room->findPlayerBySkillName(objectName());
		if (prismriver && prismriver->isAlive() && player && player->getPhase() == Player::RoundStart && prismriver->faceUp()
				&& !player->getCards("ej").isEmpty())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, prismriver, prismriver, NULL, false, player);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->preferredTarget;
		ServerPlayer *prismriver = invoke->invoker;
		int id = room->askForCardChosen(prismriver, player, "ej", objectName());
		const Card *card = Sanguosha->getCard(id);
		prismriver->obtainCard(card, true);
		JudgeStruct judge;
		judge.who = prismriver;
		judge.pattern = ".|heart,diamond";
		judge.good = true;
		judge.reason = objectName();
		room->judge(judge);
		if (judge.isGood())
			prismriver->drawCards(1);
		else
			prismriver->turnOver();
		return false;
	}
};

class Dianyin : public TriggerSkill
{
	
public:
    Dianyin() : TriggerSkill("dianyin")
	{
		events << ConfirmDamage;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *prismriver = room->findPlayerBySkillName(objectName());
		if (prismriver && prismriver->isAlive() && prismriver != damage.from && prismriver != damage.to
				&& damage.nature == DamageStruct::Thunder && damage.damage > 0)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, prismriver, prismriver, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *prismriver = invoke->invoker;
		damage.from = prismriver;
		data = QVariant::fromValue(damage);
		prismriver->drawCards(damage.damage);
		return false;
	}
};

YingchongCard::YingchongCard()
{
	target_fixed = true;
	will_throw = false;
	handling_method = Card::MethodNone;
}

void YingchongCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    foreach (int id, getSubcards()) {
		room->showCard(source, id);
	}
	ServerPlayer *wriggle = room->findPlayerBySkillName("yingchong");
	QString choice;
	if (source->getHandcardNum() == getSubcards().length())
		choice = "YCThrow";
	else {
		wriggle->tag["YingchongCards"] = QVariant::fromValue(getEffectiveId());
		choice = room->askForChoice(wriggle, "yingchong", "YCThrow+YCSnatch");
	}
	if (choice == "YCThrow") {
		int x = 0;
		foreach(int id, getSubcards()) {
			const Card *c = Sanguosha->getCard(id);
			if (c->isRed())
				x++;
		}
		room->setPlayerMark(wriggle, "yingchong", x);
		room->setPlayerFlag(wriggle, "YingchongDraw");
		room->throwCard(this, source, wriggle);
	}
	else if (choice == "YCSnatch") {
		room->setPlayerMark(wriggle, "yingchong", source->getHandcardNum() - getSubcards().length());
		room->setPlayerFlag(wriggle, "YingchongMax");
        const Card *c = source->getRandomHandCard();
		while (!(source->getHandcards().contains(c) && !getSubcards().contains(c->getEffectiveId())))
            c = source->getRandomHandCard();
		room->obtainCard(wriggle, c, false);
	}
}

class YingchongVS : public ViewAsSkill
{
	
public:
	YingchongVS() : ViewAsSkill("yingchong")
	{
		response_pattern = "@@yingchong";
	}
	
	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return !to_select->isEquipped();
	}
	
	virtual const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (!cards.isEmpty()) {
			YingchongCard *card = new YingchongCard;
			card->addSubcards(cards);
			return card;
		}
		else
			return NULL;
	}
};

class Yingchong : public TriggerSkill
{
	
public:
	Yingchong() : TriggerSkill("yingchong")
	{
		events << EventPhaseStart << DrawNCards << EventPhaseChanging;
		view_as_skill = new YingchongVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *wriggle = data.value<ServerPlayer *>();
			if (wriggle && wriggle->isAlive() && wriggle->hasSkill(this) && wriggle->getPhase() == Player::RoundStart) {
				QList<ServerPlayer *> targets;
				foreach(ServerPlayer *p, room->getOtherPlayers(wriggle)) {
					if (p->getHandcardNum() > wriggle->getHandcardNum())
						targets << p;
				}
                if (!targets.isEmpty())
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, wriggle, wriggle, targets, false);
			}
		}
		else if (event == DrawNCards) {
			ServerPlayer *wriggle = room->getCurrent();
			if (wriggle && wriggle->isAlive() && wriggle->hasFlag("YingchongDraw"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, wriggle, wriggle, NULL, true);
		}
		else if (event == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *wriggle = change.player;
			if (wriggle && wriggle->isAlive() && (wriggle->hasFlag("YingchongDraw") || wriggle->hasFlag("YingchongMax"))
					&& change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, wriggle, wriggle, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart)
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		else
			return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *wriggle = invoke->invoker;
			ServerPlayer *target = room->askForPlayerChosen(wriggle, invoke->targets, objectName());
			room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, wriggle->objectName(), target->objectName());
			wriggle->tag["YingchongTarget"] = QVariant::fromValue(target);
			room->askForUseCard(target, "@@yingchong", "@yingchong");
		}
		else if (event == DrawNCards) {
			DrawNCardsStruct qnum = data.value<DrawNCardsStruct>();
			ServerPlayer *wriggle = invoke->invoker;
			qnum.n = wriggle->getMark("yingchong");
			data = QVariant::fromValue(qnum);
		}
		else if (event == EventPhaseChanging) {
			ServerPlayer *wriggle = invoke->invoker;
			if (wriggle->hasFlag("YingchongDraw"))
				room->setPlayerFlag(wriggle, "-YingchongDraw");
			if (wriggle->hasFlag("YingchongMax"))
				room->setPlayerFlag(wriggle, "-YingchongMax");
		}
		return false;
	}
};

class YingchongMax : public MaxCardsSkill
{
	
public:
	YingchongMax() : MaxCardsSkill("#yingchong-max")
	{
		
	}
	
	int getFixed(const Player *target) const
	{
		if (target->hasFlag("YingchongMax"))
			return target->getMark("yingchong");
		return -1;
	}
};

AnchaoCard::AnchaoCard()
{
	
}

bool AnchaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->hasFlag("ValidAnchaoTarget");
}

void AnchaoCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *from = effect.from;
	ServerPlayer *to = effect.to;
	Room *room = from->getRoom();
	CardUseStruct use = from->tag["AnchaoData"].value<CardUseStruct>();
	to->obtainCard(use.card, true);
	room->addPlayerMark(from, "@anchao");
}

class AnchaoVS : public ZeroCardViewAsSkill
{
	
public:
	AnchaoVS() : ZeroCardViewAsSkill("anchao")
	{
		response_pattern = "@@anchao";
	}
	
	const Card *viewAs() const
	{
		return new AnchaoCard;
	}
};

class Anchao : public TriggerSkill
{
	
public:
	Anchao() : TriggerSkill("anchao")
	{
		events << CardFinished << EventPhaseChanging;
		view_as_skill = new AnchaoVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardFinished) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *wriggle = room->findPlayerBySkillName(objectName());
			if (wriggle && wriggle->isAlive() && use.card->isNDTrick() && use.card->isBlack() && use.from && !use.to.isEmpty()
					&& wriggle->getMark("@anchao") == 0) {
				QList<ServerPlayer *> targets;
				foreach(ServerPlayer *p, room->getOtherPlayers(use.from)) {
					if (!use.to.contains(p))
						targets << p;
				}
				if (!targets.isEmpty())
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, wriggle, wriggle, NULL, false);
			}
		}
		else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *wriggle = change.player;
			if (wriggle && wriggle->isAlive() && wriggle->hasSkill(this) && change.to == Player::RoundStart
					&& wriggle->getMark("@anchao") > 0)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, wriggle, wriggle, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == CardFinished) {
			invoke->invoker->tag["AnchaoData"] = QVariant::fromValue(data);
			CardUseStruct use = data.value<CardUseStruct>();
			foreach (ServerPlayer *p, room->getAlivePlayers())
				if (!use.to.contains(p) && p != use.from)
					room->setPlayerFlag(p, "ValidAnchaoTarget");
			bool yes = room->askForUseCard(invoke->invoker, "@@anchao", "@anchao");
			foreach (ServerPlayer *p, room->getAlivePlayers())
				if (p->hasFlag("ValidAnchaoTarget"))
					room->setPlayerFlag(p, "-ValidAnchaoTarget");
			return yes;
		}
		return true;
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (event == EventPhaseChanging) {
			room->removePlayerMark(invoke->invoker, "@anchao", 1);
		}
		return false;
	}
};

class Zonghuo : public TriggerSkill
{
	
public:
	Zonghuo() : TriggerSkill("zonghuo")
	{
		events << ConfirmDamage;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *futo = room->findPlayerBySkillName(objectName());
		if (futo && futo->isAlive() && damage.nature == DamageStruct::Normal && damage.to->isChained() && !futo->isNude())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, futo, futo, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForCard(invoke->invoker, ".|heart", "@zonghuo-discard", data, objectName());
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		damage.to->tag["ZonghuoDamage"] = data;
		QString choice = room->askForChoice(damage.to, objectName(), "ZHAdd+ZHFire");
		if (choice == "ZHAdd")
			damage.damage++;
		else if (choice == "ZHFire")
			damage.nature = DamageStruct::Fire;
		data = QVariant::fromValue(damage);
		return false;
	}
};

JinpanCard::JinpanCard()
{
	
}

bool JinpanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	IronChain *chain = new IronChain(NoSuit, 0);
	chain->deleteLater();
	return targets.length() < Self->getLostHp() && chain->targetFilter(targets, to_select, Self) && !Sanguosha->isProhibited(Self, to_select, chain, targets);
}

void JinpanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	IronChain *chain = new IronChain(NoSuit, 0);
	chain->setSkillName("jinpan");
	CardUseStruct use;
	use.from = source;
	use.to = targets;
	use.card = chain;
	foreach(ServerPlayer *p, targets) {
		const Card *card = room->askForCard(p, ".|heart|.|hand", "@jinpan-heart", QVariant(), Card::MethodNone);
		if (card) {
			room->showCard(p, card->getEffectiveId());
			if (p != source)
				source->obtainCard(card, true);
			use.nullified_list << p->objectName();
		}
	}
	room->useCard(use);
}

class JinpanVS : public ZeroCardViewAsSkill
{
	
public:
    JinpanVS() : ZeroCardViewAsSkill("jinpan")
	{
		response_pattern = "@@jinpan";
	}
	
	virtual const Card *viewAs() const
	{
		return new JinpanCard;
	}
};

class Jinpan : public TriggerSkill
{
	
public:
	Jinpan() : TriggerSkill("jinpan")
	{
		events << EventPhaseStart;
		view_as_skill = new JinpanVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *futo = data.value<ServerPlayer *>();
		if (futo && futo->isAlive() && futo->hasSkill(this) && futo->getPhase() == Player::Play && futo->isWounded()) {
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, futo, futo, NULL, false);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@jinpan", "@jinpan");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

class Michun : public TriggerSkill
{
	
public:
	Michun() : TriggerSkill("michun")
	{
		events << FinishJudge;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		JudgeStruct *judge = data.value<JudgeStruct *>();
		ServerPlayer *lily = room->findPlayerBySkillName(objectName());
		if (lily && lily->isAlive() && judge->card->isRed() && judge->who && judge->who->isAlive())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, lily, lily, NULL, false, judge->who);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->preferredTarget;
		JudgeStruct *judge = data.value<JudgeStruct *>();
		player->obtainCard(judge->card, true);
	}
};

HuawuCard::HuawuCard()
{
	
}

bool HuawuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < 2 && to_select != Self;
}

bool HuawuCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
    return (targets.length() == 1 && targets.at(0)->isWounded()) || targets.length() == 2;
}

void HuawuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	if (targets.length() == 1)
		targets.at(0)->drawCards(3);
	else if (targets.length() == 2) {
		source->drawCards(1);
		targets.at(0)->drawCards(1);
		targets.at(1)->drawCards(1);
	}
}

class Huawu : public ViewAsSkill
{
	
public:
	Huawu() : ViewAsSkill("huawu")
	{
	}
	
	virtual bool isEnabledAtPlay(const Player *target) const
	{
		return !target->hasUsed("HuawuCard");
	}
	
	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return selected.length() < 2 && !to_select->isEquipped();
	}
	
	virtual const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.length() != 2)
			return NULL;
		HuawuCard *card = new HuawuCard;
		card->addSubcards(cards);
		return card;
	}
};

FeixiangCard::FeixiangCard()
{
	
}

bool FeixiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->getHandcardNum() < to_select->getHp();
}

void FeixiangCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	effect.to->drawCards(2);
	if (effect.to->getMark("@hisou") == 0)
		room->addPlayerMark(effect.to, "@hisou", 1);
	else
		room->askForUseCard(effect.to, "slash", "@feixiang-slash", -1, Card::MethodUse, false);
}

class FeixiangVS : public ZeroCardViewAsSkill
{
	
public:
	FeixiangVS() : ZeroCardViewAsSkill("feixiang")
	{
		response_pattern = "@@feixiang";
	}
	
	virtual const Card *viewAs() const
	{
		return new FeixiangCard;
	}
};

class Feixiang : public MasochismSkill
{
	
public:
	Feixiang() : MasochismSkill("feixiang")
	{
		view_as_skill = new FeixiangVS;
	}
	
	QList<SkillInvokeDetail> triggerable(const Room *room, const DamageStruct &damage) const
	{
		ServerPlayer *tenshi = damage.to;
		if (tenshi && tenshi->isAlive() && tenshi->hasSkill(this))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tenshi, tenshi, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@feixiang", "@feixiang");
	}
	
	void onDamaged(Room *, QSharedPointer<SkillInvokeDetail>, const DamageStruct &) const
	{
	}
};

GuiqiaoCard::GuiqiaoCard()
{
	will_throw = false;
}

bool GuiqiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select != Self;
}

void GuiqiaoCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	effect.to->obtainCard(this, true);
	effect.from->tag["GuiqiaoObtainer"] = QVariant::fromValue(effect.to);
	ServerPlayer *player = room->askForPlayerChosen(effect.from, room->getOtherPlayers(effect.to), "guiqiao");
	room->damage(DamageStruct("guiqiao", effect.to, player));
	effect.from->turnOver();
}

class Guiqiao : public OneCardViewAsSkill
{
	
public:
	Guiqiao() : OneCardViewAsSkill("guiqiao")
	{
		filter_pattern = ".|spade";
	}
	
	virtual bool isEnabledAtPlay(const Player *target) const
	{
		return !target->hasUsed("GuiqiaoCard");
	}
	
	virtual const Card *viewAs(const Card *originalCard) const
	{
		GuiqiaoCard *card = new GuiqiaoCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Chuangshi : public TriggerSkill
{
	
public:
	Chuangshi() : TriggerSkill("chuangshi")
	{
		events << TurnedOver;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *player = data.value<ServerPlayer *>();
		ServerPlayer *ex_keine = room->findPlayerBySkillName(objectName());
		if (player && player->isAlive() && ex_keine && ex_keine->isAlive())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_keine, ex_keine, NULL, false, player);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->preferredTarget;
		ServerPlayer *ex_keine = invoke->invoker;
		ex_keine->tag["ChuangshiTarget"] = data;
		QString choice = room->askForChoice(ex_keine, objectName(), "basic+trick+equip");
		room->touhouLogmessage("#ChuangshiLog", ex_keine, choice, QList<ServerPlayer *>(), QString());
		QList<int> card_ids = room->getNCards(4, false);
		QList<int> disabled, enabled;
		foreach(int id, card_ids) {
			const Card *card = Sanguosha->getCard(id);
			if (card->getType() != choice)
				disabled << id;
			else
				enabled << id;
		}
		room->fillAG(card_ids, ex_keine, disabled);
		int id = room->askForAG(ex_keine, enabled, true, objectName());
		room->clearAG(ex_keine);
		if (id > 0) {
			const Card *card = Sanguosha->getCard(id);
			player->obtainCard(card, true);
			card_ids.removeOne(id);
		}
		room->askForUseCard(player, "Slash,EquipCard|.|.|hand", "@chuangshi-use", -1, Card::MethodUse, false);
		room->askForGuanxing(ex_keine, card_ids, Room::GuanxingUpOnly, objectName());
		return false;
	}
};

class Baize : public TriggerSkill
{
	
public:
	Baize() : TriggerSkill("baize")
	{
		events << Damaged;
		frequency = Wake;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *ex_keine = damage.to;
		if (ex_keine && ex_keine->isAlive() && ex_keine->hasSkill(this) && ex_keine->getMark(objectName()) == 0
				&& !ex_keine->faceUp() && damage.from && damage.from->isAlive()
				&& ex_keine->getHandcardNum() <= damage.from->getHandcardNum())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_keine, ex_keine, NULL, true);
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->doSuperLightbox("ex_keine:baize", "baize");
		ServerPlayer *ex_keine = invoke->invoker;
		room->addPlayerMark(ex_keine, objectName(), 1);
		if (room->changeMaxHpForAwakenSkill(ex_keine))
			room->acquireSkill(ex_keine, "chuangshi");
		ex_keine->turnOver();
		return false;
	}
};

class Shenxing : public MasochismSkill
{
	
public:
	Shenxing() : MasochismSkill("shenxing")
	{
		frequency = Compulsory;
	}
	
	QList<SkillInvokeDetail> triggerable(const Room *room, const DamageStruct &damage) const
	{
		ServerPlayer *ex_kanako = damage.to;
		if (ex_kanako && ex_kanako->isAlive() && ex_kanako->hasSkill(this) && damage.from && damage.from->isAlive()
				&& (!damage.card || !damage.card->isKindOf("Slash")))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_kanako, ex_kanako, NULL, true, damage.from);
		return QList<SkillInvokeDetail>();
	}
	
	void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
	{
		room->sendCompulsoryTriggerLog(invoke->invoker, objectName());
		invoke->preferredTarget->turnOver();
	}
};

ShenquanCard::ShenquanCard()
{
	
}

bool ShenquanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->isWounded();
}

void ShenquanCard::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.from->getRoom();
	RecoverStruct recover;
	recover.who = effect.from;
	recover.recover = 1;
	recover.reason = "shenquan";
	room->recover(effect.to, recover);
	if (effect.to->isChained())
		room->setPlayerProperty(effect.to, "chained", false);
	if (!effect.to->faceUp())
		effect.to->turnOver();
}

class ShenquanVS : public ZeroCardViewAsSkill
{
	
public:
	ShenquanVS() : ZeroCardViewAsSkill("shenquan")
	{
		response_pattern = "@@shenquan";
	}
	
	virtual const Card *viewAs() const
	{
		return new ShenquanCard;
	}
};

class Shenquan : public TriggerSkill
{
	
public:
	Shenquan() : TriggerSkill("shenquan")
	{
		events << Death;
		view_as_skill = new ShenquanVS;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DeathStruct death = data.value<DeathStruct>();
		ServerPlayer *ex_kanako = room->findPlayerBySkillName(objectName());
		if (ex_kanako && (ex_kanako->isAlive() || ex_kanako == death.who))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_kanako, ex_kanako, NULL, false);
		return QList<SkillInvokeDetail>();
	}
	
	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@shenquan", "@shenquan");
	}
	
	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

ShenfengCard::ShenfengCard()
{
	target_fixed = true;
}

void ShenfengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	room->doSuperLightbox("ex_kanako:shenfeng", "shenfeng");
	room->removePlayerMark(source, "@kamikaze", 1);
	int x = 1;
	foreach(ServerPlayer *p, room->getOtherPlayers(source)) {
		QStringList choices;
		choices << "SFDamage";
		if (p->getEquips().length() >= x)
			choices << "SFThrow";
		if (!p->isKongcheng() && source->isAlive())
			choices << "SFGive";
		p->tag["ShenfengX"] = QVariant::fromValue(x);
		QString choice = room->askForChoice(p, "shenfeng", choices.join("+"));
		if (choice == "SFDamage") {
			if (source->isAlive())
				room->damage(DamageStruct("shenfeng", source, p, x));
			else
				room->damage(DamageStruct("shenfeng", NULL, p, x));
		}
		else if (choice == "SFThrow") {
			p->throwAllEquips();
			x++;
		}
		else if (choice == "SFGive") {
			DummyCard *dummy = new DummyCard;
			dummy->addSubcards(p->getHandcards());
			source->obtainCard(dummy, false);
		}
	}
}

class Shenfeng : public ZeroCardViewAsSkill
{
	
public:
	Shenfeng() : ZeroCardViewAsSkill("shenfeng")
	{
		frequency = Limited;
		limit_mark = "@kamikaze";
	}
	
	virtual bool isEnabledAtPlay(const Player *target) const
	{
		return target->getMark("@kamikaze") > 0;
	}
	
	virtual const Card *viewAs() const
	{
		return new ShenfengCard;
	}
};

THShadowPackage::THShadowPackage()
	: Package("thshadow")
{
	General *daiyousei = new General(this, "daiyousei", "hakurei", 3, false);
	daiyousei->addSkill(new Liuyue);
	daiyousei->addSkill(new Fangmu);
	//daiyousei->addSkill(new THBanling);
	
	General *youmu = new General(this, "youmu", "hakurei", 3, false);
	youmu->addSkill(new Yaodao);
	youmu->addSkill(new YaodaoAdd);
	related_skills.insertMulti("yaodao", "#yaodao-add");
    youmu->addSkill(new THBanling);
	
	General *yuyuko = new General(this, "yuyuko", "hakurei", 4, false);
	yuyuko->addSkill(new Huadie);
	
	General *hina = new General(this, "hina", "hakurei", 3, false);
	hina->addSkill(new Lie);
	hina->addSkill(new Anyun);
	
	General *suimitsu = new General(this, "suimitsu", "hakurei", 4, false);
	suimitsu->addSkill(new Julang);
	suimitsu->addSkill(new Bamao);
	
	General *prismriver = new General(this, "prismriver", "moriya", 3, false);
	prismriver->addSkill(new Mianxian);
	prismriver->addSkill(new Dianyin);
	
	General *wriggle = new General(this, "wriggle", "moriya", 3, false);
	wriggle->addSkill(new Yingchong);
	wriggle->addSkill(new YingchongMax);
	related_skills.insertMulti("yingchong", "#yingchong-max");
	wriggle->addSkill(new Anchao);
	
	General *futo = new General(this, "futo", "moriya", 4, false);
	futo->addSkill(new Zonghuo);
	futo->addSkill(new Jinpan);
	
	General *lily = new General(this, "lily", "moriya", 3, false);
	lily->addSkill(new Michun);
	lily->addSkill(new Huawu);
	
	General *tenshi = new General(this, "tenshi", "moriya", 4, false);
	tenshi->addSkill(new Feixiang);
	
	General *ex_keine = new General(this, "ex_keine", "god", 4, false);
	ex_keine->addSkill(new Guiqiao);
	ex_keine->addSkill(new Baize);
	
	General *ex_kanako = new General(this, "ex_kanako", "god", 3, false);
	ex_kanako->addSkill(new Shenxing);
	ex_kanako->addSkill(new Shenquan);
	ex_kanako->addSkill(new Shenfeng);
	
	addMetaObject<LiuyueCard>();
	addMetaObject<LieCard>();
	addMetaObject<JulangCard>();
	addMetaObject<BamaoCard>();
	addMetaObject<YingchongCard>();
	addMetaObject<AnchaoCard>();
	addMetaObject<JinpanCard>();
	addMetaObject<HuawuCard>();
	addMetaObject<FeixiangCard>();
	addMetaObject<GuiqiaoCard>();
	addMetaObject<ShenquanCard>();
	addMetaObject<ShenfengCard>();
	
	skills << new Chuangshi;
}

ADD_PACKAGE(THShadow)

