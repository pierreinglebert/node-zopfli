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
