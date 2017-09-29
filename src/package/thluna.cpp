#include "thluna.h"
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
#include "settings.h"

#include <QCommandLinkButton>

class Yueyin : public TriggerSkill
{

public:
	Yueyin() : TriggerSkill("yueyin")
	{
		events << CardUsed << EventPhaseChanging;
		frequency = Compulsory;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardUsed) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *benben = use.from;
			if (benben && benben->isAlive() && benben->hasSkill(this) && !use.card->isKindOf("SkillCard") && benben->getPhase() == Player::Play)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, benben, benben, NULL, true);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *benben = change.player;
			if (benben && benben->isAlive() && benben->hasSkill(this) && change.from == Player::Play)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, benben, benben, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *benben = invoke->invoker;
		if (event == CardUsed) {
			room->sendCompulsoryTriggerLog(benben, objectName());
			room->clearPlayerCardLimitation(benben, true);
			CardUseStruct use = data.value<CardUseStruct>();
			room->setPlayerMark(benben, "yueyin", use.card->getNumber());
			if (use.card->getNumber() > 0) {
				QString pattern = ".|.|";
				for (int i = 1; i <= 13; i++) {
					if (qAbs(i - benben->getMark("yueyin")) <= 2)
						pattern += QString::number(i) + ",";
				}
				pattern.chop(1);
				pattern += "|hand";
				room->setPlayerCardLimitation(benben, "use", pattern, true);
			}
		} else if (event == EventPhaseChanging) {
			room->clearPlayerCardLimitation(benben, true);
		}
		return false;
	}
};

LieboCard::LieboCard()
{

}

bool LieboCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	if (targets.isEmpty())
		return to_select != Self || to_select->hasFlag("LieboUseTo");
	const Player *player = targets.at(0);
	if (player->hasFlag("LieboUseTo"))
		return to_select->hasFlag("LieboUseTo");
	return false;
}

void LieboCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets)
		p->drawCards(1);
}

class LieboVS : public ZeroCardViewAsSkill
{

public:
	LieboVS() : ZeroCardViewAsSkill("liebo")
	{
		response_pattern = "@@liebo";
	}

	const Card *viewAs() const
	{
		return new LieboCard;
	}
};

class Liebo : public TriggerSkill
{

public:
	Liebo() : TriggerSkill("liebo")
	{
		events << TargetConfirmed;
		view_as_skill = new LieboVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *benben = use.from;
		if (benben && benben->isAlive() && benben->hasSkill(this) && (use.card->isBlack() || use.card->getNumber() >= 10))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, benben, benben, NULL, false);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *benben = invoke->invoker;
		CardUseStruct use = data.value<CardUseStruct>();
		foreach (ServerPlayer *p, use.to)
			room->setPlayerFlag(p, "LieboUseTo");
		bool yes = room->askForUseCard(invoke->invoker, "@@liebo", "@liebo");
		foreach (ServerPlayer *p, room->getAlivePlayers())
			room->setPlayerFlag(p, "-LieboUseTo");
		return yes;
	}

	bool effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &) const
	{
		return false;
	}
};

DuanxiangCard::DuanxiangCard()
{
	will_throw = false;
	target_fixed = true;
}

void DuanxiangCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	room->setPlayerFlag(source, "DuanxiangInvoked");
	ServerPlayer *yatsuhashi = room->findPlayerBySkillName("duanxiang");
	const Card *card = Sanguosha->getCard(getSubcards().at(0));
	room->showCard(source, card->getEffectiveId(), yatsuhashi);
	Slash *slash = new Slash(Card::SuitToBeDecided, 0);
	if (card->isKindOf("Slash"))
		slash->addSubcard(card);
	if (!card->isAvailable(source) || (card->isKindOf("Slash") && !slash->IsAvailable(source, card, false))) {
		yatsuhashi->tag["DuanxiangCard"] = QVariant::fromValue(Sanguosha->getCard(getSubcards().at(0)));
		const Card *c = room->askForCard(yatsuhashi, ".|.|.|hand", "@duanxiang-exchange", QVariant(), Card::MethodNone);
		if (c) {
			room->showCard(source, card->getEffectiveId());
			source->obtainCard(c, false);
			yatsuhashi->obtainCard(card, true);
		}
	}
}

class DuanxiangVS : public OneCardViewAsSkill
{

public:
	DuanxiangVS() : OneCardViewAsSkill("duanxiang")
	{
		response_pattern = "@@duanxiang";
		filter_pattern = ".|.|.|hand";
	}

	const Card *viewAs(const Card *originalCard) const
	{
		DuanxiangCard *card = new DuanxiangCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Duanxiang : public TriggerSkill
{

public:
	Duanxiang() : TriggerSkill("duanxiang")
	{
		events << CardFinished << EventPhaseChanging;
		view_as_skill = new DuanxiangVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardFinished) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *player = use.from;
			ServerPlayer *yatsuhashi = room->findPlayerBySkillName(objectName());
			if (player && player->isAlive() && yatsuhashi && yatsuhashi->isAlive() && (use.card->isKindOf("BasicCard") || use.card->isNDTrick()) && player->getPhase() == Player::Play
					&& !player->hasFlag("DuanxiangInvoked") && player != yatsuhashi)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yatsuhashi, player, NULL, false);
		} else if (event == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *player = change.player;
			ServerPlayer *yatsuhashi = room->findPlayerBySkillName(objectName());
			if (player && player->isAlive() && player->hasFlag("DuanxiangInvoked") && change.from == Player::Play && yatsuhashi && yatsuhashi->isAlive())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yatsuhashi, player, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->invoker;
		ServerPlayer *yatsuhashi = invoke->owner;
		if (event == CardFinished) {
			return room->askForUseCard(player, "@@duanxiang", "@duanxiang:" + yatsuhashi->objectName());
		}
		return true;
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *player = invoke->invoker;
		if (event == EventPhaseChanging)
			room->setPlayerFlag(player, "-DuanxiangInvoked");
		return false;
	}
};

class Jinxian : public TriggerSkill
{

public:
	Jinxian() : TriggerSkill("jinxian")
	{
		events << CardsMoveOneTime;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *yatsuhashi = qobject_cast<ServerPlayer *>(move.to);
		if (yatsuhashi && yatsuhashi->isAlive() && yatsuhashi->hasSkill(this)) {
			foreach (int id, move.card_ids) {
				const Card *c = Sanguosha->getCard(id);
				if (c->hasFlag("visible"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yatsuhashi, yatsuhashi, NULL, false);
			}
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yatsuhashi = invoke->invoker;
		yatsuhashi->tag["JinxianData"] = data;
		ServerPlayer *target = room->askForPlayerChosen(yatsuhashi, room->getAlivePlayers(), objectName(), "@jinxian-draw", true, true);
		if (target) {
			yatsuhashi->tag["JinxianTarget"] = QVariant::fromValue(target);
			return true;
		}
		return false;
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *yatsuhashi = invoke->invoker;
		DummyCard *dummy = new DummyCard;
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		dummy->addSubcards(move.card_ids);
		CardMoveReason reason(CardMoveReason::S_REASON_THROW, yatsuhashi->objectName(), QString(), objectName(), QString());
		room->throwCard(dummy, reason, yatsuhashi);
		ServerPlayer *target = yatsuhashi->tag["JinxianTarget"].value<ServerPlayer *>();
		if (target && target->isAlive()) {
			target->drawCards(2);
			foreach (ServerPlayer *p, room->getOtherPlayers(target)) {
				if (p->getHandcardNum() >= target->getHandcardNum())
					return false;
			}
			room->loseHp(yatsuhashi, 1);
		}
		return false;
	}
};

class Jieyu : public TriggerSkill
{

public:
	Jieyu() : TriggerSkill("jieyu")
	{
		events << CardUsed << Damaged;
		frequency = Frequent;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == CardUsed) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *kosuzu = room->findPlayerBySkillName(objectName());
			if (kosuzu && kosuzu->isAlive() && use.from && !use.card->isKindOf("SkillCard")) {
				foreach (int id, use.card->getSubcards()) {
					const Card *c = Sanguosha->getCard(id);
					if (c->objectName() != use.card->objectName())
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kosuzu, kosuzu, NULL, true);
				}
			}
		} else if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *kosuzu = damage.to;
			if (kosuzu && kosuzu->isAlive() && kosuzu->hasSkill(this))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kosuzu, kosuzu, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *kosuzu = invoke->invoker;
		JsonArray msg;
        msg << objectName() << kosuzu->objectName();
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_INVOKE_SKILL, msg);
        room->notifySkillInvoked(kosuzu, objectName());
		kosuzu->drawCards(1);
		return false;
	}
};

