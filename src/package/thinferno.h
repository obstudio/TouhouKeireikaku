#ifndef _THINFERNO_H
#define _THINFERNO_H

#include "standard.h"

class PhoenixFlame : public SingleTargetTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE PhoenixFlame(Card::Suit suit, int number);

	QString getSubtype() const;
	
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class AzraelScythe : public Weapon
{
    Q_OBJECT

public:
    Q_INVOKABLE AzraelScythe(Card::Suit suit, int number);
};

class Yasakaninomagatama : public Treasure
{
	Q_OBJECT
	
public:
	Q_INVOKABLE Yasakaninomagatama(Card::Suit suit, int number);
};

class THInfernoPackage : public Package
{
    Q_OBJECT

public:
    THInfernoPackage();
};

#endif

