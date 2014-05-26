var chai = require("chai");
var fs = require("fs");
var zlib = require('zlib');
var async = require('async');
var zopfli = require('../lib/zopfli');

var expect = chai.expect;
var assert = chai.assert;

/*
GetOptionIfExists(options, NanNew<String>("verbose"), &zopfli_options.verbose);
// Whether to print more detailed output
GetOptionIfExists(options, NanNew<String>("verbose_more"), &zopfli_options.verbose_more);

Maximum amount of times to rerun forward and backward pass to optimize LZ77
compression cost. Good values: 10, 15 for small files, 5 for files over
several MB in size or it will be too slow.

GetOptionIfExists(options, NanNew<String>("numiterations"), &zopfli_options.numiterations);

If true, chooses the optimal block split points only after doing the iterative
LZ77 compression. If false, chooses the block split points first, then does
iterative LZ77 on each individual block. Depending on the file, either first
or last gives the best compression. Default: false (0).

GetOptionIfExists(options, NanNew<String>("blocksplitting"), &zopfli_options.blocksplitting);

If true, chooses the optimal block split points only after doing the iterative
LZ77 compression. If false, chooses the block split points first, then does
iterative LZ77 on each individual block. Depending on the file, either first
or last gives the best compression. Default: false (0).

GetOptionIfExists(options, NanNew<String>("blocksplittinglast"), &zopfli_options.blocksplittinglast);

Maximum amount of blocks to split into (0 for unlimited, but this can give
extreme results that hurt compression on some files). Default value: 15.

GetOptionIfExists(options, NanNew<String>("blocksplittingmax"), &zopfli_options.blocksplittingmax);
*/

var buf = new Buffer("test");

describe('Zopfli options',function() {
  describe('numiterations',function() {
    it('should throw an exception if something else than a number is used', function(done) {
      zopfli.deflate(buf, {
        numiterations: '1'
      }, function(err) {
        assert.instanceOf(err, TypeError);
        done();
      });
    });
    it('should throw an exception if a negative number is used', function(done) {
      zopfli.deflate(buf, {
        numiterations: -1
      }, function(err) {
        assert.instanceOf(err, TypeError);
        done();
      });
    });
  });
  describe('blocksplitting',function() {
    it('should throw an exception if something else than a number is used', function(done) {
      zopfli.deflate(buf, {
        blocksplitting: '1'
      }, function(err) {
        assert.instanceOf(err, TypeError);
        done();
      });
    });
  });
  describe('blocksplittinglast',function() {
    it('should throw an exception if something else than a number is used', function(done) {
      zopfli.deflate(buf, {
        blocksplittinglast: '1'
      }, function(err) {
        assert.instanceOf(err, TypeError);
        done();
      });
    });
  });
  describe('blocksplittingmax',function() {
    it('should throw an exception if something else than a number is used', function(done) {
      zopfli.deflate(buf, {
        blocksplittingmax: '1'
      }, function(err) {
        assert.instanceOf(err, TypeError);
        done();
      });
    });
  });
});