YaoshuDialog *YaoshuDialog::getInstance(const QString &object, bool left, bool right)
{
    static YaoshuDialog *instance;
    if (instance == NULL || instance->objectName() != object) {
        instance = new YaoshuDialog(object, left, right);
    }
    return instance;
}

YaoshuDialog::YaoshuDialog(const QString &object, bool left, bool right) : object_name(object)
{
    setObjectName(object);
    setWindowTitle(Sanguosha->translate(object)); //need translate title?
    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    if (left) layout->addWidget(createLeft());
    if (right) layout->addWidget(createRight());
    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(selectCard(QAbstractButton *)));
}

void YaoshuDialog::popup()
{
    Card::HandlingMethod method;
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
        method = Card::MethodResponse;
    else
        method = Card::MethodUse;

    QStringList checkedPatterns;
    QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
    bool play = (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY);

    //collect avaliable patterns for specific skill
    QStringList validPatterns;
    if (object_name == "huaxiang") {
        validPatterns << "slash" << "analeptic";
        if (Self->getMaxHp() <= 3)
            validPatterns << "jink";
        if (Self->getMaxHp() <= 2)
            validPatterns << "peach";
    } else if (object_name == "xihua") {
        QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
        foreach(const Card *card, cards) {
            if ((card->isNDTrick() || card->isKindOf("BasicCard"))
                    && !ServerInfo.Extensions.contains("!" + card->getPackage())) {
                QString name = card->objectName();
                if (card->isKindOf("Slash"))
                    name = "slash";
                QString markName = "xihua_record_" + name;
                if (!validPatterns.contains(name) && Self->getMark(markName) == 0)
                    validPatterns << card->objectName();
            }
        }
    }
    //if (object_name == "yaoshu") {
    else {
        QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
        foreach(const Card *card, cards) {
            if ((card->isNDTrick() || card->isKindOf("BasicCard"))
                    && !ServerInfo.Extensions.contains("!" + card->getPackage())) {
                QString name = card->objectName();
                if (card->isKindOf("Slash"))
                    name = "slash";
                if (!validPatterns.contains(name))
                    validPatterns << card->objectName();
            }
        }
    }
    //then match it and check "CardLimit"
    foreach(QString str, validPatterns) {
        const Skill *skill = Sanguosha->getSkill(object_name);
        if (play || skill->matchAvaliablePattern(str, pattern)) {
            Card *card = Sanguosha->cloneCard(str);
            DELETE_OVER_SCOPE(Card, card)
                    if (!Self->isCardLimited(card, method))
                    checkedPatterns << str;
        }
    }
    //while responsing, if only one pattern were checked, emit click()
    if (object_name != "chuangshi" && !play && checkedPatterns.length() <= 1 && !checkedPatterns.contains("slash")) {
        emit onButtonClick();
        return;
    }

    foreach (QAbstractButton *button, group->buttons()) {
        const Card *card = map[button->objectName()];
        const Player *user = NULL;
        if (object_name == "chuangshi") { //check the card is Available for chuangshi target.
            foreach(const Player *p, Self->getAliveSiblings())
            {
                if (p->getMark("chuangshi_user") > 0) {
                    user = p;
                    break;
                }
            }
        } else
            user = Self;
        if (user == NULL)
            user = Self;

        bool avaliable = card->isAvailable(user);
        if (object_name == "yaoshu" && user->getMark("xiubu"))
            avaliable = true;

        bool checked = (checkedPatterns.contains(card->objectName()) || (card->isKindOf("Slash") && checkedPatterns.contains("slash")));
        bool enabled = !user->isCardLimited(card, method, true) && avaliable && (checked || object_name == "chuangshi");
        button->setEnabled(enabled);
    }

    Self->tag.remove(object_name);
    exec();
}

void YaoshuDialog::selectCard(QAbstractButton *button)
{
    const Card *card = map.value(button->objectName());
    Self->tag[object_name] = QVariant::fromValue(card);

    emit onButtonClick();
    accept();
}

QGroupBox *YaoshuDialog::createLeft()
{
    QGroupBox *box = new QGroupBox;
    box->setTitle(Sanguosha->translate("basic"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    QStringList ban_list; //no need to ban
    if (object_name == "chuangshi")
        ban_list << "Analeptic";


    foreach (const Card *card, cards) {
        if (card->getTypeId() == Card::TypeBasic && !map.contains(card->objectName())
                && !ban_list.contains(card->getClassName()) && !ServerInfo.Extensions.contains("!" + card->getPackage())) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setParent(this);
            layout->addWidget(createButton(c));
        }
    }

    layout->addStretch();
    box->setLayout(layout);
    return box;
}

QGroupBox *YaoshuDialog::createRight()
{
    QGroupBox *box = new QGroupBox(Sanguosha->translate("ndtrick"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(Sanguosha->translate("single_target_trick"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(Sanguosha->translate("multiple_target_trick"));
    QVBoxLayout *layout2 = new QVBoxLayout;


    QStringList ban_list; //no need to ban
    if (object_name == "chuangshi")
        ban_list << "GodSalvation" << "ArcheryAttack" << "SavageAssault";
    //    ban_list << "Drowning" << "BurningCamps" << "LureTiger";



    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach (const Card *card, cards) {
        if (card->isNDTrick() && !map.contains(card->objectName()) && !ban_list.contains(card->getClassName())
                && !ServerInfo.Extensions.contains("!" + card->getPackage())) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setSkillName(object_name);
            c->setParent(this);

            QVBoxLayout *layout = c->isKindOf("SingleTargetTrick") ? layout1 : layout2;
            layout->addWidget(createButton(c));
        }
    }

    box->setLayout(layout);
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    layout1->addStretch();
    layout2->addStretch();

    layout->addWidget(box1);
    layout->addWidget(box2);
    return box;
}

QAbstractButton *YaoshuDialog::createButton(const Card *card)
{
    if (card->objectName() == "slash" && map.contains(card->objectName()) && !map.contains("normal_slash")) {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate("normal_slash"));
        button->setObjectName("normal_slash");
        button->setToolTip(card->getDescription());

        map.insert("normal_slash", card);
        group->addButton(button);

        return button;
    } else {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
        button->setObjectName(card->objectName());
        button->setToolTip(card->getDescription());

        map.insert(card->objectName(), card);
        group->addButton(button);

        return button;
    }
}

YaoshuCard::YaoshuCard()
{
    will_throw = false;
}

bool YaoshuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    const Card *card = Self->tag.value("yaoshu").value<const Card *>();
    const Card *oc = Sanguosha->getCard(subcards.first());
    Card *new_card = Sanguosha->cloneCard(card->objectName(), oc->getSuit(), oc->getNumber());
    DELETE_OVER_SCOPE(Card, new_card)
            new_card->addSubcard(oc);
    new_card->setSkillName("yaoshu");
    return new_card && new_card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, new_card, targets);
}

bool YaoshuCard::targetFixed() const
{
    const Card *card = Self->tag.value("yaoshu").value<const Card *>();
    const Card *oc = Sanguosha->getCard(subcards.first());
    Card *new_card = Sanguosha->cloneCard(card->objectName(), oc->getSuit(), oc->getNumber());
    DELETE_OVER_SCOPE(Card, new_card)
            new_card->addSubcard(oc);
    new_card->setSkillName("yaoshu");
    return new_card && new_card->targetFixed();
}

bool YaoshuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    const Card *card = Self->tag.value("yaoshu").value<const Card *>();
    const Card *oc = Sanguosha->getCard(subcards.first());
    Card *new_card = Sanguosha->cloneCard(card->objectName(), oc->getSuit(), oc->getNumber());
    DELETE_OVER_SCOPE(Card, new_card)
            new_card->addSubcard(oc);
    new_card->setSkillName("yaoshu");
    if (card->isKindOf("IronChain") && targets.length() == 0)
        return false;
    return new_card && new_card->targetsFeasible(targets, Self);
}

