#!/usr/bin/env node
/*/////////////////////////////////////////////////////////////////////////////
/// @summary Executes node-gyp to build the texture_compiler.node module, and
/// then copies the module to the correct directory for the build platform.
/// Based on the build.js file from the node-fibers project.
///////////////////////////////////////////////////////////////////////////80*/
var ChildProcess = require('child_process');
var Filesystem   = require('fs');
var Path         = require('path');

// parse command-line arguments.
var force = false;             // true to force a rebuild
var arch  = process.arch;      // current architecture
var plat  = process.platform;  // current platform name
var v8    = /[0-9]+\.[0-9]+/.exec(process.versions.v8)[0];
var path  = plat+'-'+arch+'-v8-'+v8;
var args  = process.argv.slice(2).filter(function(arg)
    {
        if (arg === '-f' || arg === '--force')
        {
            force = true;
            return false; // don't include in args array
        }
        else if (arg.substring(0, 13) === '--target_arch')
        {
            // override target architecture; pass on to node-gyp.
            arch = arg.substring(14);
        }
        // pass this argument on to node-gyp.
        return true;
    });

if (!{ia32: true, x64: true}.hasOwnProperty(arch))
{
    console.error('Unsupported architecture: `'+ arch+ '`');
    process.exit(1);
}

// if the target .node file already exists, we don't need to rebuild it.
if (!force)
{
    try
    {
        Filesystem.statSync(Path.join(__dirname,'bin',path,'texture_compiler.node'));
        console.log('`'+path+'` exists; skipping build. Use -f to force.');
        return process.exit(0);
    }
    catch (error)
    {
        /* empty */
    }
}

// non-Windows systems can spawn node-gyp as 'node-gyp' but under Windows it
// seems necessary to specify the process path as 'node-gyp.cmd'.
var     gypcmd = 'node-gyp';
switch (process.platform)
{
    case 'win32':
        gypcmd = gypcmd+'.cmd';
        break;
    default:
        break;
}

// spawn the node-gyp process to execute the build; wait for exit before
// executing the post-build step to move the .node file into place.
var  build  = ChildProcess.spawn(
     gypcmd,
    ['configure', 'rebuild'].concat(args),
    {customFds: [0, 1, 2]});
build.on('exit', function(error)
    {
        if (error)
        {
            console.error('Build failed.');
            return process.exit(error);
        }
        else postBuild();
    });

// the post-build step copies the output .node file into place.
function postBuild()
{
    var target  = Path.join(__dirname, 'build', 'Release', 'texture_compiler.node');
    var install = Path.join(__dirname, 'bin',    path,     'texture_compiler.node');

    // create the output directory.
    try
    {
        Filesystem.mkdirSync(Path.join(__dirname, 'bin', path));
    }
    catch (error)
    {
        /* empty */
    }

    // make sure the build output file exists.
    try
    {
        Filesystem.statSync(target);
    }
    catch (error)
    {
        console.error('Build succeeded but target not found.');
        process.exit(1);
    }
    // move the build output file to the correct location.
    Filesystem.renameSync(target, install);
    console.log('Installed in `'+ install+ '`');
}
