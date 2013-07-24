var chai = require("chai");
var mocha = require("mocha");
var fs = require("fs");
var zlib = require('zlib');
var async = require('async');
var zopfli = require('../zopfli');

var expect = chai.expect;
var assert = chai.assert;

var path = require("path");

var rmdir = function(dir) {
  var list = fs.readdirSync(dir);
  for(var i = 0; i < list.length; i++) {
    var filename = path.join(dir, list[i]);
    var stat = fs.statSync(filename);
    if(filename == "." || filename == "..") {
      // pass these files
    } else if(stat.isDirectory()) {
      // rmdir recursively
      rmdir(filename);
    } else {
      // rm fiilename
      fs.unlinkSync(filename);
    }
  }
  fs.rmdirSync(dir);
};

/*
var zopflipng = require("../build/Release/zopflipng");
describe('Zopflipng',function() {
  describe('Options Verification', function() {
    it('should say that lossy_transparent is wrong',function(){
      var fn = function() { zopflipng.compress({lossy_transparent: "string"}); };
      expect(fn).to.throw(TypeError, /lossy_transparent/);
      fn = function() { zopflipng.compress({lossy_transparent: 3}); };
      expect(fn).to.throw(TypeError, /lossy_transparent/);
      fn = function() { zopflipng.compress({lossy_transparent: {}}); };
      expect(fn).to.throw(TypeError, /lossy_transparent/);
    });
    it('should say that lossy_8bit is wrong',function(){
      var fn = function() { zopflipng.compress({lossy_8bit: "string"}); };
      expect(fn).to.throw(TypeError, /lossy_8bit/);
      fn = function() { zopflipng.compress({lossy_8bit: 4}); };
      expect(fn).to.throw(TypeError, /lossy_8bit/);
      fn = function() { zopflipng.compress({lossy_8bit: {}}); };
      expect(fn).to.throw(TypeError, /lossy_8bit/);
    });
    it('should say that filter_strategies is wrong',function(){
      var fn = function() { zopflipng.compress({filter_strategies: "string"}); };
      expect(fn).to.throw(TypeError, /filter_strategies/);
      fn = function() { zopflipng.compress({filter_strategies: 4}); };
      expect(fn).to.throw(TypeError, /filter_strategies/);
      fn = function() { zopflipng.compress({filter_strategies: true}); };
      expect(fn).to.throw(TypeError, /filter_strategies/);
      fn = function() { zopflipng.compress({filter_strategies: ['fakestrategy']}); };
      expect(fn).to.throw(TypeError, /Wrong strategy/);
    });
    it('should say that auto_filter_strategy is wrong',function(){
      var fn = function() { zopflipng.compress({auto_filter_strategy: "string"}); };
      expect(fn).to.throw(TypeError, /auto_filter_strategy/);
      fn = function() { zopflipng.compress({auto_filter_strategy: 4}); };
      expect(fn).to.throw(TypeError, /auto_filter_strategy/);
      fn = function() { zopflipng.compress({auto_filter_strategy: {}}); };
      expect(fn).to.throw(TypeError, /auto_filter_strategy/);
    });
    it('should say that keepchunks is wrong',function(){
      var fn = function() { zopflipng.compress({keepchunks: "string"}); };
      expect(fn).to.throw(TypeError, /keepchunks/);
      fn = function() { zopflipng.compress({keepchunks: 4}); };
      expect(fn).to.throw(TypeError, /keepchunks/);
      fn = function() { zopflipng.compress({keepchunks: {}}); };
      expect(fn).to.throw(TypeError, /keepchunks/);
    });
    it('should say that use_zopfli is wrong',function(){
      var fn = function() { zopflipng.compress({use_zopfli: "string"}); };
      expect(fn).to.throw(TypeError, /use_zopfli/);
      fn = function() { zopflipng.compress({use_zopfli: 4}); };
      expect(fn).to.throw(TypeError, /use_zopfli/);
      fn = function() { zopflipng.compress({use_zopfli: {}}); };
      expect(fn).to.throw(TypeError, /use_zopfli/);
    });
    it('should say that num_iterations is wrong',function(){
      var fn = function() { zopflipng.compress({num_iterations: "string"}); };
      expect(fn).to.throw(TypeError, /num_iterations/);
      fn = function() { zopflipng.compress({num_iterations: true}); };
      expect(fn).to.throw(TypeError, /num_iterations/);
      fn = function() { zopflipng.compress({num_iterations: {}}); };
      expect(fn).to.throw(TypeError, /num_iterations/);
    });
    it('should say that num_iterations_large is wrong',function(){
      var fn = function() { zopflipng.compress({num_iterations_large: "string"}); };
      expect(fn).to.throw(TypeError, /num_iterations_large/);
      fn = function() { zopflipng.compress({num_iterations_large: true}); };
      expect(fn).to.throw(TypeError, /num_iterations_large/);
      fn = function() { zopflipng.compress({num_iterations_large: {}}); };
      expect(fn).to.throw(TypeError, /num_iterations_large/);
    });
    it('should say that block_split_strategy is wrong',function(){
      var fn = function() { zopflipng.compress({block_split_strategy: true}); };
      expect(fn).to.throw(TypeError, /block_split_strategy/);
      fn = function() { zopflipng.compress({block_split_strategy: 4}); };
      expect(fn).to.throw(TypeError, /block_split_strategy/);
      fn = function() { zopflipng.compress({block_split_strategy: {}}); };
      expect(fn).to.throw(TypeError, /block_split_strategy/);
      fn = function() { zopflipng.compress({block_split_strategy: "string"}); };
      expect(fn).to.throw(TypeError, /Wrong value/);
    });
  });
});
*/

describe('Zopfli',function() {
  if(fs.existsSync('tmp')) rmdir('tmp');
  fs.mkdirSync('tmp');
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
  //rmdir('tmp');
});


