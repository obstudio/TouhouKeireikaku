var code = `
int getCorrect(const Player *from, const Player *to) const = -1;
`

var html3 = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("2-3").innerHTML = html3)