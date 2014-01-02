var chai = require("chai");
var fs = require("fs");
var zlib = require('zlib');
var async = require('async');
var zopfli = require('../lib/zopfli');

var expect = chai.expect;
var assert = chai.assert;

var path = require("path");

//just a tool function to rm dir recursively
var rmdir = function(dir) {
  var list = fs.readdirSync(dir);
  for(var i = 0; i < list.length; i++) {
    var filename = path.join(dir, list[i]);
    var stat = fs.statSync(filename);
    if(filename == "." || filename == "..") {
    } else if(stat.isDirectory()) {
      rmdir(filename);
    } else {
      fs.unlinkSync(filename);
    }
  }
  fs.rmdirSync(dir);
};

var testStream = function(deflate, inflate, done) {
  var files = fs.readdirSync('test/fixtures');
  async.eachSeries(files, function(file, next) {
    var writeStream = fs.createWriteStream('tmp/'  + file);
    writeStream.on('error', function(err) {
      next(err);
    });
    writeStream.on('close', function() {
      assert.equal(fs.readFileSync('test/fixtures/' + file).toString(), fs.readFileSync('tmp/'  + file).toString());
      next();
    });
    fs.createReadStream('test/fixtures/' + file)
      .pipe(deflate())
      .pipe(inflate())
      .pipe(writeStream);
  },
  function(err){
    if(err) done(err);
    done();
  });
};

describe('Zopfli stream',function() {
  before(function(done) {
    if(fs.existsSync('tmp')) rmdir('tmp');
    fs.mkdirSync('tmp');
    done();
  });
  after(function(done) {
    rmdir('tmp');
    done();
  });
  describe('deflate',function() {
    it('should deflate using stream by node zlib', function(done){
      testStream(zopfli.createDeflate, zlib.createInflateRaw, done);
    });
  });
  describe('zlib',function() {
    it('should inflate using stream by node zlib', function(done){
      testStream(zopfli.createZlib, zlib.createInflate, done);
    });
  });
  describe('gzip',function() {
    it('should inflate using stream by node gzip', function(done){
      testStream(zopfli.createGzip, zlib.createGunzip, done);
    });
  });
});
