node-zopfli
===========

Bindings for [Zopfli](http://en.wikipedia.org/wiki/Zopfli) compressing lib.
Compress gzip files 5% better than gzip.

It is considerably slower than gzip (~100x) so you may want to use it only for static content and cached resources.

[![Build Status](https://secure.travis-ci.org/pierreinglebert/node-zopfli.png)](http://travis-ci.org/pierreinglebert/node-zopfli)
[![Coverage Status](https://coveralls.io/repos/pierreinglebert/node-zopfli/badge.png?branch=master)](https://coveralls.io/r/pierreinglebert/node-zopfli?branch=master)
[![Dependency Status](https://gemnasium.com/pierreinglebert/node-zopfli.png)](https://gemnasium.com/pierreinglebert/node-zopfli)
[![Stories in Ready](https://badge.waffle.io/pierreinglebert/node-zopfli.png)](http://waffle.io/pierreinglebert/node-zopfli)

# Prerequisites for building

* Python 2.7
* make (unix) or visual studio express (windows) see [Node Building tools](https://github.com/TooTallNate/node-gyp#installation)

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

## Usage examples
### Stream (async):

```javascript
var zopfli = require("node-zopfli");
fs.createReadStream('file.js')
  .pipe(zopfli.createGzip(options))
  .pipe(fs.createWriteStream('file.js.gz'));
```

Instead of zopfli.createGzip, you can also use

```javascript
new Zopfli("gzip", options);
```

### Buffer (async):

```javascript
var zopfli = require("node-zopfli");
var input = new Buffer('i want to be compressed');
zopfli.deflate(input, options, function(err, deflated) {});
zopfli.zlib(input, options, function(err, zlibed) {});
zopfli.gzip(input, options, function(err, gziped) {});
```

### Buffer (sync):

```javascript
var zopfli = require("node-zopfli");
var input = new Buffer('i want to be compressed');
var deflated = zopfli.deflateSync(input, options);
var zlibed = zopfli.zlibSync(input, options);
var gziped = zopfli.gzipSync(input, options);
```

## Api

### compress(input, format, [options, callback])

`input` is the input buffer

`format` can be one of `deflate`, `zlib` and `gzip`

`callback`, if present, gets two arguments `(err, buffer)` where `err` is an error object, if any, and `buffer` is the resultant compressed data.

If no callback is provided, it returns an A+ Promise.

#### aliases

`deflate`, `zlib` and `gzip` methods are aliases on `compress` without `format` argument.

### Options

Here are the options with defaults values you can pass to zopfli :

```javascript
{
  verbose: false,
  verbose_more: false,
  numiterations: 15,
  blocksplitting: true,
  blocksplittinglast: false,
  blocksplittingmax: 15
}
```

#### numiterations
Maximum amount of times to rerun forward and backward pass to optimize LZ77 compression cost. Good values: 10, 15 for small files, 5 for files over several MB in size or it will be too slow.

#### blocksplitting
If true, splits the data in multiple deflate blocks with optimal choice for the block boundaries. Block splitting gives better compression.

#### blocksplittinglast
If true, chooses the optimal block split points only after doing the iterative LZ77 compression. If false, chooses the block split points first, then does iterative LZ77 on each individual block. Depending on the file, either first or last gives the best compression.

#### blocksplittingmax
Maximum amount of blocks to split into (0 for unlimited, but this can give extreme results that hurt compression on some files).


#Build from sources

    git clone https://github.com/pierreinglebert/node-zopfli
    npm install

# Tests
mocha is used for tests, you can run them with :

    npm test
