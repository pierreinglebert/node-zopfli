var fs = require('fs');
var zopfli = require('bindings')('zopfli.node');
var crc32 = require('buffer-crc32');
var util = require('util');
var Transform = require('stream').Transform;
var _ = require('underscore');

/* Streaming part */
var defaultOptions = {
  verbose: false,
  verbose_more: false,
  numiterations: 15,
  blocksplitting: true,
  blocksplittinglast: false,
  blocksplittingmax: 15
};

function Zopfli(format, opts) {
  this.first = true;
  this.adler = 0x01;
  this.crc = null;
  this.format = format || "deflate";
  opts = opts || {};
  this.options = _.extend(defaultOptions, opts);
  Transform.prototype.constructor.apply(this, arguments);
}

util.inherits(Zopfli, Transform);

Zopfli.prototype._transform = function (chunk, encoding, done) {
  if(this.first) {
    this.insize = 0;
    var startBuffer = Buffer(0);
    if(this.format == "zlib") {
      //Write header for zlib
      var cmf = 120;  /* CM 8, CINFO 7. See zlib spec.*/
      var flevel = 0;
      var fdict = 0;
      var cmfflg = 256 * cmf + fdict * 32 + flevel * 64;
      var fcheck = 31 - cmfflg % 31;
      cmfflg += fcheck;
      startBuffer = new Buffer(2);
      startBuffer.writeUInt8(parseInt(cmfflg / 256, 10), 0);
      startBuffer.writeUInt8(parseInt(cmfflg % 256, 10), 1);
    } else if(this.format == "gzip") {
      //Write header for gzip
      startBuffer = new Buffer(10);
      startBuffer.writeUInt8(31, 0);  /* ID1 */
      startBuffer.writeUInt8(139, 1); /* ID2 */
      startBuffer.writeUInt8(8, 2);   /* CM */
      startBuffer.writeUInt8(0, 3);   /* FLG */
      startBuffer.writeUInt8(0, 4);   /* MTIME */
      startBuffer.writeUInt8(2, 8);   /* XFL, 2 indicates best compression. */
      startBuffer.writeUInt8(3, 9);   /* OS follows Unix conventions. */
    }
    this.push(startBuffer);
    this.first = false;
  }

  //Update crc or adler
  if(this.format == "zlib") {
    this.adler = zopfli.adler32(this.adler, chunk);
  } else if(this.format == "gzip") {
    this.crc = crc32(chunk, this.crc);
  }

  //Deflate chunk
  this.insize += chunk.length;
  var transform = this;
  zopfli.deflate(new Buffer(chunk), 'deflate', this.options, function(err, outbuf) {
    if (err) {
      done(err);
    } else {
      transform.push(outbuf);
      done();
    }
  });
};

Zopfli.prototype._flush = function (done) {
  var endBuffer = new Buffer(0);
  if(this.format == "zlib") {
    //Don't use bit operations cause v8 internally changes to signed and it messes all up
    endBuffer = new Buffer(4);
    endBuffer.writeUInt8((parseInt(this.adler / 16777216, 10) % 256), 0);
    endBuffer.writeUInt8((parseInt(this.adler / 65536, 10) % 256), 1);
    endBuffer.writeUInt8((parseInt(this.adler / 256, 10) % 256), 2);
    endBuffer.writeUInt8((this.adler % 256), 3);
  } else if(this.format == "gzip") {
    endBuffer = new Buffer(8);
    /* CRC */
    var crcvalue = this.crc.readUInt32BE(0);
    endBuffer.writeUInt8(parseInt(crcvalue % 256, 10), 0);
    endBuffer.writeUInt8(parseInt(crcvalue / 256, 10) % 256, 1);
    endBuffer.writeUInt8(parseInt(crcvalue / 65536, 10) % 256, 2);
    endBuffer.writeUInt8(parseInt(crcvalue / 16777216, 10) % 256, 3);
    /* ISIZE */
    endBuffer.writeUInt8(parseInt(this.insize % 256, 10), 4);
    endBuffer.writeUInt8(parseInt(this.insize / 256, 10) % 256, 5);
    endBuffer.writeUInt8(parseInt(this.insize / 65536, 10) % 256, 6);
    endBuffer.writeUInt8(parseInt(this.insize / 16777216, 10) % 256, 7);
  }
  this.push(endBuffer);
  done();
};

/* Stream */
Zopfli.createGzip = function(options) {
  return new Zopfli('gzip', options);
};

Zopfli.createZlib = function(options) {
  return new Zopfli('zlib', options);
};

Zopfli.createDeflate = function(options) {
  return new Zopfli('deflate', options);
};

/* Buffer */
Zopfli.gzip = function(buffer, options, callback) {
  if(typeof callback == "undefined" && typeof options == "function")
    callback = options;
  zopfli.deflate(buffer, "gzip", options, callback);
};

Zopfli.zlib = function(buffer, options, callback) {
  if(typeof callback == "undefined" && typeof options == "function")
    callback = options;
  zopfli.deflate(buffer, "zlib", options, callback);
};

Zopfli.deflate = function(buffer, options, callback) {
  if(typeof callback == "undefined" && typeof options == "function")
    callback = options;
  zopfli.deflate(buffer, "deflate", options, callback);
};

/* Sync buffer */
Zopfli.gzipSync = function(buffer, options) {
  return zopfli.deflateSync(buffer, "gzip", options);
};

Zopfli.zlibSync = function(buffer, options) {
  return zopfli.deflateSync(buffer, "zlib", options);
};

Zopfli.deflateSync = function(buffer, options) {
  return zopfli.deflateSync(buffer, "deflate", options);
};

module.exports = Zopfli;
