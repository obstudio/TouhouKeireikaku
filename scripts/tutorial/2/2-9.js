var code = `
class Yingzi : public MaxCardsSkill
{

public:
    Yingzi() : MaxCardsSkill("yingzi")
    {
    }

    int getFixed(const Player *target) const
    {
        if (target->hasSkill(this))
            return target->getMaxHp();
    }
};
`

var html9 = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("2-9").innerHTML = html9)