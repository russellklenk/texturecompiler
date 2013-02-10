#! /usr/bin/env node
/*/////////////////////////////////////////////////////////////////////////////
/// @summary Implements the texture compiler process. The texture compiler
/// script calls out to native code to perform most of the actual image
/// processing.
///////////////////////////////////////////////////////////////////////////80*/
var Filesystem        = require('fs');
var Path              = require('path');
var Program           = require('commander');
var DataCompiler      = require('datacompiler');
var TextureCompiler   = require('../index');

/// Constants and global values used throughout the application module.
var application       = {
    /// The name of the application module.
    NAME              : 'texture',
    /// The path from which the application was started.
    STARTUP_DIRECTORY : process.cwd(),
    /// The filename of the file containing default texture build values.
    DEFAULTS_FILENAME : 'texture_defaults.json',
    /// An object defining the pre-digested command-line arguments passed to
    /// the application, not including the node or script name values.
    args              : {},
    /// The data compiler version number.
    version           : 1,
};

/// Constants representing the various application exit codes.
var exit_code         = {
    /// The program has exited successfully.
    SUCCESS           : 0,
    /// The program has exited with an unknown error.
    ERROR             : 1,
    /// The program has exited because the source file does not exist.
    FILE_NOT_FOUND    : 2
};

/// The default values for various attributes. These can be overridden by the
/// values from the texture_defaults.json file.
var texture_defaults  = {
    type              : 'COLOR',
    format            : 'RGB',
    target            : 'TEXTURE_2D',
    wrapModeS         : 'CLAMP_TO_EDGE',
    wrapModeT         : 'CLAMP_TO_EDGE',
    minifyFilter      : 'LINEAR',
    magnifyFilter     : 'LINEAR',
    borderMode        : 'CLAMP',
    premultipliedAlpha: false,
    forcePowerOfTwo   : false,
    flipY             : true,
    buildMipmaps      : false
};

/// A handy utility function that prevents having to write the same
/// obnoxious code everytime. The typical javascript '||' trick works for
/// strings, arrays and objects, but it doesn't work for booleans or
/// integer values.
/// @param value The value to test.
/// @param theDefault The value to return if @a value is undefined.
/// @return Either @a value or @a theDefault (if @a value is undefined.)
function defaultValue(value, theDefault)
{
    return (value !== undefined) ? value : theDefault;
}

/// Processes any options specified on the command line. If necessary, help
/// information is displayed and the application exits.
/// @return An object whose properties are the configuration specified by the
/// command-line arguments, with suitable defaults filled in where necessary.
function command_line()
{
    // parse the command line, display help, etc. if the command
    // line is invalid, commander will call process.exit() for us.
    Program
        .version('1.0.0')
        .option('-P, --persistent',    'Start in persistent mode.',          Boolean, false)
        .option('-i, --input [path]',  'Specify the source file.',           String, '')
        .option('-o, --output [path]', 'Specify the destination file.',      String, '')
        .option('-t, --target [name]', 'Specify the build target platform.', String, '')
        .parse(process.argv);

    var defaultsPath = Path.join(
        application.STARTUP_DIRECTORY,
        application.DEFAULTS_FILENAME);
    load_texture_defaults(defaultsPath, false);

    if (Program.persistent)
    {
        // when running in persistent mode, command-line arguments are ignored.
        return {
            persistent : true,
            sourcePath : process.argv[1],
            targetPath : process.argv[1],
            platform   : ''
        };
    }

    // when running in command-line mode, we have additional work to do.
    if (!DataCompiler.isFile(Program.input))
    {
        console.log('Error: No input file specified or input file not found.');
        console.log();
        process.exit(exit_code.FILE_NOT_FOUND);
    }
    if (!Program.output)
    {
        var file       = Path.basename(Program.input);
        var path       = process.cwd();
        file           = DataCompiler.changeExtension(file, 'pixels');
        Program.output = Path.join(path, file);
    }
    return {
        persistent : false,
        sourcePath : Program.input,
        targetPath : Program.output,
        platform   : Program.target
    };
}

