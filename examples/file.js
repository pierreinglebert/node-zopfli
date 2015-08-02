'use strict';

var fs = require('fs');
var zopfli = require('../zopfli.js');

fs.createReadStream('file.js')
  .pipe(new zopfli.createGzip())
  .pipe(fs.createWriteStream('file.js.gz'));