const Card *YaoshuCard::validate(CardUseStruct &use) const
{
    QString to_use = user_string;

    const Card *card = Sanguosha->getCard(subcards.first());
    Card *use_card = Sanguosha->cloneCard(to_use, card->getSuit(), card->getNumber());
    use_card->setSkillName("yaoshu");
    use_card->addSubcard(subcards.first());
    use_card->deleteLater();
    use.from->getRoom()->setPlayerMark(use.from, "yaoshu", 1);
    use_card->setFlags("YaoshuFlag");
    return use_card;
}

class YaoshuVS : public OneCardViewAsSkill
{
public:
    YaoshuVS() : OneCardViewAsSkill("yaoshu")
    {
        filter_pattern = ".|.|.|hand";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("YaoshuCard") && !player->isKongcheng() && player->getHandcardNum() < player->getMaxHp();
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        const Card *c = Self->tag.value("yaoshu").value<const Card *>();
        if (c) {
            YaoshuCard *card = new YaoshuCard;
            card->setUserString(c->objectName());
            card->addSubcard(originalCard);
            return card;
        } else
            return NULL;
    }
};

class Yaoshu : public TriggerSkill
{

public:
	Yaoshu() : TriggerSkill("yaoshu")
	{
		events << TargetConfirmed << CardFinished;
		view_as_skill = new YaoshuVS;
	}

	QDialog *getDialog() const
    {
    	return YaoshuDialog::getInstance("yaoshu");
    }

	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == TargetConfirmed) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *kosuzu = use.from;
			if (kosuzu && kosuzu->isAlive() && kosuzu->hasSkill(this) && use.card->hasFlag("YaoshuFlag") && use.to.length() == 1)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kosuzu, kosuzu, NULL, true);
		} else if (event == CardFinished) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *kosuzu = use.from;
			if (kosuzu && kosuzu->isAlive() && kosuzu->hasSkill(this))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kosuzu, kosuzu, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *kosuzu = invoke->invoker;
		if (event == TargetConfirmed) {
			CardUseStruct use = data.value<CardUseStruct>();
			use.card->setFlags("YaoshuSingleTarget");
		} else if (event == CardFinished) {
			CardUseStruct use = data.value<CardUseStruct>();
			if (use.card->hasFlag("YaoshuSingleTarget")) {
				room->damage(DamageStruct(objectName(), NULL, kosuzu));
			}
			room->clearCardFlag(use.card);
		}
		return false;
	}
};

class Xiaoyan : public TriggerSkill
{

public:
	Xiaoyan() : TriggerSkill("xiaoyan")
	{
		events << CardsMoveOneTime;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *from = qobject_cast<ServerPlayer *>(move.from);
		ServerPlayer *clownpiece = room->findPlayerBySkillName(objectName());
		if (clownpiece && clownpiece->isAlive() && from && from->isAlive() && from->isKongcheng() && move.from_places.contains(Player::PlaceHand)
				&& from->getPhase() == Player::NotActive) {
			foreach (ServerPlayer *p, room->getAllPlayers(false)) {
				if (p->getHp() <= 0)
					return QList<SkillInvokeDetail>();
			}
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, clownpiece, clownpiece, NULL, false, from);
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		invoke->invoker->tag["XiaoyanPlayer"] = QVariant::fromValue(invoke->preferredTarget);
		return room->askForSkillInvoke(invoke->invoker, objectName(), data);
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *clownpiece = invoke->invoker;
		ServerPlayer *from = invoke->preferredTarget;
		from->drawCards(qMin(from->getLostHp(), 5));
		room->damage(DamageStruct(objectName(), clownpiece, from, 1, DamageStruct::Fire));
		if (room->askForSkillInvoke(from, "xiaoyan_draw", QVariant("self:" + clownpiece->objectName())))
			clownpiece->drawCards(1);
		return false;
	}
};

WanchuCard::WanchuCard()
{
	handling_method = Card::MethodRecast;
	will_throw = false;
	can_recast = true;
}

bool WanchuCard::targetFixed() const
{
	return getSubcards().length() == 1;
}

bool WanchuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && to_select->getWeapon() && to_select->getHp() >= Self->getHp();
}

void WanchuCard::onUse(Room *room, const CardUseStruct &use) const
{
	if (getSubcards().length() == 0) {
		CardUseStruct card_use = use;
	    ServerPlayer *player = card_use.from;

	    room->sortByActionOrder(card_use.to);

	    QList<ServerPlayer *> targets = card_use.to;
	    card_use.to = targets;

	    LogMessage log;
	    log.from = player;
	    if (!card_use.card->targetFixed() || card_use.to.length() > 1 || !card_use.to.contains(card_use.from))
	        log.to = card_use.to;
	    log.type = "#UseCard";
	    log.card_str = card_use.card->toString(true);
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
	    thread->trigger(CardUsed, room, data);
	    thread->trigger(CardFinished, room, data);
	} else if (getSubcards().length() == 1) {
		CardUseStruct card_use = use;
		ServerPlayer *ringo = card_use.from;

	    CardMoveReason reason(CardMoveReason::S_REASON_RECAST, ringo->objectName());
	    reason.m_skillName = "wanchu";
	    room->moveCardTo(this, ringo, NULL, Player::DiscardPile, reason);
	    ringo->broadcastSkillInvoke("@recast");

	    int id = card_use.card->getSubcards().first();

	    LogMessage log;
	    log.type = "#UseCard_Recast";
	    log.from = ringo;
	    log.card_str = QString::number(id);
	    room->sendLog(log);

	    ringo->drawCards(1, "recast");

	    room->addPlayerMark(ringo, "wanchu", 1);
		if (ringo->getMark("wanchu") >= 3) {
			QList<ServerPlayer *> wounds;
			foreach (ServerPlayer *p, room->getAlivePlayers()) {
				if (p->isWounded())
					wounds << p;
			}
			if (!wounds.isEmpty()) {
				ServerPlayer *target = room->askForPlayerChosen(ringo, wounds, "wanchu", "@wanchu-recover", true, false);
				if (target) {
					room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, ringo->objectName(), target->objectName());
					RecoverStruct recover;
					recover.who = ringo;
					recover.recover = 1;
					recover.reason = "wanchu";
					room->recover(target, recover);
				}
			}
		}
	}
}

