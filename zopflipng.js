var fs = require('fs');

var zopfli = require("./build/Release/zopflipng.node");
console.log( zopfli.hello() );

/*var Buffer = require('buffer').Buffer;
var zopfli = require('zopfli');
var input = new Buffer('lorem ipsum dolor sit amet');
var compressed = zopfli.deflate(input);
var output = zopfli.inflate(compressed);
*/
