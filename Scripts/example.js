const DOCPATH = "_NewAPIDocTemp/documentation.html";

console.clear();
console.log("Hello world! Type \".doc\" to view API documentation.");

console.listen(function(command) {
    if(command == ".doc") {
        console.log("Opening " + DOCPATH + " ...");
        exec("start " + DOCPATH);
    }
    else {
        console.log(eval.call(global, command));
    }
});

events.ondraw(function(e) {
    var ctx = e.drawingContext;
    ctx.fontFamily = "arial";
    ctx.drawtext(10, 100, "Hello world!\n\n" +
        "Type \".doc\" in the script console\n" +
        "to view API documentation");
});
