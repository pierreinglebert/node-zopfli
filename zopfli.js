var fs = require('fs');

var zlib = require('zlib');

var zopfli = require('bindings')('zopfli.node');
var crc32 = require('buffer-crc32');
var util = require('util');
var Transform = require('stream').Transform;

module.exports = Zopfli;

function Zopfli(opts) {
  this.first = true;
  this.adler = 0x01;
  this.crc = null;
  this.options = opts || {};
  this.options.format = this.options.format || "deflate";
  Transform.prototype.constructor.apply(this, arguments);
}

util.inherits(Zopfli, Transform);

var stream = require('stream');
var liner = new Zopfli();

Zopfli.prototype._transform = function (chunk, encoding, done) {
  if(this.first) {
    this.insize = 0;
    //Write output header for zlib
    var startBuffer = Buffer(0);
    if(this.options.format == "zlib") {
      //Pre
      var cmf = 120;  /* CM 8, CINFO 7. See zlib spec.*/
      var flevel = 0;
      var fdict = 0;
      var cmfflg = 256 * cmf + fdict * 32 + flevel * 64;
      var fcheck = 31 - cmfflg % 31;
      cmfflg += fcheck;
      startBuffer = new Buffer(2);
      startBuffer.writeUInt8(parseInt(cmfflg / 256, 10), 0);
      startBuffer.writeUInt8(parseInt(cmfflg % 256, 10), 1);
    } else if(this.options.format == "gzip") {
      startBuffer = new Buffer(10);
      startBuffer.writeUInt8(31, 0);  /* ID1 */
      startBuffer.writeUInt8(139, 1); /* ID2 */
      startBuffer.writeUInt8(8, 2);   /* CM */
      startBuffer.writeUInt8(0, 3);   /* FLG */
      startBuffer.writeUInt8(0, 4);  /* MTIME */
      startBuffer.writeUInt8(2, 8);   /* XFL, 2 indicates best compression. */
      startBuffer.writeUInt8(3, 9);   /* OS follows Unix conventions. */
    }
    this.push(startBuffer);
    this.first = false;
  }

  //Update crc or adler
  if(this.options.format == "zlib") {
    this.adler = zopfli.adler32(this.adler, chunk);
  } else if(this.options.format == "gzip") {
    this.crc = crc32(chunk, this.crc);
  }
  this.insize += chunk.length;
  var transform = this;
  zopfli.deflate(new Buffer(chunk), "deflate", {numiterations: 15, verbose: false}, function(err, outbuf) {
    if (err) {
      done(err);
    } else {
      transform.push(outbuf);
      done();
      /*fs.writeFile(outfile, outbuf, function (err) {
        done(err);
      });*/
    }
  });
};

Zopfli.prototype._flush = function (done) {
  var endBuffer = new Buffer(0);
  if(this.options.format == "zlib") {
    endBuffer = new Buffer(4);
    endBuffer.writeUInt8(((this.adler >> 24) % 256), 0);
    endBuffer.writeUInt8(((this.adler >> 16) % 256), 1);
    endBuffer.writeUInt8(((this.adler >> 8) % 256), 2);
    endBuffer.writeUInt8((this.adler % 256), 3);
  } else if(this.options.format == "gzip") {
    endBuffer = new Buffer(8);
    /* CRC */
    var crcvalue = this.crc.readUInt32BE(0);
    endBuffer.writeUInt8(parseInt(crcvalue % 256, 10), 0); //Bit operations works only on unsigned int, v8 internally changes to signed...
    endBuffer.writeUInt8(parseInt(crcvalue / 256, 10) % 256, 1);
    endBuffer.writeUInt8(parseInt(crcvalue / 65536, 10) % 256, 2);
    endBuffer.writeUInt8(parseInt(crcvalue / 16777216, 10) % 256, 3);
    //this.crc.copy(endBuffer);
    /* ISIZE */
    endBuffer.writeUInt8(parseInt(this.insize % 256, 10), 4);
    endBuffer.writeUInt8((this.insize >> 8) % 256, 5);
    endBuffer.writeUInt8((this.insize >> 16) % 256, 6);
    endBuffer.writeUInt8((this.insize >> 24) % 256, 7);
  }
  this.push(endBuffer);
  done();
};

var source = fs.createReadStream('/home/pierre/testing');
var destStream = fs.createWriteStream('/home/pierre/testing.gz');
var parser = new Zopfli({format: "gzip"});

source.pipe(parser).pipe(destStream);

fs.readFile('/home/pierre/testing.out', function (err, data) {
  if (err) throw err;
  zlib.inflate(data, function(err, out) {
    if (err) throw err;
    console.log(out.toString());
  });
});
/*
var defsource = fs.createReadStream('/home/pierre/testing.out');
var defdestStream = fs.createWriteStream('/home/pierre/testing.inf');
var inf = new zlib.createInflate().on('error', function(err) { console.log(err); });
defsource.pipe(inf).pipe(defdestStream);
*/

Zopfli.zlibFile = function(infile, outfile, options, callback) {
  fs.readFile(infile, function (err, data) {
    if (err) {
      callback(err);
    } else {
      zopfli.deflate(new Buffer(data), "zlib", options, function(err, outbuf) {
        if (err) {
          callback(err);
        } else {
          fs.writeFile(outfile, outbuf, function (err) {
            callback(err);
          });
        }
      });
    }
  });
};

//Zopfli.zlibFile("/home/pierre/testing", "/home/pierre/testing.zlib", {}, function(err) { console.log(err); });

/*
zopfli.deflate(new Buffer("testaezrrrrrrrrrrrrrrrrrlllllllllllllllrrrrrring"), "zlib", {numiterations: 15}, function(err, out) {
  console.log('');
  console.log('*********************');
  console.log(out);
  console.log('*********************');
});
*/
Zopfli.gzip = function(buffer, options, callback) {
  zopfli.deflate(buffer, "gzip", options, callback);
};

Zopfli.gzipFile = function(infile, outfile, options, callback) {
  fs.readFile(infile, function (err, data) {
    if (err) {
      callback(err);
    } else {
      zopfli.deflate(new Buffer(data), "gzip", options, function(err, outbuf) {
        if (err) {
          callback(err);
        } else {
          fs.writeFile(outfile, outbuf, function (err) {
            callback(err);
          });
        }
      });
    }
  });
};
