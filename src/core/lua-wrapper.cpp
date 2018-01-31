#include "lua-wrapper.h"
#include "util.h"
#include "general.h"
#include "settings.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "client.h"
#include <QCommandLinkButton>
//#include "th10.h"

QijiDialog *QijiDialog::getInstance(const QString &object, bool left, bool right)
{
    static QijiDialog *instance;
    if (instance == NULL || instance->objectName() != object) {
        instance = new QijiDialog(object, left, right);
    }
    return instance;
}

QijiDialog::QijiDialog(const QString &object, bool left, bool right) : object_name(object)
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

void QijiDialog::popup()
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
    //if (object_name == "qiji") {
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
        if (object_name == "qiji" && user->getMark("xiubu"))
            avaliable = true;

        bool checked = (checkedPatterns.contains(card->objectName()) || (card->isKindOf("Slash") && checkedPatterns.contains("slash")));
        bool enabled = !user->isCardLimited(card, method, true) && avaliable && (checked || object_name == "chuangshi");
        button->setEnabled(enabled);
    }

    Self->tag.remove(object_name);
    exec();
}

void QijiDialog::selectCard(QAbstractButton *button)
{
    const Card *card = map.value(button->objectName());
    Self->tag[object_name] = QVariant::fromValue(card);

    emit onButtonClick();
    accept();
}

QGroupBox *QijiDialog::createLeft()
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

QGroupBox *QijiDialog::createRight()
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

QAbstractButton *QijiDialog::createButton(const Card *card)
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

LuaTriggerSkill::LuaTriggerSkill(const char *name, Frequency frequency, const char *limit_mark)
    : TriggerSkill(name), on_record(0), can_trigger(0), on_cost(0), on_effect(0)
{
    this->frequency = frequency;
    this->limit_mark = QString(limit_mark);
    this->priority = (frequency == Skill::Wake) ? 3 : 2;
}

LuaProhibitSkill::LuaProhibitSkill(const char *name)
    : ProhibitSkill(name), is_prohibited(0)
{
}

LuaViewAsSkill::LuaViewAsSkill(const char *name, const char *response_pattern)
    : ViewAsSkill(name), view_filter(0), view_as(0), should_be_visible(0),
      enabled_at_play(0), enabled_at_response(0), enabled_at_nullification(0), guhuo_dialog_type(NoDialog)
{
    this->response_pattern = response_pattern;
}

QDialog *LuaViewAsSkill::getDialog() const
{
    int dialog_type = (int)guhuo_dialog_type;
    if (dialog_type == 0)
        return NULL;

    bool has_left = (dialog_type & 1);
    bool has_right = (dialog_type & 2);

    return QijiDialog::getInstance(objectName(), has_left, has_right);
}

LuaFilterSkill::LuaFilterSkill(const char *name)
    : FilterSkill(name), view_filter(0), view_as(0)
{
}

LuaDistanceSkill::LuaDistanceSkill(const char *name)
    : DistanceSkill(name), correct_func(0)
{
}

LuaMaxCardsSkill::LuaMaxCardsSkill(const char *name)
    : MaxCardsSkill(name), extra_func(0), fixed_func(0)
{
}

LuaTargetModSkill::LuaTargetModSkill(const char *name, const char *pattern)
    : TargetModSkill(name), no_limit_func(0), distance_limit_func(0), extra_target_func(0)
{
    this->pattern = pattern;
}

LuaAttackRangeSkill::LuaAttackRangeSkill(const char *name)
    : AttackRangeSkill(name), extra_func(0), fixed_func(0)
{
}

static QHash<QString, const LuaSkillCard *> LuaSkillCards;
static QHash<QString, QString> LuaSkillCardsSkillName;