void WanchuCard::onEffect(const CardEffectStruct &effect) const
{
	if (getSubcards().isEmpty()) {
		ServerPlayer *from = effect.from;
		ServerPlayer *to = effect.to;
		Room *room = from->getRoom();
		CardMoveReason reason(CardMoveReason::S_REASON_PUT, to->objectName(), QString(), "wanchu", QString());
		room->moveCardTo(to->getWeapon(), to, NULL, Player::DrawPile, reason);
		if (from->getMark("wanchu") >= 3) {
			QList<ServerPlayer *> targets;
			foreach (ServerPlayer *p, room->getAlivePlayers()) {
				if (p->isWounded())
					targets << p;
			}
			if (!targets.isEmpty()) {
				ServerPlayer *target = room->askForPlayerChosen(from, targets, "wanchu", "@wanchu-recover", true, false);
				if (target) {
					room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, from->objectName(), target->objectName());
					RecoverStruct recover;
					recover.who = from;
					recover.recover = 1;
					recover.reason = "wanchu";
					room->recover(target, recover);
				}
			}
		}
	}
}

class Wanchu : public ViewAsSkill
{

public:
	Wanchu() : ViewAsSkill("wanchu")
	{
	}

	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return selected.isEmpty() && to_select->isKindOf("EquipCard");
	}

	bool isEnabledAtPlay(const Player *) const
	{
		return true;
	}

	const Card *viewAs(const QList<const Card *> &cards) const
	{
		WanchuCard *card = new WanchuCard;
		card->addSubcards(cards);
		return card;
	}
};

class WanchuClear : public TriggerSkill
{

public:
	WanchuClear() : TriggerSkill("#wanchu-clear")
	{
		events << EventPhaseChanging;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		PhaseChangeStruct change = data.value<PhaseChangeStruct>();
		ServerPlayer *ringo = change.player;
		if (ringo && ringo->isAlive() && ringo->hasSkill("wanchu") && change.from == Player::Finish)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ringo, ringo, NULL, true);
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->setPlayerMark(invoke->invoker, "wanchu", 0);
		return false;
	}
};

YingdanCard::YingdanCard()
{
	target_fixed = true;
}

void YingdanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
	QList<int> card_ids = room->getNCards(4, false);
	QList<int> slashes;
	QList<int> others;
	foreach (int id, card_ids) {
		const Card *c = Sanguosha->getCard(id);
		if (c->isKindOf("Slash")) {
			slashes << id;
		} else {
			others << id;
		}
	}
	if (source->getLostHp() > 0) {
		int n = source->getLostHp();
		int id = 0;
		while (id >= 0 && n > 0) {
			room->fillAG(card_ids, source, others);
			int id = room->askForAG(source, slashes, true, "yingdan");
			room->clearAG(source);
			if (id > 0) {
				Card *slash = Sanguosha->getCard(id);
				slash->setSkillName("yingdan");
				QList<ServerPlayer *> valid_targets;
				room->setPlayerFlag(source, "slashNoDistanceLimit");
				foreach (ServerPlayer *p, room->getOtherPlayers(source)) {
					if (slash->targetFilter(QList<const Player *>(), p, source) && source->canSlash(p, NULL, false) && !p->hasFlag("YingdanTargeted"))
						valid_targets << p;
				}
				room->setPlayerFlag(source, "-slashNoDistanceLimit");
				source->tag["YingdanSlash"] = QVariant::fromValue(slash);
				ServerPlayer *target = room->askForPlayerChosen(source, valid_targets, "yingdan", "@yingdan-slash-target", true, false);
				if (target) {
					n--;
					room->setPlayerFlag(target, "YingdanTargeted");
					CardUseStruct use;
					use.card = slash;
					use.from = source;
					use.to.append(target);
					room->useCard(use);
					card_ids.removeOne(id);
					slashes.removeOne(id);
					others.append(id);
				} else {
					break;
				}
			} else {
				break;
			}
		}
	}
	if (card_ids.length() > 0) {
		room->askForGuanxing(source, card_ids, Room::GuanxingBothSides);
	}
}

class Yingdan : public ZeroCardViewAsSkill
{

public:
	Yingdan() : ZeroCardViewAsSkill("yingdan")
	{
	}

	bool isEnabledAtPlay(const Player *seiran) const
	{
		return !seiran->hasUsed("YingdanCard");
	}

	const Card *viewAs() const
	{
		return new YingdanCard;
	}
};

class YingdanClear : public TriggerSkill
{

public:
	YingdanClear() : TriggerSkill("#yingdan-clear")
	{
		events << EventPhaseChanging;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		PhaseChangeStruct change = data.value<PhaseChangeStruct>();
		ServerPlayer *seiran = change.player;
		if (seiran && seiran->isAlive() && seiran->hasSkill(this) && change.from == Player::Play)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, seiran, seiran, NULL, true);
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		foreach (ServerPlayer *p, room->getAlivePlayers()) {
			if (p->hasFlag("YingdanTargeted"))
				room->setPlayerFlag(p, "-YingdanTargeted");
		}
		return false;
	}
};

class Yuechong : public TriggerSkill
{

public:
	Yuechong() : TriggerSkill("yuechong")
	{
		events << CardsMoveOneTime;
		frequency = Frequent;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *seiran = room->findPlayerBySkillName(objectName());
		if (seiran && seiran->isAlive() && (move.reason.m_reason ^ 3) % 16 == 0) {
			foreach (int id, move.card_ids) {
				const Card *c = Sanguosha->getCard(id);
				if (c->isKindOf("Weapon"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, seiran, seiran, NULL, true);
			}
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		ServerPlayer *seiran = invoke->invoker;
		int n = 0;
		foreach (int id, move.card_ids) {
			const Card *c = Sanguosha->getCard(id);
			if (c->isKindOf("Weapon"))
				n++;
		}
		seiran->drawCards(n);
		return false;
	}
};

NianliCard::NianliCard()
{
}

bool NianliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	if (targets.isEmpty()) {
		QList<const Card *> cards = to_select->getEquips();
		cards << to_select->getJudgingArea();
		return !cards.isEmpty();
	} else if (targets.length() == 1) {
		const Player *first = targets.at(0);
		QList<const Card *> cards = first->getEquips();
		cards << first->getJudgingArea();
		if (first == to_select)
			return false;
		foreach (const Card *c, cards) {
			if ((c->isKindOf("Weapon") && !to_select->getWeapon()) || (c->isKindOf("Armor") && !to_select->getArmor())
					|| (c->isKindOf("DefensiveHorse") && !to_select->getDefensiveHorse()) || (c->isKindOf("OffensiveHorse") && !to_select->getOffensiveHorse())
					|| (c->isKindOf("Treasure") && !to_select->getTreasure()))
				return true;
			else if (c->isKindOf("DelayedTrick")) {
				foreach (const Card *j, to_select->getJudgingArea()) {
					if (j->objectName() == c->objectName())
						return false;
				}
				return true;
			}
		}
	}
	return false;
}

bool NianliCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
	return targets.length() == 2;
}

void NianliCard::onUse(Room *room, const CardUseStruct &use) const
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

void NianliCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	ServerPlayer *from = targets.at(0);
	ServerPlayer *to = targets.at(1);
	QList<int> cards;
	QList<int> enabled;
	QList<int> disabled;
	foreach (const Card *c, from->getCards("ej")) {
		cards << c->getEffectiveId();
		if ((c->isKindOf("Weapon") && !to->getWeapon()) || (c->isKindOf("Armor") && !to->getArmor())
				|| (c->isKindOf("DefensiveHorse") && !to->getDefensiveHorse()) || (c->isKindOf("OffensiveHorse") && !to->getOffensiveHorse())
				|| (c->isKindOf("Treasure") && !to->getTreasure()))
			enabled << c->getEffectiveId();
		else if (c->isKindOf("DelayedTrick")) {
			foreach (const Card *j, to->getJudgingArea()) {
				if (j->objectName() != c->objectName())
					enabled << c->getEffectiveId();
			}
		} else
			disabled << c->getEffectiveId();
	}
	room->fillAG(cards, source, disabled);
	source->tag["NianliMoveFrom"] = QVariant::fromValue(from);
	int id = room->askForAG(source, enabled, false, "nianli");
	room->clearAG(source);
	const Card *to_move = Sanguosha->getCard(id);
	if (!source->hasFlag("RuhuanLess"))
		room->loseHp(source, 1);
	CardMoveReason reason(CardMoveReason::S_REASON_TRANSFER, source->objectName(), "nianli", QString());
	room->moveCardTo(to_move, from, to, room->getCardPlace(to_move->getEffectiveId()), reason, true);
	if (from == source || to == source)
		source->drawCards(1);
	room->addPlayerMark(source, "nianli", 1);
}

class Nianli : public ZeroCardViewAsSkill
{

public:
	Nianli() : ZeroCardViewAsSkill("nianli")
	{
	}

	bool isEnabledAtPlay(const Player *sumireko) const
	{
		if (sumireko->hasFlag("RuhuanLess"))
			return !sumireko->hasUsed("NianliCard");

		QList<const Player *> players = sumireko->getAliveSiblings();
		players << sumireko;
		if (sumireko->getMark("nianli") >= players.length() - 1)
			return false;
		foreach (const Player *p, players) {
			QList<const Card *> cards = p->getEquips();
			cards << p->getJudgingArea();
			if (!cards.isEmpty())
				return true;
		}
		return false;
	}

	const Card *viewAs() const
	{
		return new NianliCard;
	}
};

class NianliClear : public TriggerSkill
{

public:
	NianliClear() : TriggerSkill("#nianli-clear")
	{
		events << EventPhaseChanging;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		PhaseChangeStruct change = data.value<PhaseChangeStruct>();
		ServerPlayer *sumireko = change.player;
		if (sumireko && sumireko->isAlive() && sumireko->hasSkill(this) && change.from == Player::Play)
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sumireko, sumireko, NULL, true);
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->setPlayerMark(invoke->invoker, "nianli", 0);
		return false;
	}
};

class Ruhuan : public TriggerSkill
{

public:
	Ruhuan() : TriggerSkill("ruhuan")
	{
		events << DrawNCards << CardUsed << EventPhaseChanging;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
	{
		if (e == DrawNCards) {
			DrawNCardsStruct q = data.value<DrawNCardsStruct>();
			ServerPlayer *sumireko = q.player;
			if (sumireko && sumireko->isAlive() && sumireko->hasSkill(this))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sumireko, sumireko, NULL, false);
		} else if (e == CardUsed) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *sumireko = use.from;
			if (sumireko && sumireko->isAlive() && sumireko->hasFlag("RuhuanMore") && (use.card->isKindOf("Slash") || use.card->isNDTrick()) && !sumireko->isNude())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sumireko, sumireko, NULL, true);
		} else if (e == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *sumireko = change.player;
			if (sumireko && sumireko->isAlive() && sumireko->hasSkill(this)) {
				if (change.from == Player::Finish && sumireko->hasFlag("RuhuanMore"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sumireko, sumireko, NULL, true);
				if (change.to == Player::RoundStart && sumireko->hasFlag("RuhuanLess"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sumireko, sumireko, NULL, true);
			}
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (e == DrawNCards)
			return room->askForSkillInvoke(invoke->invoker, objectName(), data);
		return true;
	}

	bool effect(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *sumireko = invoke->invoker;
		if (e == DrawNCards) {
			QString choice = room->askForChoice(sumireko, objectName(), "RuhuanMore+RuhuanLess");
			room->setPlayerFlag(sumireko, choice);
			DrawNCardsStruct q = data.value<DrawNCardsStruct>();
			if (choice == "RuhuanMore")
				q.n += 2;
			else {
				q.n = 0;
				RecoverStruct recover;
				recover.who = NULL;
				recover.recover = 1;
				recover.reason = objectName();
				room->recover(sumireko, recover);
			}
			data = QVariant::fromValue(q);
		} else if (e == CardUsed) {
			room->sendCompulsoryTriggerLog(sumireko, objectName());
			room->askForDiscard(sumireko, objectName(), 1, 1, false, true, "@ruhuan-discard");
		} else if (e == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			if (change.from == Player::Finish) {
				room->setPlayerFlag(sumireko, "-RuhuanMore");
			} else if (change.to == Player::RoundStart) {
				room->setPlayerFlag(sumireko, "-RuhuanLess");
			}
		}
		return false;
	}
};

class RuhuanDist : public DistanceSkill
{

public:
    RuhuanDist() : DistanceSkill("#ruhuan-dist")
    {
    }

    virtual int getCorrect(const Player *from, const Player *to) const
    {
        if (from != to && to->hasFlag("RuhuanLess"))
        	return 1;
        return 0;
    }
};

class Mengwei : public TriggerSkill
{

public:
	Mengwei() : TriggerSkill("mengwei")
	{
		events << EventPhaseStart;
		frequency = Wake;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		ServerPlayer *sumireko = data.value<ServerPlayer *>();
		if (sumireko && sumireko->isAlive() && sumireko->hasSkill(this) && sumireko->getMark(objectName()) == 0 && sumireko->getPhase() == Player::RoundStart) {
			bool hp_lowest = true;
			bool equip_most = true;
			foreach (ServerPlayer *p, room->getOtherPlayers(sumireko)) {
				if (p->getHp() < sumireko->getHp()) {
					hp_lowest = false;
					break;
				}
			}
			if (sumireko->getEquips().isEmpty())
				equip_most = false;
			foreach (ServerPlayer *p, room->getOtherPlayers(sumireko)) {
				if (p->getEquips().length() > sumireko->getEquips().length()) {
					equip_most = false;
					break;
				}
			}
			if (hp_lowest || equip_most)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, sumireko, sumireko, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *sumireko = invoke->invoker;
		if (room->changeMaxHpForAwakenSkill(sumireko)) {
			room->doSuperLightbox("sumireko:mengwei", "mengwei");
			room->setPlayerMark(sumireko, objectName(), 1);
			sumireko->drawCards(2);
			room->acquireSkill(sumireko, "ruhuan");
		}
		return false;
	}
};

DongheCard::DongheCard()
{
}

bool DongheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.isEmpty() && Self->distanceTo(to_select) <= getSubcards().length() && to_select != Self;
}

void DongheCard::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *from = effect.from;
	ServerPlayer *to = effect.to;
	Room *room = from->getRoom();
	if (!room->askForDiscard(to, "donghe", getSubcards().length(), getSubcards().length(), true, false)) {
		room->damage(DamageStruct("donghe", from, to));
	}
}

class Donghe : public ViewAsSkill
{

public:
	Donghe() : ViewAsSkill("donghe")
	{
	}

	bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
	{
		return !to_select->isEquipped();
	}

	bool isEnabledAtPlay(const Player *kogasa) const
	{
		return !kogasa->hasUsed("DongheCard");
	}

