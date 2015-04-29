REPORTER = spec
test:
	$(MAKE) lint
	@NODE_ENV=test ./node_modules/.bin/mocha -b --reporter $(REPORTER) test

lint:
	./node_modules/.bin/jshint ./examples ./lib ./test

test-cov:
	./node_modules/.bin/istanbul cover ./node_modules/.bin/_mocha -- -R spec test

test-coveralls:
	./node_modules/.bin/istanbul cover ./node_modules/.bin/_mocha -- -R spec test
	cat ./coverage/lcov.info | ./node_modules/coveralls/bin/coveralls.js || true
	rm -rf lib-cov

clean:
	rm -rf ./lib/binding
	rm -rf ./coverage

.PHONY: test
