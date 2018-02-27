var code = `
class Nianlai : public MaxCardsSkill
{

public:
    Nianlai() : MaxCardsSkill("nianlai")
    {
    }

    int getExtra(const Player *target) const
    {
        if (to->hasSkill(this) && from->getMaxHp() > to->getMaxHp())
            return -1;
        return 0;
    }

    int getFixed(const Player *target) const
    {
        if (from->hasSkill(this) && from->getMaxHp() > to->getMaxHp())
            return 1;
        return -1;
    }
};
`

var html_nianlai = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("nianlai").innerHTML = html_nianlai)