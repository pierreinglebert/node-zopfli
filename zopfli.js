var fs = require('fs');

var zopfli = require('bindings')('zopfli.node');

exports.gzip = function(buffer, options, callback) {
  zopfli.deflate(buffer, "gzip", options, callback);
};

exports.gzipFile = function(infile, outfile, options, callback) {
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


