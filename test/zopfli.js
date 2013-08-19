var chai = require("chai");
var mocha = require("mocha");
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

describe('Zopfli buffer sync',function() {
  describe('deflate',function() {
    it('could be deflate synchronously using buffer by node zlib', function(done){
      var files = fs.readdirSync('test/fixtures');
      async.eachSeries(files, function(file, next) {
        var buffer = fs.readFileSync('test/fixtures/' + file);
        var result = zopfli.deflateSync(buffer, {});
        zlib.inflateRaw(result, function(err, result) {
          if(!err) {
            assert.equal(result.toString(), fs.readFileSync('test/fixtures/' + file).toString());
          }
          next(err);
        });
      },
      function(err){
        assert.isNull(err, 'there was no error');
        done();
      });
    });
  });
  describe('zlib',function() {
    it('could be zlib synchronously using buffer by node zlib', function(done){
      var files = fs.readdirSync('test/fixtures');
      async.eachSeries(files, function(file, next) {
        var buffer = fs.readFileSync('test/fixtures/' + file);
        var result = zopfli.zlibSync(buffer, {});
        zlib.inflate(result, function(err, result) {
          if(!err) {
            assert.equal(result.toString(), fs.readFileSync('test/fixtures/' + file).toString());
          }
          next(err);
        });
      },
      function(err){
        assert.isNull(err, 'there was no error');
        done();
      });
    });
  });
  describe('gzip',function() {
    it('could be deflate synchronously using buffer by node zlib', function(done){
      var files = fs.readdirSync('test/fixtures');
      async.eachSeries(files, function(file, next) {
        var buffer = fs.readFileSync('test/fixtures/' + file);
        var result = zopfli.gzipSync(buffer, {});
        zlib.gunzip(result, function(err, result) {
          if(!err) {
            assert.equal(result.toString(), fs.readFileSync('test/fixtures/' + file).toString());
          }
          next(err);
        });
      },
      function(err){
        assert.isNull(err, 'there was no error');
        done();
      });
    });
  });
});

describe('Zopfli buffer async',function() {
  describe('deflate',function() {
    it('could be deflate using buffer by node zlib', function(done){
      var files = fs.readdirSync('test/fixtures');
      async.eachSeries(files, function(file, next) {
        var buffer = fs.readFileSync('test/fixtures/' + file);
        zopfli.deflate(buffer, {}, function(err, result) {
          if(err) {
            next(err);
          } else {
            zlib.inflateRaw(result, function(err, result) {
              if(!err) {
                assert.equal(result.toString(), fs.readFileSync('test/fixtures/' + file).toString());
              }
              next(err);
            });
          }
        });
      },
      function(err){
        assert.isNull(err, 'there was no error');
        done();
      });
    });
  });
  describe('zlib',function() {
    it('could be zlib using buffer by node zlib', function(done){
      var files = fs.readdirSync('test/fixtures');
      async.eachSeries(files, function(file, next) {
        var buffer = fs.readFileSync('test/fixtures/' + file);
        zopfli.zlib(buffer, {}, function(err, result) {
          if(err) {
            next(err);
          } else {
            zlib.inflate(result, function(err, result) {
              if(!err) {
                assert.equal(result.toString(), fs.readFileSync('test/fixtures/' + file).toString());
              }
              next(err);
            });
          }
        });
      },
      function(err){
        assert.isNull(err, 'there was no error');
        done();
      });
    });
  });
  describe('gzip',function() {
    it('could be gzip using buffer by node zlib', function(done){
      var files = fs.readdirSync('test/fixtures');
      async.eachSeries(files, function(file, next) {
        var buffer = fs.readFileSync('test/fixtures/' + file);
        zopfli.gzip(buffer, {}, function(err, result) {
          if(err) {
            next(err);
          } else {
            zlib.gunzip(result, function(err, result) {
              if(!err) {
                assert.equal(result.toString(), fs.readFileSync('test/fixtures/' + file).toString());
              }
              next(err);
            });
          }
        });
      },
      function(err){
        assert.isNull(err, 'there was no error');
        done();
      });
    });
  });
});

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
    it('could be deflate using stream by node zlib', function(done){
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
          .pipe(zopfli.createDeflate())
          .pipe(zlib.createInflateRaw())
          .pipe(writeStream);
      },
      function(err){
        if(err) console.log(err);
        done();
      });
    });
  });
  describe('zlib',function() {
    it('could be inflate using stream by node zlib', function(done){
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
          .pipe(zopfli.createZlib())
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
    it('could be inflate using stream by node gzip', function(done){
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
          .pipe(zopfli.createGzip())
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


