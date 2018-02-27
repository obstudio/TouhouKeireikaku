var code = `
class Jineng : public DistancSkill
{
    Q_OBJECT

public:
    Jineng() : DistanceSkill("jineng")
    {
    }

    int getCorrect(const Player *from, const Player *to) const
    {
        // Some skill codes...
    }
};
`

var html2 = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("2-2").innerHTML = html2)