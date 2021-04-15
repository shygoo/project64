if(localStorage && localStorage.getItem('theme')) {
    document.documentElement.setAttribute('data-theme', localStorage.getItem('theme'));
}
else if(window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches) {
    document.documentElement.setAttribute('data-theme', 'dark');
}

window.addEventListener('load', function() {
    if(document.documentElement.getAttribute('data-theme') == 'dark') {
        document.querySelector('#darkmode-toggle').checked = true;
    }
    
    document.querySelector('#darkmode-toggle').onchange = function(e) {
        var theme = e.target.checked ? 'dark' : 'default';
        document.documentElement.setAttribute('data-theme', theme);
        if(localStorage) {
            localStorage.setItem('theme', theme);
        }
    }
});
