#include "thsoul.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "room.h"
#include "WrappedCard.h"
#include "roomthread.h"

SoulSlash::SoulSlash(Suit suit, int number)
	: NatureSlash(suit, number, DamageStruct::Soul)
{
	setObjectName("soul_slash");
	nature = DamageStruct::Soul;
}

NecroMusic::NecroMusic(Suit suit, int number)
	: AOE(suit, number)
{
	setObjectName("necro_music");
}

void NecroMusic::onEffect(const CardEffectStruct &effect) const
{
	Room *room = effect.to->getRoom();
	const Card *nonbasic = room->askForCard(effect.to,
                                         "TrickCard,EquipCard",
                                         "necro-music-card-ask:" + effect.from->objectName(),
                                         QVariant::fromValue(effect),
                                         Card::MethodResponse,
                                         effect.from->isAlive() ? effect.from : NULL);
    if (!nonbasic) {
        room->damage(DamageStruct(this, effect.from->isAlive() ? effect.from : NULL, effect.to, 1, DamageStruct::Soul));
        room->getThread()->delay();
    }
}

THSoulPackage::THSoulPackage()
    : Package("thsoul", Package::CardPack)
{
    QList<Card *> cards;

    // spade
	cards << new SoulSlash(Card::Spade, 1)
		<< new SoulSlash(Card::Spade, 4)
		<< new SoulSlash(Card::Spade, 6);
    
    // club
	cards << new SoulSlash(Card::Club, 2)
		<< new SoulSlash(Card::Club, 5)
		<< new SoulSlash(Card::Club, 6);

    // heart
	cards << new NecroMusic(Card::Heart, 9)
        << new Jink(Card::Heart, 3)
        << new Jink(Card::Heart, 4)
        << new Jink(Card::Heart, 5);
    
    // diamond
    cards << new Jink(Card::Diamond, 12);
    
    foreach(Card *card, cards)
        card->setParent(this);
	
}

ADD_PACKAGE(THSoul)

