/* jshint mocha: true */

'use strict';

var chai = require('chai');
var fs = require('fs');
var zlib = require('zlib');
var async = require('async');
var path = require('path');
var zopfli = require('../lib/zopfli');
var stream = require('stream');
var util = require('util');

var assert = chai.assert;

var fixturesPath = path.join(__dirname, 'fixtures');

function MemoryStream() {
  stream.Writable.call(this);
  this.data = new Buffer(0);
}
util.inherits(MemoryStream, stream.Writable);
MemoryStream.prototype._write = function(chunk, encoding, done) {
  this.data = Buffer.concat([this.data, chunk]);
  done();
};

var testStream = function(deflate, inflate, done) {
  var files = fs.readdirSync('test/fixtures');
  async.eachSeries(files, function(file, next) {
    var writeStream = new MemoryStream();
    writeStream.on('finish', function() {
      assert.equal(
        fs.readFileSync(path.join(fixturesPath, file)).toString(),
        writeStream.data.toString()
      );
      next();
    });
    fs.createReadStream(path.join(fixturesPath, file))
      .pipe(deflate())
      .pipe(inflate())
      .pipe(writeStream);
  },
  function(err) {
    if (err) done(err);
    done();
  });
};

describe('Zopfli stream', function() {
  before(function(done) {
    var randStr = require('randomstring').generate(1 * 1024 * 1024);
    fs.writeFile(path.join(fixturesPath, 'big'), randStr, done);
  });
  after(function(done) {
    fs.unlink(path.join(fixturesPath, 'big'), done);
  });
  describe('deflate', function() {
    this.timeout(30000);    // let time to deflate a big file
    it('should deflate using stream by node zlib', function(done) {
      testStream(zopfli.createDeflate, zlib.createInflateRaw, done);
    });
  });
  describe('zlib', function() {
    this.timeout(30000);    // let time to deflate a big file
    it('should inflate using stream by node zlib', function(done) {
      testStream(zopfli.createZlib, zlib.createInflate, done);
    });
  });
  describe('gzip', function() {
    this.timeout(30000);    // let time to deflate a big file
    it('should inflate using stream by node gzip', function(done) {
      testStream(zopfli.createGzip, zlib.createGunzip, done);
    });
  });
});