	const Card *viewAs(const QList<const Card *> &cards) const
	{
		if (cards.isEmpty())
			return NULL;
		DongheCard *card = new DongheCard;
		card->addSubcards(cards);
		return card;
	}
};

YeyuCard::YeyuCard()
{
}

bool YeyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return targets.length() < Self->getMark("yeyu") && to_select->isAlive();
}

void YeyuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	room->setPlayerFlag(source, "YeyuUsed");
	foreach (ServerPlayer *p, targets) {
		QStringList choices;
		if (p->isWounded())
			choices << "YeyuRecover";
		choices << "YeyuDraw";
		source->tag["YeyuDecideOn"] = QVariant::fromValue(p);
		QString choice = room->askForChoice(source, "yeyu", choices.join("+"), QVariant::fromValue(p));
		if (choice == "YeyuRecover") {
			RecoverStruct recover;
			recover.who = source;
			recover.recover = 1;
			recover.reason = "yeyu";
			room->recover(p, recover);
		} else if (choice == "YeyuDraw")
			p->drawCards(1);
	}
}

class YeyuVS : public ZeroCardViewAsSkill
{

public:
	YeyuVS() : ZeroCardViewAsSkill("yeyu")
	{
		response_pattern = "@@yeyu";
	}

	const Card *viewAs() const
	{
		return new YeyuCard;
	}
};

class Yeyu : public TriggerSkill
{

public:
	Yeyu() : TriggerSkill("yeyu")
	{
		events << CardsMoveOneTime << EventPhaseChanging;
		view_as_skill = new YeyuVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
	{
		if (e == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *kogasa = room->findPlayerBySkillName(objectName());
			if (kogasa && kogasa->isAlive() && kogasa->hasFlag("YeyuUsed") && change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kogasa, kogasa, NULL, true);
			return QList<SkillInvokeDetail>();
		}
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		bool has_black = false;
		foreach (int id, move.card_ids) {
			const Card *c = Sanguosha->getCard(id);
			if (c->isBlack()) {
				has_black = true;
				break;
			}
		}
		if (!has_black)
			return QList<SkillInvokeDetail>();
		ServerPlayer *player = qobject_cast<ServerPlayer *>(move.from);
		ServerPlayer *kogasa = room->findPlayerBySkillName(objectName());
		if (kogasa && kogasa->isAlive() && player && player->isAlive() && kogasa != player && (move.reason.m_reason ^ 3) % 16 == 0 && move.card_ids.length() >= kogasa->getHp()
				&& !kogasa->hasFlag("YeyuUsed"))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kogasa, kogasa, NULL, false);
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (e == EventPhaseChanging)
			return true;
		CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
		room->setPlayerMark(invoke->invoker, "yeyu", move.card_ids.length());
		return room->askForUseCard(invoke->invoker, "@@yeyu", "@yeyu");
	}

	bool effect(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (e == EventPhaseChanging)
			room->setPlayerFlag(invoke->invoker, "-YeyuUsed");
		return false;
	}
};

MieliCard::MieliCard()
{

}

bool MieliCard::targetFilter(const QList<const Player *> &, const Player *to_select, const Player *Self) const
{
	return Self->distanceTo(to_select) <= 1;
}

void MieliCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets) {
		room->damage(DamageStruct("mieli", source, p, 1, DamageStruct::Soul));
	}
}

class MieliVS : public ZeroCardViewAsSkill
{

public:
	MieliVS() : ZeroCardViewAsSkill("mieli")
	{
		response_pattern = "@@mieli";
	}

	const Card *viewAs() const
	{
		return new MieliCard;
	}
};

class Mieli : public TriggerSkill
{

public:
	Mieli() : TriggerSkill("mieli")
	{
		events << CardFinished;
		view_as_skill = new MieliVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *ex_yuyuko = use.from;
		if (ex_yuyuko && ex_yuyuko->isAlive() && ex_yuyuko->hasSkill(this) && use.card->isNDTrick()) {
			foreach (ServerPlayer *p, room->getOtherPlayers(ex_yuyuko)) {
				if (ex_yuyuko->distanceTo(p) <= 1)
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_yuyuko, ex_yuyuko, NULL, true);
			}
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return room->askForUseCard(invoke->invoker, "@@mieli", "@mieli");
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		return false;
	}
};

class Zijin : public TriggerSkill
{

public:
	Zijin() : TriggerSkill("zijin")
	{
		events << Damage << EventPhaseChanging;
		frequency = Compulsory;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
	{
		if (e == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *ex_yuyuko = room->findPlayerBySkillName(objectName());
			if (ex_yuyuko && ex_yuyuko->isAlive() && change.from == Player::Finish) {
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_yuyuko, ex_yuyuko, NULL, true);
			}
		} else if (e == Damage) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *ex_yuyuko = damage.from;
			if (ex_yuyuko && ex_yuyuko->isAlive() && ex_yuyuko->hasSkill(this))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_yuyuko, ex_yuyuko, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_yuyuko = invoke->invoker;
		if (e == EventPhaseChanging) {
			bool yes = false;
			if (ex_yuyuko->getMark("zijin") >= 3)
				yes = true;
			room->setPlayerMark(ex_yuyuko, "zijin", 0);
			if (yes) {
				room->sendCompulsoryTriggerLog(ex_yuyuko, objectName());
				room->loseHp(ex_yuyuko, 1);
				ex_yuyuko->gainAnExtraTurn();
			}
		} else if (e == Damage) {
			room->addPlayerMark(ex_yuyuko, "zijin", 1);
		}
		return false;
	}
};

YouwangCard::YouwangCard()
{
    will_throw = false;
}

bool YouwangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    QString card = user_string;
    Card *new_card = Sanguosha->cloneCard(card, Card::NoSuit, 0);
    new_card->setSkillName("youwang");
    return new_card && new_card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, new_card, targets);
}

bool YouwangCard::targetFixed() const
{
    QString card = user_string;
    Card *new_card = Sanguosha->cloneCard(card, Card::NoSuit, 0);
    new_card->setSkillName("youwang");
    return new_card && new_card->targetFixed();
}

bool YouwangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    QString card = user_string;
    Card *new_card = Sanguosha->cloneCard(card, Card::NoSuit, 0);
    new_card->setSkillName("youwang");
    if (new_card->isKindOf("IronChain") && targets.length() == 0)
        return false;
    if (new_card->isKindOf("PhoenixFlame") && targets.length() == 0)
    	return false;
    return new_card && new_card->targetsFeasible(targets, Self);
}

const Card *YouwangCard::validate(CardUseStruct &use) const
{
	Room *room = use.from->getRoom();
	room->doSuperLightbox("ex_yuyuko:youwang", "youwang");
	room->setPlayerMark(use.from, "@lure", 0);

    QString to_use = user_string;

    Card *use_card = Sanguosha->cloneCard(to_use, Card::NoSuit, 0);
    use_card->setSkillName("youwang");
    use_card->deleteLater();
    use_card->setFlags("YouwangFlag");
    return use_card;
}

class Youwang : public ZeroCardViewAsSkill
{

public:
	Youwang() : ZeroCardViewAsSkill("youwang")
	{
		frequency = Limited;
		limit_mark = "@lure";
	}

	QDialog *getDialog() const
	{
		return YaoshuDialog::getInstance("youwang", false);
	}

	bool isEnabledAtPlay(const Player *ex_yuyuko) const
	{
		return ex_yuyuko->getMark("@lure") > 0 && ex_yuyuko->isWounded();
	}

