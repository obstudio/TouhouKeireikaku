#include "package.h"
#include "card.h"

class THWeavePackage : public Package
{
	Q_OBJECT
	
public:
	THWeavePackage();
};

class JiejieCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE JiejieCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class ShishenCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE ShishenCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class YuanqiCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE YuanqiCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class MishiCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE MishiCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class MiezuiCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE MiezuiCard();
	
};

class KuaiqingCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE KuaiqingCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HerongCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE HerongCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YuzhiCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE YuzhiCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LongzuanCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE LongzuanCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};