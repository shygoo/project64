function GfxCommand(w0, w1)
{
    this.w0 = w0;
    this.w1 = w1;
}

GfxCommand.prototype.commandByte = function()
{
    return this.w0 >>> 24;
}
