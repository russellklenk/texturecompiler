/*/////////////////////////////////////////////////////////////////////////////
/// @summary Defines the base DataCompiler class.
///////////////////////////////////////////////////////////////////////////80*/
var Filesystem = require('fs');
var Path       = require('path');
var Events     = require('events');

/// Defines the various types of IPC messages between the CompilerCache and a
/// data compiler process. This enumeration must be kept in sync with the
/// corresponding version in the content.js compiler.js file.
var IPCMessageType = {
    /// The CompilerCache is requesting the compiler name and version from the
    /// data compiler process.
    /// Data: none
    VERSION_QUERY  : 0,

    /// The data compiler process is passing compiler version information back
    /// to the requesting CompilerCache.
    /// Data: An object {
    ///     version     : Number
    /// }
    VERSION_DATA   : 1,

    /// The CompilerCache is requesting that a data compiler process a source
    /// file and generate corresponding target file(s).
    /// Data: An object {
    ///     sourcePath  : String,
    ///     targetPath  : String,
    ///     platform    : String
    /// }
    BUILD_REQUEST  : 2,

    /// The data compiler is reporting the results of a build operation back to
    /// the CompilerCache.
    /// Data: An object {
    ///     sourcePath  : String,
    ///     targetPath  : String,
    ///     platform    : String,
    ///     success     : Boolean,
    ///     errors      : Array of String (error and warning messages),
    ///     outputs     : Array of String (absolute paths of target files),
    ///     references  : Array of String (absolute paths of referenced files)
    /// }
    BUILD_RESULT   : 3
};

/// The Globals object defines all of the module exports.
var Globals        = new Events.EventEmitter();

/// Replaces the extension portion of a path string (anything after the last
/// period character.) If the path currently has no extension, one is added. If
/// the new extension is null, undefined, or an empty string, any existing
/// extension is removed.
/// @param path The path string to modify.
/// @param newExtension The new extension, without the leading period
/// character. If null, undefined, or an empty string, any existing extension
/// will be removed.
/// @return The new, modified path string.
Globals.changeExtension = function (path, newExtension)
{
    if (newExtension && newExtension.length > 0)
    {
        // Path.extname() will return the leading period.
        // remove it as a convenience for the caller.
        if (newExtension[0] === '.')
        {
            newExtension = newExtension.substring(1);
        }
    }
    if (newExtension && newExtension.length > 0)
    {
        var ls  = path.lastIndexOf(Path.sep);
        var lp  = path.indexOf('.', ls >= 0 ? ls : 0);
        if (lp >= 1) // we could be passed a 'hidden' file or folder name.
        {
            // replace existing extension.
            return path.substring(0, lp + 1) + newExtension;
        }
        else
        {
            // no extension; add one.
            return path + '.' + newExtension;
        }
    }
    else
    {
        var ls  = path.lastIndexOf(Path.sep);
        var lp  = path.indexOf('.', ls >= 0 ? ls : 0);
        if (lp >= 1) // we could be passed a 'hidden' file or folder name.
        {
            // remove existing extension.
            return path.substring(0, lp);
        }
        else return path;
    }
};

/// Synchronously copies a file from one path to another.
/// @param sourceFile The path of the source file.
/// @param targetFile The path of the target file. If this specifies an
/// existing file, the existing file is overwritten.
/// @return An error object if an error occurred; otherwise, null.
Globals.copyFile = function (sourceFile, targetFile)
{
    var fdRd  = -1;
    var fdWr  = -1;
    var error = null;
    try
    {
        var BUFFER_SIZE = 4096;
        var buffer      = new Buffer(BUFFER_SIZE);
        var num         = 1;
        var pos         = 0;
        fdRd = Filesystem.openSync(sourceFile, 'r');
        fdWr = Filesystem.openSync(targetFile, 'w');
        while (num > 0)
        {
            num    = Filesystem.readSync(fdRd, buffer, 0, BUFFER_SIZE, pos);
            Filesystem.writeSync(fdWr, buffer, 0, num);
            pos   += num;
        }
    }
    catch (err)
    {
        // save the error for when we return.
        error = err;
    }
    finally
    {
        // make sure that file handles are closed.
        if (fdRd >= 0) Filesystem.closeSync(fdRd);
        if (fdWr >= 0) Filesystem.closeSync(fdWr);
    }
    return error;
};

/// Parses the extension portion of a path string to extract the resource type
/// and properties array.
/// @param path A path string, including filename portion. This is typically
/// the sourcePath passed to the data compiler.
/// @return An object with String resourceType and Array of String properties
/// fields specifying the resource attributes extracted from the path string.
Globals.parseResourcePath = function (path)
{
    var pn  = path || '';
    var ls  = pn.lastIndexOf(Path.sep);
    var fp  = pn.indexOf('.',  ls + 1); // first '.' after last Path.sep
    var lp  = pn.lastIndexOf('.');      // last '.' after last Path.sep
    var rt  = pn.substring(lp +1);      // final extension, w/o leading '.'
    var ps  = fp === lp ? '' : pn.substring(fp + 1, lp);
    return  {
        resourceType : rt,
        properties   : ps.split('.')
    };
};

