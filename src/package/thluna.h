#ifndef _thluna_H
#define _thluna_H

#include "package.h"
#include "card.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>

class THLunaPackage : public Package
{
	Q_OBJECT
	
public:
	THLunaPackage();
};

class LieboCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE LieboCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DuanxiangCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE DuanxiangCard();

	void  use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class YaoshuDialog : public QDialog
{
    Q_OBJECT

public:
    static YaoshuDialog *getInstance(const QString &object, bool left = true, bool right = true);

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    explicit YaoshuDialog(const QString &object, bool left = true, bool right = true);

    QGroupBox *createLeft();
    QGroupBox *createRight();
    QAbstractButton *createButton(const Card *card);
    QButtonGroup *group;
    QHash<QString, const Card *> map;

    QString object_name;

signals:
    void onButtonClick();
};

class YaoshuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YaoshuCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
};

class YingdanCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE YingdanCard();

	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class NianliCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE NianliCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	bool targetsFeasible(const QList<const Player *> &targets, const Player *) const;
	void onUse(Room *room, const CardUseStruct &use) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DongheCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE DongheCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class YeyuCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE YeyuCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class WanchuCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE WanchuCard();

	bool targetFixed() const;
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void onUse(Room *room, const CardUseStruct &use) const;
	void onEffect(const CardEffectStruct &effect) const;
};

class MieliCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE MieliCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YouwangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YouwangCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(CardUseStruct &card_use) const;
};

class ZaobiCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE ZaobiCard();

	bool willThrow() const;
	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

/*class ZaobiExtraTargetCard : public SkillCard
{
	Q_OBJECT

public:
	Q_INVOKABLE ZaobiExtraTargetCard();

	bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
	void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};*/

#endif