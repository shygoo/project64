const yaml = require('yaml');
const fs = require('fs');

function format(text)
{
    return text.trim()
               .replace(/```([\w\W]+?)```/g, `<pre class="ex">$1</pre>`)
               .replace(/`(.+?)`/g, `<div class="snip">$1</div>`)
               .replace(/\[(.+?)\]\((https?:\/\/.+?)\)/g, `<a target="blank" href=\"$2\">$1</a>`)
               .replace(/\[(.+?)\]\((.+?)\)/g, `<a href=\"$2\">$1</a>`);
}

function idfmt(text)
{
    return text.replace(/\./g, "_");
}

var cssSource = fs.readFileSync('docsrc/style.css').toString();
var yamlSource = fs.readFileSync('docsrc/documentation.yaml').toString();

var content = '';
var modlinks = '';

var mods = yaml.parse(yamlSource);

mods.forEach(mod => {
    var propLinks = ''
    var propsContent = '';

    if(mod.props)
    mod.props.forEach(prop => {
        propLinks += `<li><a href="#${idfmt(prop.name)}">${prop.js}</a></li>\n`;

        var propTags = '';
        
        if(prop.tags)
        propTags = prop.tags.map((tag) => {
            return `<div class="tag ${tag[1]}">${tag[0]}</div>`
        }).join('');

        propsContent += (
            `<!-- ${prop.name} -->\n` +
            `<div class="prop"><span class="title" id="${idfmt(prop.name)}">${prop.js}</span>${propTags}</div>\n` +
            (prop.js2 ? `<div><span class="title2">${prop.js2}</span></div>`: '') +
            (prop.ts ? `<div class="tsproto">${prop.ts}</div>\n` : '') +
            `<div class="space"></div>\n` +
            format(prop.desc) + `\n`
        );
    });

    modlinks += `<li><a href="#${idfmt(mod.name)}">${mod.name}</a></li>\n`;
    modlinks += `<ul>\n${propLinks}</ul>\n`;

    content += (
        `<!-- ${mod.name} -->\n` +
        `<div class="module">\n` +
        `<div class="modtitle"><span class="title" id="${idfmt(mod.name)}">${mod.name}</span></div>\n` +
        (mod.desc ? `<div class="moddesc">${format(mod.desc)}</div>\n` : '') +
        `<div class="space"></div>\n` +
        `<ul>\n` +
        `${propLinks}` +
        `</ul>\n` +
        `${propsContent}\n` +
        `</div>\n`
    );
});

cssSource = cssSource.trim();
modlinks = modlinks.trim();
content = content.trim();

var html = (`<!DOCTYPE html>
<!-- Generated ${(new Date()).toDateString().match(/.+? (.+?)$/)[1]} -->
<!-- Source: https://github.com/shygoo/pj64d-docs -->
<head>
<title>Project64 JavaScript API</title>
<link href='https://fonts.googleapis.com/css?family=Open+Sans:400,400italic,700,700italic' rel='stylesheet' type='text/css'>
<style>
${cssSource}
</style>
</head>
<body>
<div class="modtitle">Project64 JavaScript API</div>
<hr>
<ul>
${modlinks}
</ul>
${content}
</body>
`);

fs.writeFileSync("documentation.html", html);
console.log("documentation.html created");
