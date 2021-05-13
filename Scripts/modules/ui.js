module.exports = {
    ScreenCtrlPane: ScreenCtrlPane,
    CheckboxCtrl: CheckboxCtrl
};

function ScreenCtrlPane() {
    this.controls = [];
}

ScreenCtrlPane.prototype.add = function(control) {
    this.controls.push(control);
}

ScreenCtrlPane.prototype.draw = function(ctx) {
    this.controls.forEach(function(control) {
        control.draw(ctx);
    })
}

ScreenCtrlPane.prototype.click = function(x, y) {
    var hitCtrls = this.hitTest(x, y);
    hitCtrls.forEach(function(ctrl) {
        ctrl.click();
    });
    return hitCtrls.length > 0;
}

ScreenCtrlPane.prototype.hitTest = function(x, y) {
    var hitCtrls = [];
    for(var i in this.controls) {
        var ctrl = this.controls[i];
        if(x >= ctrl.x && x < ctrl.x + ctrl.width &&
           y >= ctrl.y && y < ctrl.y + ctrl.height)
        {
            hitCtrls.push(ctrl);
        }
    }
    return hitCtrls;
}

function CheckboxCtrl(x, y, label)
{
    this.x = x;
    this.y = y;
    this.width = 100;
    this.height = 14;

    this.label = label;
    this.checked = false;
    var _this = this;

    this.click = function() {
        _this.checked = !_this.checked;
    }
}

CheckboxCtrl.prototype.draw = function(ctx)
{
    ctx.fillColor = 0x00000010;
    ctx.fillrect(this.x, this.y, this.width, this.height);

    ctx.fillColor = this.checked ? COLOR_BLACK : 0x00000080;
    ctx.fillrect(this.x, this.y, 14, 14);
    ctx.fillColor = this.checked ? COLOR_GREEN : 0x80808080;
    ctx.fillrect(this.x+2, this.y+2, 10, 10);
    ctx.strokeColor = this.checked ? COLOR_BLACK : 0x00000080
    ctx.fillColor = this.checked ? COLOR_WHITE : 0xFFFFFF80;
    ctx.drawtext(this.x + 18, this.y - 2, this.label);
}
