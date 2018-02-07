#include "package.h"
#include "card.h"

class THStandardPackage : public Package
{
	Q_OBJECT
	
public:
	THStandardPackage();
};

class FengmoCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE FengmoCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class ShantouCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE ShantouCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class DaosheCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE DaosheCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class HeiyanCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE HeiyanCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HuangyanCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE HuangyanCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DiaoouCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE DiaoouCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class AnjiCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE AnjiCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class YaofengCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE YaofengCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class YuzhuCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE YuzhuCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class ChiwaCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE ChiwaCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class FangyingCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE FangyingCard();
	
	void use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class DuannianCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE DuannianCard();
	
	void use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class HunquCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE HunquCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class ZhangqiCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE ZhangqiCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class XianboCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE XianboCard();
	
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class HuanniCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE HuanniCard();
	
	void use(Room *, ServerPlayer *, QList<ServerPlayer *> &) const;
};

class CitanCard : public SkillCard
{
	Q_OBJECT
	
public:
	Q_INVOKABLE CitanCard();
	
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};