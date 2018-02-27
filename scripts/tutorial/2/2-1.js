var code = `
class DistanceSkill : public Skill
{
    Q_OBJECT

public:
    DistanceSkill(const QString &name);

    virtual int getCorrect(const Player *from, const Player *to) const = 0;
};
`

var html = Prism.highlight(code, Prism.languages.cpp);

console.log(html);

addEventListener('load', () => document.getElementById("2-1").innerHTML = html)