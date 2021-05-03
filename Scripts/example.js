const DOCPATH = "_NewAPIDocTemp/documentation.html";

console.clear();
console.log("Hello world! Type \".doc\" to view API documentation.");

console.listen(function(command) {
    if(command == ".doc") {
        console.log("Opening " + DOCPATH + " ...");
        exec("start " + DOCPATH);
    }
    else {
        console.log(eval(command));
    }
});

events.ondraw(function(e) {
    var ctx = e.drawingContext;

    ctx.fontFamily = "arial";
    ctx.fontWeight = "bold";
    ctx.fontSize = 18;
    ctx.strokeWidth = 3;
    ctx.strokeColor = COLOR_BLACK;
    ctx.fillColor = COLOR_WHITE;
    
    ctx.print(10, 100, "Hello world!");
    ctx.print(10, 120, "Type \".doc\" in the script console to view API documentation");
});
