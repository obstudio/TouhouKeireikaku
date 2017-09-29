#include "package.h"
#include "card.h"

class THVenusPackage : public Package
{
	Q_OBJECT
	
public:
	THVenusPackage();
};

class FanniCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE FanniCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XiaoanCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE XiaoanCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LingbaiCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE LingbaiCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JinghunCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE JinghunCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const;
};

class CaiyuanCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE CaiyuanCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class JinggeCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE JinggeCard();
	
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class FeimanCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE FeimanCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class YuniCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE YuniCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class JiaomianCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE JiaomianCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class YongyeCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE YongyeCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	bool targetFeasible(const QList<const Player *> &targets, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SuozhenCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE SuozhenCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};
