REPORTER = spec
test:
	$(MAKE) lint
	@NODE_ENV=test ./node_modules/.bin/mocha -b --require blanket --reporter $(REPORTER)

lint:
	./node_modules/.bin/jshint ./lib ./test

test-cov:
	$(MAKE) test REPORTER=spec
	$(MAKE) test REPORTER=html-cov 1> coverage.html

test-coveralls:
	$(MAKE) test REPORTER=spec
	$(MAKE) test REPORTER=mocha-lcov-reporter | ./node_modules/coveralls/bin/coveralls.js --verbose
	rm -rf lib-cov

.PHONY: test