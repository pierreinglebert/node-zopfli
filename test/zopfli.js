var chai = require("chai");
var mocha = require("mocha");
var fs = require("fs");
var zlib = require('zlib');
var async = require('async');
var zopfli = require('../zopfli');

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

describe('Zopfli',function() {
  if(fs.existsSync('tmp')) rmdir('tmp');
  fs.mkdirSync('tmp');
  after(function(done) {
    rmdir('tmp');
    done();
  });
  describe('deflate',function() {

  });
  describe('zlib',function() {
    it('could be inflate by node zlib', function(done){
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
          .pipe(new zopfli.createZlib())
          .pipe(zlib.createInflate())
          .pipe(writeStream);
      },
      function(err){
        if(err) console.log(err);
        done();
      });
    });
  });
  describe('gzip',function() {
    it('could be inflate by node gzip', function(done){
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
          .pipe(new zopfli.createGzip())
          .pipe(zlib.createGunzip())
          .pipe(writeStream);
      },
      function(err){
        if(err) console.log(err);
        done();
      });
    });
  });
});


