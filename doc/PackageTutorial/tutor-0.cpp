#include "tutor.h"
#include "general.h"

TutorPackage::TutorPackage() : Package("tutor")
{
	General *mash = new General(this, "mash", "shielder", 4, false);
}

ADD_PACKAGE(Tutor)
