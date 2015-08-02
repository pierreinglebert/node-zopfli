/* jshint mocha: true */

'use strict';

var chai = require('chai');
var fs = require('fs');
var path = require('path');
var zlib = require('zlib');
var async = require('async');
var zopfli = require('../lib/zopfli');

var assert = chai.assert;

var fixturesPath = path.join(__dirname, 'fixtures');

var testBufferSync = function(deflate, inflate, done) {
  var files = fs.readdirSync('test/fixtures');
  async.eachSeries(files, function(file, next) {
    var buffer = fs.readFileSync(path.join(fixturesPath, file));
    var result = deflate(buffer, {});
    inflate(result, function(err, result) {
      if (!err) {
        assert.equal(
          result.toString(),
          fs.readFileSync(path.join(fixturesPath, file)).toString()
        );
      }
      next(err);
    });
  },
  function(err) {
    done(err);
  });
};

describe('Zopfli buffer sync', function() {
  describe('deflate', function() {
    it('should deflate synchronously using buffer', function(done) {
      testBufferSync(zopfli.deflateSync, zlib.inflateRaw, done);
    });
  });
  describe('zlib', function() {
    it('should zlib synchronously using buffer', function(done) {
      testBufferSync(zopfli.zlibSync, zlib.inflate, done);
    });
  });
  describe('gzip', function() {
    it('should deflate synchronously using buffer', function(done) {
      testBufferSync(zopfli.gzipSync, zlib.gunzip, done);
    });
  });
});
