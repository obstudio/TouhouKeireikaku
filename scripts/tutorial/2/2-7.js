var code = `
int getCorrect(const Player *from, const Player *to) const
{
    if (to->hasSkill(this) && from != to)
        return 1;
    return 0;
}
`

var html7 = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("2-7").innerHTML = html7)