	const Card *viewAs() const
	{
		const Card *c = Self->tag.value("youwang").value<const Card *>();
        if (c) {
            YouwangCard *card = new YouwangCard;
            card->setUserString(c->objectName());
            return card;
        } else
            return NULL;
	}
};

class YouwangAdd : public TriggerSkill
{

public:
	YouwangAdd() : TriggerSkill("#youwang-add")
	{
		events << TargetConfirmed;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *ex_yuyuko = use.from;
		if (ex_yuyuko && ex_yuyuko->isAlive() && use.card->hasFlag("YouwangFlag") && !use.to.isEmpty())
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_yuyuko, ex_yuyuko, NULL, true);
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_yuyuko = invoke->invoker;
		CardUseStruct use = data.value<CardUseStruct>();
		foreach (ServerPlayer *p, use.to) {
			QString choice = room->askForChoice(p, objectName(), "YouwangThrow+YouwangDamage");
			if (choice == "YouwangThrow" && ex_yuyuko->isWounded()) {
				if (ex_yuyuko->getLostHp() >= p->getHandcardNum())
					p->throwAllHandCards();
				else
					room->askForDiscard(p, objectName(), ex_yuyuko->getLostHp(), ex_yuyuko->getLostHp(), false, false, "@youwang-discard:" + ex_yuyuko->objectName());
			} else if (choice == "YouwangDamage")
				room->damage(DamageStruct(objectName(), ex_yuyuko, p));
		}
		return false;
	}
};

ZaobiCard::ZaobiCard()
{
}

bool ZaobiCard::willThrow() const
{
	//return Self->hasFlag("Zaobi2");
	return false;
}

bool ZaobiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	if (Self->hasFlag("Zaobi1"))
		return targets.length() < 3 && !to_select->isKongcheng() && to_select != Self;
	//else if (Self->hasFlag("Zaobi2"))
	//	return to_select->hasFlag("ZaobiValid");
	return false;
}

void ZaobiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	if (source->hasFlag("Zaobi1")) {
		foreach (ServerPlayer *p, targets) {
			source->pindian(p, "zaobi", Sanguosha->getCard(getSubcards().at(0)));
		}
	} /*else if (source->hasFlag("Zaobi2")) {
		foreach (ServerPlayer *p, targets) {
			room->setPlayerFlag(p, "ZaobiExtra");
		}
	}*/
}

class ZaobiVS : public OneCardViewAsSkill
{

public:
	ZaobiVS() : OneCardViewAsSkill("zaobi")
	{
		response_pattern = "@@zaobi";
	}

	bool viewFilter(const QList<const Card *> &, const Card *to_select) const
	{
		return !to_select->isEquipped();
	}

	bool isEnabledAtPlay(const Player *ex_seiga) const
	{
		return false;
	}

	bool isEnabledAtResponse(const Player *ex_seiga, const QString &pattern) const
	{
		return pattern == "@@zaobi";
	}

	const Card *viewAs(const Card *originalCard) const
	{
		ZaobiCard *card = new ZaobiCard;
		card->addSubcard(originalCard);
		return card;
	}
};

class Zaobi : public TriggerSkill
{

public:
	Zaobi() : TriggerSkill("zaobi")
	{
		events << EventPhaseStart << EventPhaseChanging << Pindian;
		view_as_skill = new ZaobiVS;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
	{
		if (e == EventPhaseStart) {
			ServerPlayer *ex_seiga = data.value<ServerPlayer *>();
			if (ex_seiga && ex_seiga->isAlive() && ex_seiga->hasSkill(this) && ex_seiga->getPhase() == Player::Play && !ex_seiga->isKongcheng())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_seiga, ex_seiga, NULL, false);
		} else if (e == EventPhaseChanging) {
			PhaseChangeStruct change = data.value<PhaseChangeStruct>();
			ServerPlayer *ex_seiga = change.player;
			if (ex_seiga && ex_seiga->isAlive() && ex_seiga->hasSkill(this) && change.from == Player::Finish)
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_seiga, ex_seiga, NULL, true);
		} else if (e == Pindian) {
			PindianStruct *pindian = data.value<PindianStruct *>();
			ServerPlayer *ex_seiga = pindian->from;
			if (ex_seiga && ex_seiga->isAlive() && pindian->to && pindian->to->isAlive() && pindian->reason == objectName())
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_seiga, ex_seiga, NULL, true, pindian->to);
		} /*else if (e == TargetSpecifying) {
			CardUseStruct use = data.value<CardUseStruct>();
			ServerPlayer *ex_seiga = use.from;
			if (ex_seiga && ex_seiga->isAlive() && ex_seiga->hasSkill(this) && (use.card->isKindOf("Slash") || use.card->isNDTrick())) {
				foreach (ServerPlayer *p, room->getOtherPlayers(ex_seiga)) {
					if (p->hasFlag("ZaobiTarget") && use.card->targetFilter(QList<const Player *>(), p, ex_seiga)
							&& !room->isProhibited(ex_seiga, p, use.card, QList<const Player *>()) && !use.to.contains(p))
						return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_seiga, ex_seiga, NULL, false);
				}
			}
		}*/
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		if (e == EventPhaseStart) {
			room->setPlayerFlag(invoke->invoker, "Zaobi1");
			bool yes = room->askForUseCard(invoke->invoker, "@@zaobi", "@zaobi", -1, Card::MethodDiscard, false, objectName());
			room->setPlayerFlag(invoke->invoker, "-Zaobi1");
		} /*else if (e == TargetSpecifying) {
			ServerPlayer *ex_seiga = invoke->invoker;
			CardUseStruct use = data.value<CardUseStruct>();
			foreach (ServerPlayer *p, room->getOtherPlayers(ex_seiga)) {
				if (p->hasFlag("ZaobiTarget") && use.card->targetFilter(QList<const Player *>(), p, ex_seiga)
						&& !room->isProhibited(ex_seiga, p, use.card, QList<const Player *>()) && !use.to.contains(p))
					room->setPlayerFlag(p, "ZaobiValid");
			}
			room->setPlayerFlag(ex_seiga, "Zaobi2");
			bool yes = room->askForUseCard(invoke->invoker, "@@zaobi", "@zaobi-extra", -1, Card::MethodDiscard);
			foreach (ServerPlayer *p, room->getOtherPlayers(ex_seiga)) {
				room->setPlayerFlag(p, "-ZaobiValid");
			}
			room->setPlayerFlag(ex_seiga, "-Zaobi2");
			return yes;
		}*/
		return true;
	}

	bool effect(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_seiga = invoke->invoker;
		if (e == EventPhaseChanging) {
			foreach (ServerPlayer *p, room->getAlivePlayers()) {
				room->setPlayerFlag(p, "-ZaobiProhibit");
				//room->setPlayerMark(p, "Armor_Nullified", 0);
				room->setPlayerFlag(p, "-ZaobiTarget");
				room->setFixedDistance(ex_seiga, p, -1);
			}
		} else if (e == Pindian) {
			ServerPlayer *target = invoke->preferredTarget;
			PindianStruct *pindian = data.value<PindianStruct *>();
			bool success = pindian->from_number > pindian->to_number;
			if (success) {
				if (!target->isKongcheng())
					room->askForDiscard(target, objectName(), 1, 1, false, false, "@zaobi-discard:" + ex_seiga->objectName());
				//room->setPlayerMark(target, "Armor_Nullified", 1);
				room->setPlayerFlag(target, "ZaobiTarget");
				room->setFixedDistance(ex_seiga, target, 1);
			} else {
				room->damage(DamageStruct(objectName(), target, ex_seiga));
				room->setPlayerFlag(target, "ZaobiProhibit");
			}
		} /*else if (e == TargetSpecifying) {
			CardUseStruct use = data.value<CardUseStruct>();
			foreach (ServerPlayer *p, room->getOtherPlayers(ex_seiga)) {
				if (p->hasFlag("ZaobiExtra"))
					use.to.append(p);
				room->setPlayerFlag(p, "-ZaobiExtra");
			}
			room->sortByActionOrder(use.to);
			data = QVariant::fromValue(use);
		}*/
		return false;
	}
};

