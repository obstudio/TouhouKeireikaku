var code = `
int getFixed(const Player *target) const
{
    if (target->hasSkill(this))
        return target->getMaxHp();
    return -1;
}
`

var html10 = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("2-10").innerHTML = html10)