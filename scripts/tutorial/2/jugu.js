var code = `
class Jugu : public MaxCardsSkill
{

public:
    Jugu() : MaxCardsSkill("jugu")
    {
    }

    int getExtra(const Player *target) const
    {
        if (target->hasSkill(this))
            return target->getMaxHp();
        return 0;
    }
};
`

var html_jugu = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("jugu").innerHTML = html_jugu)