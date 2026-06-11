#
# This file is based on the default set of rules to compile a Pebble project.
#
# Edits made:
# - Added PEBBLE_EMULATOR_BUILD environment variable check to allow building with PBL_EMULATOR defined, which is used to run custom code in Emulator environments.
# - Added PEBBLE_RELEASE
# - Added dynamic generation of generated_settings.js based on package.json messageKeys
# - Added Babel transpilation step for modern JavaScript
# - Added cppcheck static analysis
#

import os.path
import subprocess
import shutil

try:
    from sh import CommandNotFound, jshint, cat, ErrorReturnCode_2, ErrorReturnCode
    hint = jshint
except (ImportError, CommandNotFound):
    hint = None

top = '.'
out = 'build'


def options(ctx):
    ctx.load('pebble_sdk')


def configure(ctx):
    ctx.load('pebble_sdk')


def run_cppcheck(ctx):
    try:
        from sh import cppcheck
        print("Running cppcheck...")
        try:
            cppcheck(
                '--enable=all',
                '--inconclusive',
                '--std=c99',
                '--suppress=missingIncludeSystem',
                '--suppress=unusedFunction',
                '--inline-suppr',
                '--template=gcc',
                'src/c',
                _tty_out=False,
                _err_to_out=True
            )
        except ErrorReturnCode as e:
            ctx.fatal(f"\ncppcheck failed:\n" + e.stdout.decode('utf-8'))
    except (ImportError, CommandNotFound):
        ctx.fatal("\ncppcheck command not found. Please install cppcheck.")

def run_lint_js(ctx):
    try:
        from sh import npx, CommandNotFound, ErrorReturnCode
        print("Running eslint...")
        try:
            npx('eslint', 'src/js-modern/', _tty_out=False)
        except ErrorReturnCode as e:
            ctx.fatal("\nESLint linting failed!\n" + e.stdout.decode('utf-8'))
    except (ImportError, CommandNotFound):
        ctx.fatal("\nnpx or eslint command not found.")

def run_babel_transpilation(ctx):
    # Clean the old pkjs directory
    pkjs_dir = os.path.join('build', 'generated', 'pkjs')
    if os.path.exists(pkjs_dir):
        shutil.rmtree(pkjs_dir)

    # Run Babel transpilation
    print("Transpiling modern JavaScript with Babel...")
    npm_bin = shutil.which('npm')
    if npm_bin is None:
        ctx.fatal("npm command not found. Please install Node.js and npm.")
    subprocess.run([npm_bin, "run", "babel"], check=True)

def build(ctx):
    subprocess.run(["just", "generate"], check=True)

    run_cppcheck(ctx)
    run_lint_js(ctx)
    run_babel_transpilation(ctx)

    if False and hint is not None:
        try:
            hint([node.abspath() for node in ctx.path.ant_glob("src/**/*.js")], _tty_out=False) # no tty because there are none in the cloudpebble sandbox.
        except ErrorReturnCode_2 as e:
            ctx.fatal("\nJavaScript linting failed (you can disable this in Project Settings):\n" + e.stdout)

    ctx.load('pebble_sdk')

    release_build = os.environ.get('PEBBLE_RELEASE') == '1'
    if release_build:
        print('Building release mode (PEBBLE_RELEASE=1): APP_LOG disabled')
    else:
        print('Building development mode: APP_LOG enabled')

    build_worker = os.path.exists('worker_src')
    binaries = []

    for p in ctx.env.TARGET_PLATFORMS:
        ctx.set_env(ctx.all_envs[p])
        ctx.set_group(ctx.env.PLATFORM_NAME)

        ctx.env.CFLAGS.append('-include')
        ctx.env.CFLAGS.append('src/c/developer_options.h')

        if release_build:
            ctx.env.CFLAGS.append('-DWF_RELEASE=1')
            ctx.env.CFLAGS.append('-include')
            ctx.env.CFLAGS.append('src/c/release.h')
        else:
            ctx.env.CFLAGS.append('-DWF_DEBUG=1')

        # Check for PEBBLE_EMULATOR_BUILD environment variable
        if os.environ.get('PEBBLE_EMULATOR_BUILD') == '1':
            ctx.env.CFLAGS.append('-DPBL_EMULATOR')

        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_program(source=ctx.path.ant_glob('src/c/**/*.c'), target=app_elf)

        if build_worker:
            worker_elf = '{}/pebble-worker.elf'.format(ctx.env.BUILD_DIR)
            binaries.append({'platform': p, 'app_elf': app_elf, 'worker_elf': worker_elf})
            ctx.pbl_worker(source=ctx.path.ant_glob('worker_src/c/**/*.c'), target=worker_elf)
        else:
            binaries.append({'platform': p, 'app_elf': app_elf})

    ctx.set_group('bundle')

    # Correctly find JS files inside the build directory using bldnode
    pkjs_node = ctx.bldnode.make_node('generated/pkjs')
    js_files = pkjs_node.ant_glob(['**/*.js', '**/*.json'])
    js_entry = 'build/generated/pkjs/index.js'

    ctx.pbl_bundle(binaries=binaries, js=js_files, js_entry_file=js_entry)
