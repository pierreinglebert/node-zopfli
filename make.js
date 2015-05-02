/* jshint shelljs:true */

'use strict';

require('shelljs/make');
var path = require('path');
var os   = require('os');

var COVERALLS = './node_modules/coveralls/bin/coveralls.js';
var ISTANBUL  = path.join(__dirname, 'node_modules/.bin/istanbul');
var JSHINT    = path.join(__dirname, 'node_modules/.bin/jshint');
var MOCHA     = path.join(__dirname, 'node_modules/.bin/mocha');
var _MOCHA    = path.join(__dirname, 'node_modules/mocha/bin/_mocha');

(function() {
    cd(__dirname);

    //
    // make test
    //
    target.test = function() {
        target.lint();
        env.NODE_ENV = test;
        exec(MOCHA + ' -b --reporter spec test');
    };

    //
    // make test-cov
    //
    target['test-cov'] = function() {
        exec(ISTANBUL + ' cover ' + _MOCHA + ' -- -R spec test');
    };

    //
    // make test-coveralls
    //
    target['test-coveralls'] = function() {
        exec(ISTANBUL + ' cover ' + _MOCHA + ' -- -R spec test');
        var command = os.platform() === 'win32' ?
                     'type ' + path.normalize('./coverage/lcov.info') + ' | node ' + path.normalize(COVERALLS) :
                     'cat ./coverage/lcov.info | node ' + COVERALLS;

        try {
            exec(command);
        } catch (e) {
            echo(e);
        }

        if (test('-d', path.normalize('./lib-cov'))) {
            rm('-rf', './lib-cov');
        }
    };

    //
    // make clean
    //
    target.clean = function() {
        rm('-rf', path.normalize('./lib/binding'));
        rm('-rf', path.normalize('./lib/coverage'));
    };

    //
    // make lint
    //
    target.lint = function() {
        exec(JSHINT + ' ./examples/ ./lib/ ./test/ make.js');
    };

    //
    // make all
    //
    target.all = function() {
        target.test();
    };

    //
    // make help
    //
    target.help = function() {
        echo('Available targets:');
        echo('  test            runs the tests');
        echo('  test-cov        runs coverage');
        echo('  test-coveralls  runs coverage');
        echo('  lint            runs JSHint for the codebase');
        echo('  clean           cleanup working directory');
        echo('  help            shows this help message');
    };

}());
