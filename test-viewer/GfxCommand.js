function GfxCommand(w0, w1)
{
    this.w0 = w0;
    this.w1 = w1;
}

GfxCommand.prototype.w0f = function(rsh, nbits)
{
    var val = this.w0 >>> rsh;
    var mask = (1 << nbits) - 1;
    return val & mask;
}

GfxCommand.prototype.w1f = function(rsh, nbits)
{
    var val = this.w1 >>> rsh;
    var mask = (1 << nbits) - 1;
    return val & mask;
}

GfxCommand.prototype.commandByte = function()
{
    return this.w0f(24, 8);
}