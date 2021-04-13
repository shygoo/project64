const yaml = require('yaml');
const fs = require('fs');

function docfmt(text)
{
    return text.trim()
               .replace(/```([\w\W]+?)```/g, jsfmt)
               .replace(/`(.+?)`/g, `<span class="snip">$1</span>`)
               .replace(/\[(.+?)\]\((https?:\/\/.+?)\)/g, `<a target="blank" href=\"$2\">$1</a>`)
               .replace(/\[(.+?)\]\((.+?)\)/g, `<a href=\"$2\">$1</a>`);
}

function jsfmt(text)
{
    const jsKeyWords = [
        'const', 'var', 'new', 'switch', 'case', 'break', 'default',
        'if', 'else', 'try', 'catch', 'for', 'while', 'true', 'false',
        'null', 'undefined', 'function', 'this', 'return'
    ];

    text = text.replace(/```([\w\W]+?)```/, '$1');

    var formatted = "";

    for(var i = 0; i < text.length;)
    {
        var anchor = i;
        if(/[a-zA-Z]/.test(text[i])) { // word
            i++;
            while(i < text.length && /[a-zA-Z_\d]/.test(text[i])) i++;
            var word = text.slice(anchor, i);
            formatted += jsKeyWords.indexOf(word) >= 0 ? `<span class="js-keyword">${word}</span>` : `<span class="js-word">${word}</span>`;
        }
        else if(text[i] == '"' || text[i] == "'") { // str literal
            var q = text[i]
            // don't need \ for now
            i++;
            while(text[i] != q) i++;
            i++;
            formatted += `<span class="js-string">${text.slice(anchor, i)}</span>`;
        }
        else if(text[i] == '/' && text[i+1] == '/') { // line comment
            i += 2;
            while(i < text.length && text[i] != '\n') i++;
            formatted += `<span class="js-comment">${text.slice(anchor, i)}</span>`
        }
        
        else if(text[i] == '/' && text[i+1] == '*') { // block comment
            i += 2;
            while(i < text.length && !(text[i] == '*' && text[i+1] == '/')) i++;
            i += 2;
            formatted += `<span class="js-comment">${text.slice(anchor, i)}</span>`
        }
        else if(/[\d]/.test(text[i])) { // number
            i++;
            while(i < text.length && /[a-zA-Z_\d]/.test(text[i])) i++;
            formatted += `<span class="js-number">${text.slice(anchor, i)}</span>`;
        }
        else //whitespace, operators etc.
        {
            formatted += text[i++];
        }
    }

    return `<pre class="ex">${formatted}</pre>`;
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
            `<div class="vtab"></div>\n` +
            docfmt(prop.desc) + `\n`
        );
    });

    modlinks += `<li><a href="#${idfmt(mod.name)}">${mod.name}</a>${mod.tagline ? `: ${mod.tagline}` : ''}${propLinks!=''?`<ul>\n${propLinks}</ul>`:''}<div class="vtab"></div></li>\n`;

    content += (
        `<!-- ${mod.name} -->\n` +
        `<div class="module">\n` +
        `<div class="modtitle"><span class="title" id="${idfmt(mod.name)}">${mod.name}</span></div>\n` +
        (mod.desc ? `${docfmt(mod.desc)}\n` :
         mod.tagline ? `${docfmt(mod.tagline)}\n`: '') +
        `<div class="vtab"></div>\n` +
        //`<ul>\n` +
        //`${propLinks}` +
        //`</ul>\n` +
        `${propsContent}\n` +
        `</div>\n`
    );
});

cssSource = cssSource.trim();
modlinks = modlinks.trim();
content = content.trim();

var html = (`<!DOCTYPE html>
<!-- Generated ${(new Date()).toDateString().match(/.+? (.+?)$/)[1]} -->
<!-- YAML source: https://github.com/shygoo/pj64d-docs -->
<head>
<title>Project64 JavaScript API</title>
<link href='https://fonts.googleapis.com/css?family=Open+Sans:400,400italic,700,700italic' rel='stylesheet' type='text/css'>
<style>
${cssSource}
</style>
</head>
<body>
<div class="sidebar">
<div class="sidebar-content">
<div class="pagetitle">Project64 JavaScript API</div>
<hr>
<ul>
${modlinks}
</ul>
</div>
</div>
<div class="content">
${content}
</div>
</body>
`);

fs.writeFileSync("documentation.html", html);
console.log("documentation.html created");