/// Fills in any unspecified values in a texture attributes definition with
/// their default values.
/// @param object The texture attributes definition.
/// @param defaults An object specifying the default values.
/// @return The input argument @a object.
function build_definition(object, defaults)
{
    var obj                = object;
    var def                = defaults;
    var D                  = defaultValue;
    obj.type               = D(obj.type,               def.type);
    obj.format             = D(obj.format,             def.format);
    obj.target             = D(obj.target,             def.target);
    obj.wrapModeS          = D(obj.wrapModeS,          def.wrapModeS);
    obj.wrapModeT          = D(obj.wrapModeT,          def.wrapModeT);
    obj.minifyFilter       = D(obj.minifyFilter,       def.minifyFilter);
    obj.magnifyFilter      = D(obj.magnifyFilter,      def.magnifyFilter);
    obj.borderMode         = D(obj.borderMode,         def.borderMode);
    obj.premultipliedAlpha = D(obj.premultipliedAlpha, def.premultipliedAlpha);
    obj.forcePowerOfTwo    = D(obj.forcePowerOfTwo,    def.forcePowerOfTwo);
    obj.flipY              = D(obj.flipY,              def.flipY);
    obj.buildMipmaps       = D(obj.buildMipmaps,       def.buildMipmaps);
    return obj;
}

/// Creates a new object initialized with the default texture processing
/// attributes. This object can be used when attributes are not explicitly
/// specified for a given source file.
/// @return A new object initialized with default texture attributes.
function default_texture_attributes()
{
    var def          = build_definition({}, texture_defaults);
    def.levelCount   = (def.buildMipmaps ? 0 : 1);
    def.targetWidth  =  0;
    def.targetHeight =  0;
    return def;
}

/// Reads the texture_defaults.json file to override the default values for any
/// unspecified fields in a texture attributes definition.
/// @param filename The path and filename of the file to load.
/// @param silent Whether or not to output warning information to the console.
function load_texture_defaults(filename, silent)
{
    try
    {
        filename         = filename || application.DEFAULTS_FILENAME;
        var json         = Filesystem.readFileSync(filename, 'utf8');
        var vals         = JSON.parse(json);
        texture_defaults = build_definition(vals, texture_defaults);
    }
    catch (error)
    {
        if (!silent)
        {
            console.warn('Warning: Could not load texture defaults:');
            console.warn('  with path: '+filename);
            console.warn('  exception: '+error);
            console.warn();
        }
    }
    return texture_defaults;
}

/// Attempts to load texture processing attributes from a JSON source file. If
/// the @a sourcePath instead specifies an image file, the default attributes
/// are returned.
/// @param state The build state as returned by DataCompiler.startBuild().
/// @param sourcePath The source file path.
/// @param targetPath The path of the target output file that will hold the
/// processed pixel data.
/// @return An object representing the texture attributes associated with the
/// source file @a sourcePath.
function load_texture_attributes(state, sourcePath, targetPath)
{
    var     ext   = Path.extname(sourcePath).toLowerCase();
    switch (ext)
    {
        case '.psd':
        case '.png':
        case '.jpg':
        case '.jpeg':
        case '.tga':
        case '.bmp':
        case '.gif':
        case '.hdr':
        case '.pic':
            {
                // assume a raw input image that will use the default
                // texture attributes. errors should not caught here.
                var def          = build_definition({}, texture_defaults);
                def.sourcePath   = state.addReference(sourcePath);
                def.targetPath   = targetPath;
                def.levelCount   =(def.buildMipmaps ? 0 : 1);
                def.targetWidth  = 0;
                def.targetHeight = 0;
                return def;
            }
            /* unreachable */

        default:
            {
                // assume a JSON texture attributes definition.
                // @note: errors should not be caught here.
                var json         = Filesystem.readFileSync(sourcePath, 'utf8');
                var obj          = JSON.parse(json);
                var def          = build_definition(obj, texture_defaults);
                var lc           =(def.buildMipmaps ? 0 : 1);
                def.sourcePath   = state.addReference(def.sourcePath);
                def.targetPath   = targetPath;
                def.levelCount   = defaultValue(def.levelCount,  lc);
                def.targetWidth  = defaultValue(def.targetWidth,  0);
                def.targetHeight = defaultValue(def.targetHeight, 0);
                if (!DataCompiler.isFile(def.sourcePath || ''))
                    throw new Error('Invalid sourcePath attribute: '+def.sourcePath);
                return def;
            }
            /* unreachable */
    }
}

