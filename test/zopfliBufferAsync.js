/* jshint mocha: true */

'use strict';

var chai = require('chai');
var fs = require('fs');
var zlib = require('zlib');
var async = require('async');
var zopfli = require('../lib/zopfli');

var assert = chai.assert;

var testBufferAsync = function(deflate, inflate, done) {
  var files = fs.readdirSync('test/fixtures');
  async.eachSeries(files, function(file, next) {
    var buffer = fs.readFileSync('test/fixtures/' + file);
    deflate(buffer, function(err, result) {
      if (err) {
        next(err);
      } else {
        inflate(result, function(err, result) {
          if (!err) {
            assert.equal(result.toString(), fs.readFileSync('test/fixtures/' + file).toString());
          }
          next(err);
        });
      }
    });
  },
  function(err) {
    done(err);
  });
};

describe('Zopfli buffer async', function() {
  it('should return a promise if no callback is given', function(done) {
    var buffer = fs.readFileSync('test/fixtures/test.js');
    zopfli.deflate(buffer).then(function(result) {
      zlib.inflateRaw(result, function(err, result) {
        if (!err) {
          assert.equal(result.toString(), buffer.toString());
        }
        done(err);
      });
    });
  });
  describe('deflate', function() {
    it('should deflate using buffer by zopfli and inflated by node zlib', function(done) {
      testBufferAsync(zopfli.deflate, zlib.inflateRaw, done);
    });
  });
  describe('zlib', function() {
    it('should zlib using buffer by node zlib', function(done) {
      testBufferAsync(zopfli.zlib, zlib.inflate, done);
    });
  });
  describe('gzip', function() {
    it('should gzip using buffer by node zlib', function(done) {
      testBufferAsync(zopfli.gzip, zlib.gunzip, done);
    });
  });
});
