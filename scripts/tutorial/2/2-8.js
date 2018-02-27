var code = `
class MaxCardsSkill : public Skill
{
    Q_OBJECT

public:
    MaxCardsSkill(const QString &name);

    virtual int getExtra(const Player *target) const;
    virtual int getFixed(const Player *target) const;
};
`

var html8 = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("2-8").innerHTML = html8)