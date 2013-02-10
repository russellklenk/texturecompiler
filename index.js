/*/////////////////////////////////////////////////////////////////////////////
/// @summary Exposes the exports from the texture_compiler.node module to avoid
/// having external code deal with paths to the compiled module.
///////////////////////////////////////////////////////////////////////////80*/
var Filesystem = require('fs');
var Path       = require('path');

// locate the binary texture_compiler.node for the current platform.
var v8   = 'v8-'+ /[0-9]+\.[0-9]+/.exec(process.versions.v8)[0];
var arch = process.arch;
var plat = process.platform;
var path = Path.join(__dirname, 'bin', plat+'-'+arch+'-'+v8, 'texture_compiler');
var file = path+'.node';
try
{
    Filesystem.statSync(file);
}
catch (ex)
{
    throw new Error('`'+file+'` is missing. Try reinstalling `texturecompiler`?');
}
module.exports = require(path);