LuaSkillCard::LuaSkillCard(const char *name, const char *skillName)
    : SkillCard(), filter(0), feasible(0),
      about_to_use(0), on_use(0), on_effect(0), on_validate(0), on_validate_in_response(0)
{
    if (name) {
        LuaSkillCards.insert(name, this);
        if (skillName) {
            m_skillName = skillName;
            LuaSkillCardsSkillName.insert(name, skillName);
        }
        setObjectName(name);
    }
}

LuaSkillCard *LuaSkillCard::clone() const
{
    LuaSkillCard *new_card = new LuaSkillCard(NULL, NULL);

    new_card->setObjectName(objectName());
    new_card->setSkillName(m_skillName);

    new_card->target_fixed = target_fixed;
    new_card->will_throw = will_throw;
    new_card->can_recast = can_recast;
    new_card->handling_method = handling_method;

    new_card->filter = filter;
    new_card->feasible = feasible;
    new_card->about_to_use = about_to_use;
    new_card->on_use = on_use;
    new_card->on_effect = on_effect;
    new_card->on_validate = on_validate;
    new_card->on_validate_in_response = on_validate_in_response;

    return new_card;
}

LuaSkillCard *LuaSkillCard::Parse(const QString &str)
{
    QRegExp rx("#(\\w+):(.*):(.*)");
    QRegExp e_rx("#(\\w*)\\[(\\w+):(.+)\\]:(.*):(.*)");

    static QMap<QString, Card::Suit> suit_map;
    if (suit_map.isEmpty()) {
        suit_map.insert("spade", Card::Spade);
        suit_map.insert("club", Card::Club);
        suit_map.insert("heart", Card::Heart);
        suit_map.insert("diamond", Card::Diamond);
        suit_map.insert("no_suit_red", Card::NoSuitRed);
        suit_map.insert("no_suit_black", Card::NoSuitBlack);
        suit_map.insert("no_suit", Card::NoSuit);
    }

    QStringList texts;
    QString name, suit, number;
    QString subcard_str;
    QString user_string;

    if (rx.exactMatch(str)) {
        texts = rx.capturedTexts();
        name = texts.at(1);
        subcard_str = texts.at(2);
        user_string = texts.at(3);
    } else if (e_rx.exactMatch(str)) {
        texts = e_rx.capturedTexts();
        name = texts.at(1);
        suit = texts.at(2);
        number = texts.at(3);
        subcard_str = texts.at(4);
        user_string = texts.at(5);
    } else
        return NULL;

    const LuaSkillCard *c = LuaSkillCards.value(name, NULL);
    if (c == NULL)
        return NULL;

    LuaSkillCard *new_card = c->clone();

    if (subcard_str != ".")
        new_card->addSubcards(StringList2IntList(subcard_str.split("+")));

    if (!suit.isEmpty())
        new_card->setSuit(suit_map.value(suit, Card::NoSuit));
    if (!number.isEmpty()) {
        int num = 0;
        if (number == "A")
            num = 1;
        else if (number == "J")
            num = 11;
        else if (number == "Q")
            num = 12;
        else if (number == "K")
            num = 13;
        else
            num = number.toInt();

        new_card->setNumber(num);
    }

    new_card->setUserString(user_string);
    QString skillName = LuaSkillCardsSkillName.value(name, QString());
    if (skillName.isEmpty())
        skillName = name.toLower().remove("card");
    new_card->setSkillName(skillName);
    return new_card;
}

QString LuaSkillCard::toString(bool hidden) const
{
    Q_UNUSED(hidden);
    return QString("#%1[%2:%3]:%4:%5").arg(objectName())
            .arg(getSuitString()).arg(getNumberString())
            .arg(subcardString()).arg(user_string);
}

LuaBasicCard::LuaBasicCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype)
    : BasicCard(suit, number), filter(0), feasible(0), available(0), about_to_use(0), on_use(0), on_effect(0)
{
    setObjectName(obj_name);
    this->class_name = class_name;
    this->subtype = subtype;
}

