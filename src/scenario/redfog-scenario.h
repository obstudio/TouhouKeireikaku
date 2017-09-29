#ifndef _REDFOG_SCENARIO_H
#define _REDFOG_SCENARIO_H

#include "scenario.h"
#include "card.h"

class RedfogScenario : public Scenario
{
    Q_OBJECT

public:
    RedfogScenario();

    void onTagSet(Room *room, const QString &key) const;
	bool exposeRoles() const;
	void assign(QStringList &generals, QStringList &roles) const;
	int getPlayerCount() const;
	QString getRoles() const;
	bool generalSelection() const;
};

#endif

