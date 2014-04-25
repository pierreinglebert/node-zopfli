./node_modules/node-pre-gyp/bin/node-pre-gyp clean
npm install --build-from-source
npm test
./node_modules/node-pre-gyp/bin/node-pre-gyp unpublish
./node_modules/node-pre-gyp/bin/node-pre-gyp package publish
./node_modules/node-pre-gyp/bin/node-pre-gyp clean