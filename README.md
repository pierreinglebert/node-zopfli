node-zopfli
===========

Bindings for zopfli compressing lib.

[![Build Status](https://secure.travis-ci.org/pierreinglebert/node-zopfli.png)](http://travis-ci.org/pierreinglebert/node-zopfli) [![Dependency Status](https://gemnasium.com/pierreinglebert/node-zopfli.png)](https://gemnasium.com/pierreinglebert/node-zopfli)


# USAGE

With Streams : 
    
    fs.createReadStream('file.js')
      .pipe(new zopfli.createGzip())
      .pipe(fs.createWriteStream('file.js.gz'));

With buffer :

	var input = new Buffer('i want to be compressed');
    var compressed = zopfli.deflate(input);