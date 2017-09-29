#ifndef _SPECIAL1V1_H
#define _SPECIAL1V1_H

#include "package.h"
#include "card.h"
#include "standard.h"

class Special1v1Package : public Package
{
    Q_OBJECT

public:
    Special1v1Package();
};

class New1v1CardPackage : public Package
{
    Q_OBJECT

public:
    New1v1CardPackage();
};

#endif
