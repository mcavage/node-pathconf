# pathconf

pathconf is a super simple module that just gives you access to statvfs(2). Read
`man pathconf` for more information.

# Usage

    var pathconf = require(pathconf);

    pathconf('/tmp', pathconf._PC_NAME_MAX, function (err, value) {
        assert.ifError(err); // on errno, will be a node ErrnoException
		console.log(value);
    });
});

# Installation

    npm install pathconf

Note this is a native add-on. You will need gcc, python, blah blah blah.

# License

MIT
