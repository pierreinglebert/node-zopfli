var fs = require('fs');

var zlib = require('zlib');

var zopfli = require('bindings')('zopfli.node');

var util = require('util');
var Transform = require('stream').Transform;

module.exports = Zopfli;

function Zopfli(){
  
}

function Zopfli(opts) {
  this.first = true;
  this.adler = 2583218086;
  this.options = opts || {};
  this.options.format = this.options.format || "deflate";
  Transform.prototype.constructor.apply(this, arguments);
}

util.inherits(Zopfli, Transform);

var stream = require('stream');
var liner = new Zopfli();

Zopfli.prototype._transform = function (chunk, encoding, done) {
  if(this.first) {
    //Write output header for zlib
    if(this.options.format == "zlib") {
      //Pre
      var cmf = 120;  /* CM 8, CINFO 7. See zlib spec.*/
      var flevel = 0;
      var fdict = 0;
      var cmfflg = 256 * cmf + fdict * 32 + flevel * 64;
      var fcheck = 31 - cmfflg % 31;
      cmfflg += fcheck;
      var startBuffer = new Buffer(8);
      startBuffer.writeUInt8(parseInt(cmfflg / 256), 0);
      startBuffer.writeUInt8(cmfflg % 256, 4);
      this.push(startBuffer);
    }
    this.first = false;
  }
  this.adler = zopfli.adler32(this.adler, chunk);
  console.log('-> ' + this.adler);
  var transform = this;
  zopfli.deflate(new Buffer(chunk), "deflate", {numiterations: 15, verbose:true, verbose_more:true}, function(err, outbuf) {
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
}
 
Zopfli.prototype._flush = function (done) {
  if(this.options.format == "zlib") {
    var endBuffer = new Buffer(4);
    console.log(this.adler);
    endBuffer.writeUInt8(((this.adler >> 24) % 256), 0);
    endBuffer.writeUInt8(((this.adler >> 16) % 256), 1);
    endBuffer.writeUInt8(((this.adler >> 8) % 256), 2);
    endBuffer.writeUInt8((this.adler % 256), 3);
    this.push(endBuffer);
  }
  done()
}
var myint = 4294967295;
myint = zopfli.testuint(myint);
myint = zopfli.testuint(myint);

/*
var source = fs.createReadStream('/home/pierre/testing');

var destStream = fs.createWriteStream('/home/pierre/testing.out');

var parser = new Zopfli({format: "zlib"});

//source.pipe(parser).pipe(destStream);

fs.readFile('/home/pierre/testing.out', function (err, data) {
  if (err) throw err;
  zlib.inflate(data, function(err, out) {
    if (err) throw err;
    console.log(out);
  });
});*/
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
