// Copyright (c) 2013, Mark Cavage. All rights reserved.

var assert = require('assert');

var pathconf = require('../lib');

assert.ok(pathconf);
assert.equal(typeof (pathconf), 'function');

pathconf('/tmp', pathconf._PC_NAME_MAX, function (err, value) {
    assert.ifError(err);
    assert.ok(value);
});
