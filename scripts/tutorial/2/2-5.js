var code = `
int getCorrect(const Player *from, const Player *to) const
{
    if (to->hasSkill(this))
        return 1;
}
`

var html5 = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("2-5").innerHTML = html5)