#!/usr/bin/env node
/* eslint-disable strict */

const spawn = require('cross-spawn');
const npmRunPath = require('npm-run-path-compat');

if (!process.env.CI) process.exit();

// prebuild-ci builds all 57 abi versions including old electron stuff
// that doesnt support n-api so we build specifically for node 8
if (process.version.startsWith('v8.')) {
  const ps = spawn('prebuild', [
    '-r', 'node',
    '-t', '8.16.0',
    '-u', process.env.PREBUILD_TOKEN,
    '--verbose'
  ], {
    env: npmRunPath.env()
  });
  ps.stdout.pipe(process.stdout);
  ps.stderr.pipe(process.stderr);
  ps.on('exit', function(code) {
    process.exit(code);
  });
} else {
  const ps = spawn('prebuild-ci', [], {
    env: npmRunPath.env()
  });
  ps.stdout.pipe(process.stdout);
  ps.stderr.pipe(process.stderr);
  ps.on('exit', function(code) {
    process.exit(code);
  });
}
