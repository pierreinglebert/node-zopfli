node-zopfli
===========

Bindings for zopfli compressing lib.

[![Build Status](https://secure.travis-ci.org/pierreinglebert/node-zopfli.png)](http://travis-ci.org/pierreinglebert/node-zopfli) [![Dependency Status](https://gemnasium.com/pierreinglebert/node-zopfli.png)](https://gemnasium.com/pierreinglebert/node-zopfli)

# Prerequisites

Node >=0.10

# USAGE

## Install
    
    npm install node-zopfli

or if you want zopfli binary globally

    npm install -g node-zopfli

## Binary (from command line)
To gzip a file
    
    zopfli file.txt

To compress a png file
    
    zopflipng file.png out.png

## Api
### Include
    var zopfli = require("node-zopfli");

### Stream (async):
    fs.createReadStream('file.js')
      .pipe(zopfli.createGzip(options))
      .pipe(fs.createWriteStream('file.js.gz'));

You can also directly use
    new Zopfli("gzip", options);


### Buffer (async):
    var input = new Buffer('i want to be compressed');
    zopfli.deflate(input, options, function(err, deflated) {});
    zopfli.zlib(input, options, function(err, zlibed) {});
    zopfli.gzip(input, options, function(err, gziped) {});

### Buffer (sync):
    var input = new Buffer('i want to be compressed');
    var deflated = zopfli.deflateSync(input, options);
    var zlibed = zopfli.zlibSync(input, options);
    var gziped = zopfli.gzipSync(input, options);

### Options 
    {
      verbose: false,
      verbose_more: false,
      numiterations: 15,
      blocksplitting: true,
      blocksplittinglast: false,
      blocksplittingmax: 15
    }

#Build from sources

  git clone --recursive https://github.com/pierreinglebert/node-zopfli
  node-gyp configure
  node-gyp build

# Tests
mocha is used for tests, you can run them with :
    
    npm tests
