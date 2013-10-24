// Copyright (c) 2013, Mark Cavage. All rights reserved.

var _pathconf = require('../build/Release/pathconf');

module.exports = function pathconf(path, name, callback) {
    return (_pathconf.pathconf(path, name, callback));
};

Object.keys(_pathconf).forEach(function (k) {
    module.exports[k] = _pathconf[k];
});
