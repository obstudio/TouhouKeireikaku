#include "package.h"
#include "card.h"

class THFaithPackage : public Package
{
	Q_OBJECT
	
public:
	THFaithPackage();
};

class YoudaoCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE YoudaoCard();
	
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class ShanchouCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE ShanchouCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	bool targetsFeasible(const QList<const Player *> &targets, const Player *) const;
	void onUse(Room *room, const CardUseStruct &use) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JishaCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE JishaCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class DiwenCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE DiwenCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const;
};

class ShehunCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE ShehunCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class YuansheCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE YuansheCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class HuoyunCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE HuoyunCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class ShimianCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE ShimianCard();
	
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class JiwuCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE JiwuCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	bool targetsFeasible(const QList<const Player *> &targets, const Player *) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShixiCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE ShixiCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class WangjieCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE WangjieCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};