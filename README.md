node-zopfli
===========

Bindings for zopfli compressing lib.

[![Build Status](https://secure.travis-ci.org/pierreinglebert/node-zopfli.png)](http://travis-ci.org/pierreinglebert/node-zopfli) [![Dependency Status](https://gemnasium.com/pierreinglebert/node-zopfli.png)](https://gemnasium.com/pierreinglebert/node-zopfli)


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
### Stream (async):
    fs.createReadStream('file.js')
      .pipe(new zopfli.createGzip(options))
      .pipe(fs.createWriteStream('file.js.gz'));

### Buffer (sync):
	var input = new Buffer('i want to be compressed');
    var compressed = zopfli.deflate(input, options);

### Options 
    {
      verbose: false,
      verbose_more: false,
      numiterations: 15,
      blocksplitting: true,
      blocksplittinglast: false,
      blocksplittingmax: 15
    }

# Tests
mocha is used for tests, you can run them with :
    npm tests