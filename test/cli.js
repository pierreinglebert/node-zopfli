/* jshint mocha: true */

'use strict';

var chai = require('chai');
var fs = require('fs');
var path = require('path');
var exec = require('child_process').exec;
var assert = chai.assert;


describe('zopfli cli', function() {
  it('should fail without arguments', function(done) {
    exec(
      'node ' + path.join(__dirname, '../bin/zopfli'),
      function(error, stdout, stderr) {
        assert.isNotNull(error);
        done();
      }
    );
  });

  it('should compress files', function(done) {
    var fixture = path.join(__dirname, 'fixtures/test.css');
    exec(
      'node ' + path.join(__dirname, '../bin/zopfli') + ' ' + fixture,
      function(error, stdout, stderr) {
        assert.equal(error, null);
        assert.equal(stderr, '');
        done();
      }
    );
  });
});