/* ZaobiExtraTargetCard::ZaobiExtraTargetCard()
{

}

bool ZaobiExtraTargetCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	return to_select->hasFlag("ZaobiValid");
}

void ZaobiExtraTargetCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
	foreach (ServerPlayer *p, targets)
		room->setPlayerFlag(p, "ZaobiExtra");
}

class ZaobiExtraTargetVS : public OneCardViewAsSkill
{

public:
	ZaobiExtraTargetVS() : OneCardViewAsSkill("zaobiextratarget")
	{
		response_pattern = "@@zaobiextratarget";
		filter_pattern = ".|.|.|hand";
	}

	const Card *viewAs(const Card *originalCard) const
	{
		ZaobiExtraTargetCard *card = new ZaobiExtraTargetCard;
		card->addSubcard(originalCard);
		return card;
	}
}; */

class ZaobiExtraTarget : public TriggerSkill
{

public:
	ZaobiExtraTarget() : TriggerSkill("#zaobiextratarget")
	{
		events << TargetSpecifying;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		CardUseStruct use = data.value<CardUseStruct>();
		ServerPlayer *ex_seiga = use.from;
		if (ex_seiga && ex_seiga->isAlive() && ex_seiga->hasSkill("zaobi") && (use.card->isKindOf("Slash") || use.card->isNDTrick())) {
			foreach (ServerPlayer *p, room->getOtherPlayers(ex_seiga)) {
				if (p->hasFlag("ZaobiTarget") && use.card->targetFilter(QList<const Player *>(), p, ex_seiga)
						&& !room->isProhibited(ex_seiga, p, use.card, QList<const Player *>()) && !use.to.contains(p))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_seiga, ex_seiga, NULL, false);
			}
		}
		return QList<SkillInvokeDetail>();
	}

	bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *ex_seiga = invoke->invoker;
		return room->askForCard(invoke->invoker, ".|.|.|hand", "@zaobi-extra", data, Card::MethodDiscard, NULL, false, "zaobi");
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
        ServerPlayer *ex_seiga = invoke->invoker;
		CardUseStruct use = data.value<CardUseStruct>();
		foreach (ServerPlayer *p, room->getOtherPlayers(ex_seiga)) {
			if (p->hasFlag("ZaobiTarget") && use.card->targetFilter(QList<const Player *>(), p, ex_seiga) && !use.to.contains(p)
					&& !room->isProhibited(ex_seiga, p, use.card, QList<const Player *>()))
				use.to.append(p);
		}
		room->sortByActionOrder(use.to);
		data = QVariant::fromValue(use);
		return false;
	}
};

class ZaobiPro : public ProhibitSkill
{

public:
	ZaobiPro() : ProhibitSkill("#zaobi-pro")
	{
	}

	bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &) const
	{
		return from->hasSkill("zaobi") && to->hasFlag("ZaobiProhibit");
	}
};

class Rumo : public TriggerSkill
{

public:
	Rumo() : TriggerSkill("rumo")
	{
		events << ConfirmDamage;
		frequency = Compulsory;
	}

	QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
	{
		DamageStruct damage = data.value<DamageStruct>();
		ServerPlayer *ex_seiga = room->findPlayerBySkillName(objectName());
		if (ex_seiga && ex_seiga->isAlive() && ((ex_seiga == damage.from && damage.to && ex_seiga->distanceTo(damage.to) <= 1)
				|| (ex_seiga == damage.to && damage.from && !damage.from->inMyAttackRange(ex_seiga))))
			return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, ex_seiga, ex_seiga, NULL, true);
		return QList<SkillInvokeDetail>();
	}

	bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		room->sendCompulsoryTriggerLog(invoke->invoker, objectName());
		DamageStruct damage = data.value<DamageStruct>();
		damage.damage++;
		data = QVariant::fromValue(damage);
		return false;
	}
};

THLunaPackage::THLunaPackage()
	: Package("thluna")
{
	General *benben = new General(this, "benben", "hakurei", 4, false);
	benben->addSkill(new Yueyin);
	benben->addSkill(new Liebo);

	General *yatsuhashi = new General(this, "yatsuhashi", "hakurei", 3, false);
	yatsuhashi->addSkill(new Duanxiang);
	yatsuhashi->addSkill(new Jinxian);

	General *kosuzu = new General(this, "kosuzu", "hakurei", 3, false);
	kosuzu->addSkill(new Jieyu);
	kosuzu->addSkill(new Yaoshu);

	General *clownpiece = new General(this, "clownpiece", "hakurei", 4, false);
	clownpiece->addSkill(new Xiaoyan);

	General *ringo = new General(this, "ringo", "moriya", 4, false);
	ringo->addSkill(new Wanchu);
	ringo->addSkill(new WanchuClear);
	related_skills.insertMulti("wanchu", "#wanchu-clear");

	General *seiran = new General(this, "seiran", "moriya", 3, false);
	seiran->addSkill(new Yingdan);
	seiran->addSkill(new YingdanClear);
	related_skills.insertMulti("yingdan", "#yingdan-clear");
	seiran->addSkill(new Yuechong);

	General *sumireko = new General(this, "sumireko", "moriya", 4, false);
	sumireko->addSkill(new Nianli);
	sumireko->addSkill(new NianliClear);
	related_skills.insertMulti("nianli", "#nianli-clear");
	sumireko->addSkill(new Mengwei);
	sumireko->addSkill(new RuhuanDist);
	related_skills.insertMulti("ruhuan", "#ruhuan-dist");

	General *kogasa = new General(this, "kogasa", "moriya", 3, false);
	kogasa->addSkill(new Donghe);
	kogasa->addSkill(new Yeyu);

	General *ex_yuyuko = new General(this, "ex_yuyuko", "god", 3, false);
	ex_yuyuko->addSkill(new Mieli);
	ex_yuyuko->addSkill(new Zijin);
	ex_yuyuko->addSkill(new Youwang);
	ex_yuyuko->addSkill(new YouwangAdd);
	related_skills.insertMulti("youwang", "#youwang-add");

	General *ex_seiga = new General(this, "ex_seiga", "god", 5, false);
	ex_seiga->addSkill(new Zaobi);
	ex_seiga->addSkill(new ZaobiPro);
	ex_seiga->addSkill(new ZaobiExtraTarget);
	related_skills.insertMulti("zaobi", "#zaobi-pro");
	ex_seiga->addSkill(new Rumo);

	addMetaObject<LieboCard>();
	addMetaObject<DuanxiangCard>();
	addMetaObject<YaoshuCard>();
	addMetaObject<YingdanCard>();
	addMetaObject<NianliCard>();
	addMetaObject<DongheCard>();
	addMetaObject<YeyuCard>();
	addMetaObject<WanchuCard>();
	addMetaObject<MieliCard>();
	addMetaObject<YouwangCard>();
	addMetaObject<ZaobiCard>();
	//addMetaObject<ZaobiExtraTargetCard>();

	skills << new Ruhuan;
}

ADD_PACKAGE(THLuna)

