#ifndef _THGLIMPSE_H
#define _THGLIMPSE_H

#include "standard.h"
#include "maneuvering.h"

class DoomNight : public SingleTargetTrick
{
	Q_OBJECT
	
public:
	Q_INVOKABLE DoomNight(Card::Suit suit, int number);
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class Oracle : public DelayedTrick
{
	Q_OBJECT
	
public:
	Q_INVOKABLE Oracle(Card::Suit suit, int number);
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void takeEffect(ServerPlayer *target) const;
};

class THGlimpsePackage : public Package
{
    Q_OBJECT

public:
    THGlimpsePackage();
};

#endif

