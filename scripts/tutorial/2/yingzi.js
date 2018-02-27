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
        return -1;
    }
};
`

var html_yingzi = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("yingzi").innerHTML = html_yingzi)