var chai = require("chai");
var path = require("path");
var fs = require("fs");
var zlib = require('zlib');
var async = require('async');
var zopfli = require('../lib/zopfli');

var expect = chai.expect;
var assert = chai.assert;

var testBufferAsync = function(deflate, inflate, done) {
  var files = fs.readdirSync('test/fixtures');
  async.eachSeries(files, function(file, next) {
    var buffer = fs.readFileSync('test/fixtures/' + file);
    deflate(buffer, {}, function(err, result) {
      if(err) {
        next(err);
      } else {
        inflate(result, function(err, result) {
          if(!err) {
            assert.equal(result.toString(), fs.readFileSync('test/fixtures/' + file).toString());
          }
          next(err);
        });
      }
    });
  },
  function(err){
    done(err);
  });
};

describe('Zopfli buffer async',function() {
  describe('deflate',function() {
    it('could be deflated using buffer by zopfli and inflated by node zlib', function(done){
      testBufferAsync(zopfli.deflate, zlib.inflateRaw, done);
    });
  });
  describe('zlib',function() {
    it('could be zlib using buffer by node zlib', function(done){
      testBufferAsync(zopfli.zlib, zlib.inflate, done);
    });
  });
  describe('gzip',function() {
    it('could be gzip using buffer by node zlib', function(done){
      testBufferAsync(zopfli.gzip, zlib.gunzip, done);
    });
  });
});
