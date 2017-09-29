#ifndef _THSOUL_H
#define _THSOUL_H

#include "standard.h"
#include "maneuvering.h"

class SoulSlash : public NatureSlash
{
    Q_OBJECT

public:
    Q_INVOKABLE SoulSlash(Card::Suit suit, int number);
};

class NecroMusic : public AOE
{
	Q_OBJECT
	
public:
	Q_INVOKABLE NecroMusic(Card::Suit suit, int number);
	
	void onEffect(const CardEffectStruct &effect) const;
};

class THSoulPackage : public Package
{
    Q_OBJECT

public:
    THSoulPackage();
};

#endif

