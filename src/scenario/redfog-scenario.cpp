#include "redfog-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "standard.h"
#include "room.h"
#include "roomthread.h"

class RedfogRule : public ScenarioRule
{
public:
    RedfogRule(Scenario *scenario)
        : ScenarioRule(scenario)
    {
        events << GameStart;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        switch (triggerEvent) {
        case GameStart: {
			ServerPlayer *remilia = room->findPlayer("remilia");
			room->installEquip(remilia, "QinggangSword");
			room->acquireSkill(remilia, "shengqiang");
			room->acquireSkill(remilia, "#shengqiang-maso");
			room->acquireSkill(remilia, "hongye");
			
			ServerPlayer *flandre = room->findPlayer("flandre");
			room->installEquip(flandre, "Fan");
			room->acquireSkill(flandre, "shengxue");
			room->acquireSkill(flandre, "wosui");
			room->acquireSkill(flandre, "heiyan");
			
			ServerPlayer *pachouli = room->findPlayer("pachouli");
			room->installEquip(pachouli, "Spear");
			room->installEquip(pachouli, "Vine");
			room->acquireSkill(pachouli, "yaoshi");
			room->acquireSkill(pachouli, "huangyan");
			room->acquireSkill(pachouli, "jingyue");
			
            break;
        }
        default:
            break;
        }

        return false;
    }
};

bool RedfogScenario::exposeRoles() const
{
    return true;
}

void RedfogScenario::assign(QStringList &generals, QStringList &roles) const
{
    Q_UNUSED(generals);

    roles << "lord";
    int i;
    for (i = 0; i < 3; i++)
        roles << "rebel";
	for (i = 0; i < 2; i++)
		roles << "loyalist";

    qShuffle(roles);
}

int RedfogScenario::getPlayerCount() const
{
    return 3;
}

QString RedfogScenario::getRoles() const
{
    return "FFF";
}

bool RedfogScenario::generalSelection() const
{
    return true;
}

RedfogScenario::RedfogScenario()
    : Scenario("redfog")
{
    lord = "remilia";
    loyalists << "flandre" << "pachouli";

    rule = new RedfogRule(this);

	//skills
	
	//addMetaObject
	
}

void RedfogScenario::onTagSet(Room *room, const QString &key) const
{
    //dummy
}

