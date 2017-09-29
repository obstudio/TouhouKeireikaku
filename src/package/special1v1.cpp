#include "special1v1.h"
#include "general.h"
#include "standard.h"
#include "standard-equips.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"
#include "settings.h"
#include "maneuvering.h"
#include "thinferno.h"
#include "util.h"
#include "roomthread.h"

class Xingxie : public TriggerSkill
{
	
public:
	Xingxie() : TriggerSkill("xingxie")
	{
		events << EventPhaseStart << Damaged;
		frequency = Frequent;
	}
	
	QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
	{
		if (event == EventPhaseStart) {
			ServerPlayer *marisa = data.value<ServerPlayer *>();
			if (marisa && marisa->isAlive() && marisa->hasSkill(this)) {
				if (marisa->getPhase() == Player::Finish && marisa->hasFlag("XingxieDamageCaused"))
					return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, marisa, marisa, NULL, true);
			}
		}
        else if (event == Damaged) {
			DamageStruct damage = data.value<DamageStruct>();
			ServerPlayer *marisa = room->findPlayerBySkillName(objectName());
			if (marisa && marisa->isAlive() && marisa == damage.from && marisa->getPhase() == Player::Play
					&& !marisa->hasFlag("XingxieDamageCaused"))
				return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, marisa, marisa, NULL, true);
		}
		return QList<SkillInvokeDetail>();
	}
	
	bool effect(TriggerEvent event, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
	{
		ServerPlayer *marisa = invoke->invoker;
		if (event == EventPhaseStart) {
			room->notifySkillInvoked(marisa, objectName());
			marisa->drawCards(1);
			room->setPlayerFlag(marisa, "-XingxieDamageCaused");
		}
		else if (event == Damaged) {
			room->setPlayerFlag(marisa, "XingxieDamageCaused");
		}
		return false;
	}
};

Special1v1Package::Special1v1Package()
    : Package("Special1v1")
{
    General *kof_marisa = new General(this, "kof_marisa", "hakurei", 3, false);
	kof_marisa->addSkill(new Xingxie);
	kof_marisa->addSkill("sheyue");
}

ADD_PACKAGE(Special1v1)

New1v1CardPackage::New1v1CardPackage()
: Package("New1v1Card")
{
    QList<Card *> cards;
    cards << new Duel(Card::Spade, 1)
		<< new GudingBlade(Card::Spade, 1)
        << new EightDiagram(Card::Spade, 2)
		<< new Slash(Card::Spade, 2)
        << new Dismantlement(Card::Spade, 3)
		<< new Analeptic(Card::Spade, 3)
        << new Snatch(Card::Spade, 4)
		<< new ThunderSlash(Card::Spade, 4)
        << new Slash(Card::Spade, 5)
		<< new Haze(Card::Spade, 5)
        << new QinggangSword(Card::Spade, 6)
		<< new Indulgence(Card::Spade, 6)
        << new Slash(Card::Spade, 7)
		<< new SavageAssault(Card::Spade, 7)
        << new Slash(Card::Spade, 8)
		<< new Dismantlement(Card::Spade, 8)
        << new IceSword(Card::Spade, 9)
		<< new Analeptic(Card::Spade, 9)
        << new Slash(Card::Spade, 10)
		<< new AzraelScythe(Card::Spade, 10)
        << new Snatch(Card::Spade, 11)
        << new ThunderSlash(Card::Spade, 11)
        << new Spear(Card::Spade, 12)
		<< new Slash(Card::Spade, 12)
        << new SavageAssault(Card::Spade, 13)
		<< new Haze(Card::Spade, 13);

    cards << new ArcheryAttack(Card::Heart, 1)
		<< new MindReading(Card::Heart, 1)
        << new Jink(Card::Heart, 2)
		<< new Jink(Card::Heart, 2)
        << new Peach(Card::Heart, 3)
		<< new FireSlash(Card::Heart, 3)
        << new Peach(Card::Heart, 4)
		<< new Slash(Card::Heart, 4)
        << new Jink(Card::Heart, 5)
		<< new Jink(Card::Heart, 5)
        << new Indulgence(Card::Heart, 6)
		<< new PhoenixFlame(Card::Heart, 6)
        << new ExNihilo(Card::Heart, 7)
		<< new Jink(Card::Heart, 7)
        << new ExNihilo(Card::Heart, 8)
		<< new FireSlash(Card::Heart, 8)
        << new Peach(Card::Heart, 9)
		<< new FireAttack(Card::Heart, 9)
        << new Slash(Card::Heart, 10)
		<< new Peach(Card::Heart, 10)
        << new Slash(Card::Heart, 11)
		<< new Jink(Card::Heart, 11)
        << new Dismantlement(Card::Heart, 12)
		<< new MindReading(Card::Heart, 12)
        << new Nullification(Card::Heart, 13)
		<< new Slash(Card::Heart, 13);

    cards << new Duel(Card::Club, 1)
		<< new Crossbow(Card::Club, 1)
        << new RenwangShield(Card::Club, 2)
		<< new Vine(Card::Club, 2)
        << new Dismantlement(Card::Club, 3)
		<< new Blade(Card::Club, 3)
        << new Slash(Card::Club, 4)
		<< new ThunderSlash(Card::Club, 4)
        << new Slash(Card::Club, 5)
		<< new Snatch(Card::Club, 5)
        << new Slash(Card::Club, 6)
		<< new Nullification(Card::Club, 6)
        << new SavageAssault(Card::Club, 7)
		<< new ThunderSlash(Card::Club, 7)
        << new Slash(Card::Club, 8)
		<< new Slash(Card::Club, 8)
        << new Slash(Card::Club, 9)
		<< new Analeptic(Card::Club, 9)
        << new Slash(Card::Club, 10)
		<< new SupplyShortage(Card::Club, 10)
        << new Slash(Card::Club, 11)
		<< new Slash(Card::Club, 11)
        << new SupplyShortage(Card::Club, 12)
		<< new Dismantlement(Card::Club, 12)
        << new Nullification(Card::Club, 13)
		<< new Nullification(Card::Club, 13);

    cards << new Crossbow(Card::Diamond, 1)
		<< new Duel(Card::Diamond, 1)
        << new Jink(Card::Diamond, 2)
		<< new FireAttack(Card::Diamond, 2)
        << new Jink(Card::Diamond, 3)
		<< new Jink(Card::Diamond, 3)
        << new Snatch(Card::Diamond, 4)
		<< new FireSlash(Card::Diamond, 4)
        << new Axe(Card::Diamond, 5)
		<< new Jink(Card::Diamond, 5)
        << new Slash(Card::Diamond, 6)
		<< new Peach(Card::Diamond, 6)
        << new Jink(Card::Diamond, 7)
		<< new Jink(Card::Diamond, 7)
        << new Jink(Card::Diamond, 8)
		<< new PhoenixFlame(Card::Diamond, 8)
        << new Slash(Card::Diamond, 9)
		<< new Slash(Card::Diamond, 9)
        << new Jink(Card::Diamond, 10)
		<< new Jink(Card::Diamond, 10)
        << new Jink(Card::Diamond, 11)
		<< new FireSlash(Card::Diamond, 11)
        << new Peach(Card::Diamond, 12)
		<< new FireAttack(Card::Diamond, 12)
        << new Slash(Card::Diamond, 13)
		<< new Slash(Card::Diamond, 13);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(New1v1Card)
