"use strict";

module.exports = 
{
    decodeHalf: function(binary) 
    {
        var pow = Math.pow;
        var exponent = (binary & 0x7C00) >> 10;
        var fraction =  binary & 0x03FF;
        var result = (binary >> 15 ? -1 : 1) * (
            exponent ?
            (
                exponent === 0x1F ?
                fraction ? NaN : Infinity :
                pow(2, exponent - 15) * (1 + fraction / 0x400)
            ) :
            (6.103515625e-5 * (fraction / 0x400))
        );
        return result;
    },
    
    decodeLocation: function(string)
    {
        // TODO
        return string;
    }
};