LuaBasicCard *LuaBasicCard::clone(Card::Suit suit, int number) const
{
    if (suit == Card::SuitToBeDecided) suit = getSuit();
    if (number == -1) number = getNumber();
    LuaBasicCard *new_card = new LuaBasicCard(suit, number, objectName().toStdString().c_str(), class_name.toStdString().c_str(), subtype.toStdString().c_str());
    new_card->subtype = subtype;

    new_card->target_fixed = target_fixed;
    new_card->can_recast = can_recast;

    new_card->filter = filter;
    new_card->feasible = feasible;
    new_card->available = available;
    new_card->about_to_use = about_to_use;
    new_card->on_use = on_use;
    new_card->on_effect = on_effect;

    return new_card;
}

LuaTrickCard::LuaTrickCard(Card::Suit suit, int number, const char *obj_name, const char *class_name, const char *subtype)
    : TrickCard(suit, number), filter(0), feasible(0), available(0), is_cancelable(0),
      about_to_use(0), on_use(0), on_effect(0), on_nullified(0)
{
    setObjectName(obj_name);
    this->class_name = class_name;
    this->subtype = subtype;
}

LuaTrickCard *LuaTrickCard::clone(Card::Suit suit, int number) const
{
    if (suit == Card::SuitToBeDecided) suit = getSuit();
    if (number == -1) number = getNumber();
    LuaTrickCard *new_card = new LuaTrickCard(suit, number, objectName().toStdString().c_str(), class_name.toStdString().c_str(), subtype.toStdString().c_str());
    new_card->subclass = subclass;
    new_card->subtype = subtype;

    new_card->target_fixed = target_fixed;
    new_card->can_recast = can_recast;

    new_card->filter = filter;
    new_card->feasible = feasible;
    new_card->available = available;
    new_card->is_cancelable = is_cancelable;
    new_card->about_to_use = about_to_use;
    new_card->on_use = on_use;
    new_card->on_effect = on_effect;
    new_card->on_nullified = on_nullified;

    return new_card;
}

LuaWeapon::LuaWeapon(Card::Suit suit, int number, int range, const char *obj_name, const char *class_name)
    : Weapon(suit, number, range)
{
    setObjectName(obj_name);
    this->class_name = class_name;
}

LuaWeapon *LuaWeapon::clone(Card::Suit suit, int number) const
{
    if (suit == Card::SuitToBeDecided) suit = getSuit();
    if (number == -1) number = getNumber();
    LuaWeapon *new_card = new LuaWeapon(suit, number, getRange(), objectName().toStdString().c_str(), class_name.toStdString().c_str());

    new_card->on_install = on_install;
    new_card->on_uninstall = on_uninstall;

    return new_card;
}

LuaArmor::LuaArmor(Card::Suit suit, int number, const char *obj_name, const char *class_name)
    : Armor(suit, number)
{
    setObjectName(obj_name);
    this->class_name = class_name;
}

LuaArmor *LuaArmor::clone(Card::Suit suit, int number) const
{
    if (suit == Card::SuitToBeDecided) suit = getSuit();
    if (number == -1) number = getNumber();
    LuaArmor *new_card = new LuaArmor(suit, number, objectName().toStdString().c_str(), class_name.toStdString().c_str());

    new_card->on_install = on_install;
    new_card->on_uninstall = on_uninstall;

    return new_card;
}

LuaTreasure::LuaTreasure(Card::Suit suit, int number, const char *obj_name, const char *class_name)
    : Treasure(suit, number)
{
    setObjectName(obj_name);
    this->class_name = class_name;
}

LuaTreasure *LuaTreasure::clone(Card::Suit suit, int number) const
{
    if (suit == Card::SuitToBeDecided) suit = getSuit();
    if (number == -1) number = getNumber();
    LuaTreasure *new_card = new LuaTreasure(suit, number, objectName().toStdString().c_str(), class_name.toStdString().c_str());

    new_card->on_install = on_install;
    new_card->on_uninstall = on_uninstall;

    return new_card;
}