/// Synchronously determines whether a given path exists and represents a file
/// by querying the filesystem.
/// @param path The path to check.
/// @return true if @a path represents a file that exists.
Globals.isFile = function (path)
{
    try
    {
        var    stat = Filesystem.statSync(path);
        return stat.isFile();
    }
    catch (err)
    {
        // doesn't exist, can't access, etc.
        return false;
    }
};

/// Synchronously determines whether a given path exists and represents a
/// directory by querying the filesystem.
/// @param path The path to check.
/// @return true if @a path represents a directory that exists.
Globals.isDirectory = function (path)
{
    try
    {
        var    stat = Filesystem.statSync(path);
        return stat.isDirectory();
    }
    catch (err)
    {
        // doesn't exist, can't access, etc.
        return false;
    }
};

/// Indicates the start of a source file build.
/// @param args An object describing the build environment.
/// @param args.sourcePath The path of the input source file.
/// @param args.targetPath The path of the target resource, without extension.
/// @param args.platform The name of the current build target.
/// @param args.isIPC Should be true if the build was triggered via IPC.
Globals.startBuild   = function (args)
{
    return {
        success      : true,
        sourcePath   : Path.resolve(args.sourcePath || ''),
        targetPath   : Path.resolve(args.targetPath || ''),
        platform     : args.platform || '',
        ipcReply     : args.isIPC     ? true : false,
        errors       : [],
        outputs      : [],
        references   : [],
        addError     : function (info)
            {
                var errorStr = info.toString();
                this.success = false;
                this.errors.push(errorStr);
            },
        addOutput    : function (path)
            {
                var dirPath = Path.dirname(this.targetPath);
                var absPath = Path.resolve(dirPath, path);
                var outputs = this.outputs;
                for (var i  = 0, n = outputs.length; i < n; ++i)
                {
                    if (absPath  === outputs[i])
                        return absPath;
                }
                outputs.push(absPath);
                return absPath;
            },
        addReference : function (path)
            {
                var dirPath = Path.dirname(this.sourcePath);
                var absPath = Path.resolve(dirPath, path);
                var refs    = this.references;
                for (var i  = 0, n = refs.length; i < n; ++i)
                {
                    if (absPath  === refs[i])
                        return absPath;
                }
                refs.push(absPath);
                return absPath;
            }
    };
};

/// Indicates that the build associated with a particular source file is done.
/// @param args The object returned by DataCompiler.startBuild(). Set the
/// value of args.success if you wish to explicitly specify whether the build
/// was successful or not.
Globals.finishBuild  = function (args)
{
    if (args.ipcReply)
    {
        process.send({
            type : IPCMessageType.BUILD_RESULT,
            data : {
                sourcePath : args.sourcePath,
                targetPath : args.targetPath,
                platform   : args.platform,
                success    : args.success,
                errors     : args.errors,
                outputs    : args.outputs,
                references : args.references
            }
        });
        args.ipcReply = false;
    }
};

/// This function will be called to determine the current version of the data
/// compiler. Implementations should attach their own function to this field.
/// @return A Number specifying the current data compiler version.
Globals.queryCompilerVersion  = function ()
{
    return 1;
};

/// Callback invoked when the process receives an IPC VERSION_QUERY message.
Globals.handleIPCVersionQuery = function ()
{
    var func = Globals.queryCompilerVersion || function defaultVersionQuery ()
        {
            return 1;
        };
    var vers = func();
    process.send({
        type : IPCMessageType.VERSION_DATA,
        data : {
            version : vers
        }
    });
};

/// Callback invoked when the process receives an IPC BUILD_REQUEST message.
/// Emits the 'build' event, passing @a data along as-is.
/// @param data Data associated with the build request.
/// @param data.sourcePath The absolute path of the input source file.
/// @param data.targetPath The absolute path of the target resource, not
/// including the file extension (resource type).
/// @param data.platform The name of the current build target. An empty string
/// or the string 'generic' indicates a platform-agnostic build.
Globals.handleIPCBuildRequest = function (data)
{
    Globals.emit('build', data);
};

/// Callback invoked when the process receives an IPC message.
/// @param message An object specifying the message data.
/// @param message.type One of the values of the IPCMessageType enumeration.
/// @param message.data An object containing message-specific data.
Globals.handleIPCMessage = function (message)
{
    switch (message.type)
    {
        case IPCMessageType.VERSION_QUERY:
            Globals.handleIPCVersionQuery();
            break;
        case IPCMessageType.BUILD_REQUEST:
            Globals.handleIPCBuildRequest(message.data);
            break;
        default:
            break;
    }
};

/// Hook the global process IPC event.
process.on('message', Globals.handleIPCMessage);

/// Set the module exports.
module.exports = Globals;
