var fs = require('fs');

var zopfli = require("./build/Debug/zopflipng.node");
console.log( zopfli.compress(false, false, [], true, ["iTxt", "iMet"], true, 5, 15, 3) );


/*var Buffer = require('buffer').Buffer;
var zopfli = require('zopfli');
var input = new Buffer('lorem ipsum dolor sit amet');
var compressed = zopfli.deflate(input);
var output = zopfli.inflate(compressed);
*/