/// Writes a JSON document specifying texture metadata to disk.
/// @param targetPath The path and filename of the target file.
/// @param texture An object specifying texture metadata.
function save_texture_definition(targetPath, texture)
{
    // @note: errors should not be caught here.
    var json = JSON.stringify(texture, null, '\t')+'\n';
    Filesystem.writeFileSync(targetPath, json, 'utf8');
}

/// Implements the build process for the data compiler.
/// @param input An object describing the build environment.
/// @param input.sourcePath The path of the input source file.
/// @param input.targetPath The path of the target resource, without extension.
/// @param input.platform The name of the current build target.
/// @param input.isIPC Should be true if the build was triggered via IPC.
function compiler_build(input)
{
    var state  = DataCompiler.startBuild(input);
    var rinfo  = DataCompiler.parseResourcePath(input.sourcePath);
    var mpath  = DataCompiler.changeExtension(input.targetPath, 'texture');
    var apath  = input.sourcePath; // attributes path
    var ppath  = input.targetPath; // pixels path
    try
    {
        var ta = load_texture_attributes(state, apath, ppath);
        var md = TextureCompiler.compile(ta);
        save_texture_definition(mpath, md);
        state.addOutput(mpath);
        state.addOutput(ppath);
    }
    catch (error)
    {
        // add the error; build will be unsuccessful.
        state.addError(error);
        // when running stand-alone, output the error so the user knows
        // that something went wrong. in persistent mode, the error is
        // output for us by the build system.
        if (!application.args.persistent)
        {
            console.error('An error has occurred:');
            console.error('  '+error);
            console.error();
            process.exit(exit_code.ERROR);
        }
    }
    DataCompiler.finishBuild(state);
}

/// Override the default DataCompiler implementation to return the correct
/// version of our data compiler.
/// @return A Number specifying the current data compiler version.
DataCompiler.queryCompilerVersion = function ()
{
    return application.version;
};

/// Handles the DataCompiler build event, emitted when a build is triggered
/// via an IPC mechanism.
/// @param data Data associated with the build request.
/// @param data.sourcePath The absolute path of the input source file.
/// @param data.targetPath The absolute path of the target resource, not
/// including the file extension (resource type).
/// @param data.platform The name of the current build target. An empty string
/// or the string 'generic' indicates a platform-agnostic build.
DataCompiler.on('build', function (data)
{
    compiler_build({
        sourcePath : data.sourcePath,
        targetPath : data.targetPath,
        platform   : data.platform,
        isIPC      : true
    });
});

/// Catches any unhandled exceptions that occur during execution.
/// @param error An Error instance specifying additional information.
process.on('unhandledException', function (error)
{
    console.error('An unhandled exception has occurred:');
    console.error('  Error: '+error);
    console.error();
    process.exit(exit_code.ERROR);
});

/// Handles the SIGTERM signal that may be sent to the process. The process
/// terminates immediately, returning a success code.
process.on('SIGTERM', function ()
{
    process.exit(exit_code.SUCCESS);
});

/// Handles the SIGINT signal that may be sent to the process. The process
/// terminates immediately, returning a success code.
process.on('SIGINT', function ()
{
    process.exit(exit_code.SUCCESS);
});

/// Implements and executes the entry point of the command-line application.
var main = (function Main()
{
    application.args = command_line();
    var ipcMode      = application.args.persistent;
    if (ipcMode    === false)
    {
        compiler_build({
            sourcePath : application.args.sourcePath,
            targetPath : application.args.targetPath,
            platform   : application.args.platform,
            isIPC      : false
        });
    }
}());
