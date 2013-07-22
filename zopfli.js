var fs = require('fs');

var zopfli = require('bindings')('zopfli.node');

/*
var util = require('util');
var Transform = require('stream').Transform;
*/

module.exports = Zopfli;

function Zopfli(){
  
}

/*
function Zopfli(opts) {
  Transform.prototype.constructor.apply(this, arguments);
}

util.inherits(Zopfli, Transform);

var stream = require('stream');
var liner = new Zopfli( { objectMode: true } );

fs.createReadStream(
  '/home/pierre/Rendu_prototype_1.png',
  {}
  )
  .addListener( "data", function(chunk) {
    console.log('data');
  }).addListener( "close",function() {
    //response.end();
    console.log('end');
});
*/

/*
liner._transform = function (chunk, encoding, done) {
     var data = chunk.toString()
     if (this._lastLineData) data = this._lastLineData + data
 
     var lines = data.split('\n')
     this._lastLineData = lines.splice(lines.length-1,1)[0]
 
     lines.forEach(this.push.bind(this))
     done()
}
 
liner._flush = function (done) {
     if (this._lastLineData) this.push(this._lastLineData)
     this._lastLineData = null
     done()
}

/*
Decoder.prototype._transform = function (chunk, write, done) {
  console.log('_transform(): (%d bytes)', chunk.length);
  var out, mh, self;
  self = this;
  mh = this.mh;

  binding.zopfli_feed(mh, chunk, chunk.length, afterFeed);

  function afterFeed (ret) {
    // XXX: a hack to ensure that "chunk" doesn't get GC'd until
    // after feed() is done. We could do this in C++-land to be "clean", but
    // doing this saves sizeof(Persistent<Object>) from the req struct.
    // It's also probably overkill...
    chunk = chunk;

    //debug('zopfli_feed() = %d', ret);
    if (!ret) {
      return done(new Error('zopfli_feed() failed: ' + ret));
    }
    read();
  }

  function read () {
    out = new Buffer(safe_buffer);
    binding.zopfli_read(mh, out, out.length, afterRead);
    // XXX: the `afterRead` function below holds the reference to the "out"
    // buffer while being filled by `mpg123_read()` on the thread pool.
  }

  function afterRead (ret, bytes, meta) {
    //debug('mpg123_read() = %d (bytes=%d) (meta=%d)', ret, bytes, meta);
    if (meta & MPG123_NEW_ID3) {
      debug('MPG123_NEW_ID3');
      binding.mpg123_id3(mh, function (ret2, id3) {
        if (ret2 == MPG123_OK) {
          self.emit('id3v' + (id3.tag ? 1 : 2), id3);
          handleRead(ret, bytes);
        } else {
          // error getting ID3 tag info (probably shouldn't happen)...
          done(new Error('mpg123_id3() failed: ' + ret2));
        }
      });
    } else {
      handleRead(ret, bytes);
    }
  }

  function handleRead (ret, bytes) {
    if (bytes > 0) {
      // got decoded data
      assert(out.length >= bytes);
      if (out.length != bytes) {
        debug('slicing output buffer from %d to %d', out.length, bytes);
        out = out.slice(0, bytes);
      }

      if (self.push) self.push(out);
      else write(out); // XXX: compat for old Transform API... remove at some point
    }
    if (ret == MPG123_DONE) {
      debug('done');
      return done();
    }
    if (ret == MPG123_NEED_MORE) {
      debug('needs more!');
      return done();
    }
    if (ret == MPG123_NEW_FORMAT) {
      var format = binding.mpg123_getformat(mh);
      debug('new format: %j', format);
      self.emit('format', format);
      return read();
    }
    if (MPG123_OK != ret) {
      return done(new Error('mpg123_read() failed: ' + ret));
    }
    read();
  }
};
*/

Zopfli.gzip = function(buffer, options, callback) {
  zopfli.deflate(buffer, "gzip", options, callback);
};

Zopfli.gzipFile = function(infile, outfile, options, callback) {
  fs.readFile(infile, function (err, data) {
    if (err) {
      callback(err);
    } else {
      zopfli.deflate(new Buffer(data), "gzip", options, function(err, outbuf) {
        if (err) {
          callback(err);
        } else {
          fs.writeFile(outfile, outbuf, function (err) {
            callback(err);
          });
        }
      });
    }
  });
};
