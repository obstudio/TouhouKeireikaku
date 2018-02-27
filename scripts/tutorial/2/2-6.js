var code = `
int getCorrect(const Player *from, const Player *to) const
{
    if (to->hasSkill(this) && from != to)
        return 1;
}
`

var html6 = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("2-6").innerHTML = html6)