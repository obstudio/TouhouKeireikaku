var code = `
bool hasSkill(const QString &skill_name, bool include_lose = false) const;
bool hasSkill(const Skill *skill, bool include_lose = false) const;
`

var html4 = Prism.highlight(code, Prism.languages.cpp);

addEventListener('load', () => document.getElementById("2-4").innerHTML = html4)