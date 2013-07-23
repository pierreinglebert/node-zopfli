var Zopfli = require('./lib/zopfli');

exports.createGzip = function(options) {
  return new Zopfli('gzip', options);
};

exports.createZlib = function(options) {
  return new Zopfli('zlib', options);
};

exports.createDeflate = function(options) {
  return new Zopfli('deflate', options);
};

exports.gzip = function(buffer, options, callback) {
  Zopfli.gzip(buffer, options, callback);
};

exports.zlib = function(buffer, options, callback) {
  Zopfli.zlib(buffer, options, callback);
};

exports.deflate = function(buffer, options, callback) {
  zopfli.deflate(buffer, options, callback);
};
