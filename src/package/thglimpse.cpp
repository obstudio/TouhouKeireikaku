#include "thglimpse.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "room.h"
#include "WrappedCard.h"
#include "roomthread.h"

DoomNight::DoomNight(Suit suit, int number)
	: SingleTargetTrick(suit, number)
{
	setObjectName("doom_night");
}

bool DoomNight::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);
    return targets.length() < total_num && to_select != Self && !to_select->isNude();
}

void DoomNight::onEffect(const CardEffectStruct &effect) const
{
	ServerPlayer *from = effect.from;
	ServerPlayer *to = effect.to;
	Room *room = from->getRoom();
	
	to->tag["DoomNightSource"] = QVariant::fromValue(from);
    if (!to->isNude())
		room->askForDiscard(to, objectName(), 1, 1, false, true, "@doom_night-1");
	if (!(!to->isNude() && room->askForDiscard(to, "DoomNightExtra", 1, 1, true, true, "@doom_night-2"))) {
		RecoverStruct recover;
		recover.who = from;
		recover.reason = objectName();
		recover.recover = 1;
		room->recover(from, recover);
	}
}

Oracle::Oracle(Suit suit, int number)
	: DelayedTrick(suit, number)
{
	setObjectName("oracle");
	
	judge.pattern = ".|diamond";
	judge.good = true;
	judge.reason = objectName();
}

bool Oracle::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
	bool ignore = (Self && Self->hasSkill("tianqu") && Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY && to_select != Self && !hasFlag("IgnoreFailed"));
    return targets.isEmpty() && (!to_select->containsTrick(objectName()) || ignore)
            && to_select != Self;
}

void Oracle::takeEffect(ServerPlayer *target) const
{
	target->drawCards(2);
	RecoverStruct recover;
	recover.who = NULL;
	recover.recover = 1;
	recover.reason = objectName();
	target->getRoom()->recover(target, recover);
}

THGlimpsePackage::THGlimpsePackage()
    : Package("thglimpse", Package::CardPack)
{
    QList<Card *> cards;

    // spade
	cards << new DoomNight(Card::Spade, 5);
	
    // club
	cards << new DoomNight(Card::Spade, 5);
	
    // heart
	
    // diamond
	cards << new Oracle(Card::Diamond, 8);
	cards << new Oracle(Card::Diamond, 9);
    
    foreach(Card *card, cards)
        card->setParent(this);
	
}

ADD_PACKAGE(THGlimpse)

