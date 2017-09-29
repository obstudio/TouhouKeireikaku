#include "package.h"
#include "card.h"

class THShadowPackage : public Package
{
	Q_OBJECT
	
public:
	THShadowPackage();
};


class LiuyueCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE LiuyueCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class LieCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE LieCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class JulangCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE JulangCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class BamaoCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE BamaoCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class YingchongCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE YingchongCard();
	
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class AnchaoCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE AnchaoCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class JinpanCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE JinpanCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HuawuCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE HuawuCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	bool targetsFeasible(const QList<const Player *> &targets, const Player *) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FeixiangCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE FeixiangCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class GuiqiaoCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE GuiqiaoCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class ShenquanCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE ShenquanCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class ShenfengCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE ShenfengCard();
	
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};