var code = `
class Feiying : public DistanceSkill
{

public:
    Feiying() : DistanceSkill("feiying")
    {
    }

    int getCorrect(const Player *from, const Player *to) const
    {
        if (to->hasSkill(this) && from != to)
            return 1;
        return 0;
    }
};
`

var html_feiying = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("feiying").innerHTML = html_feiying)