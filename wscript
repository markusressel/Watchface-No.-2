#
# This file is based on the default set of rules to compile a Pebble project.
#
# Edits made:
# - Added PEBBLE_EMULATOR_BUILD environment variable check to allow building with PBL_EMULATOR defined, which is used to run custom code in Emulator environments.
# - Added PEBBLE_RELEASE
#

import os.path
try:
    from sh import CommandNotFound, jshint, cat, ErrorReturnCode_2
    hint = jshint
except (ImportError, CommandNotFound):
    hint = None

top = '.'
out = 'build'


def options(ctx):
    ctx.load('pebble_sdk')


def configure(ctx):
    ctx.load('pebble_sdk')


def build(ctx):
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

        if release_build:
            ctx.env.CFLAGS.append('-DRELEASE=1')
            ctx.env.CFLAGS.append('-include')
            ctx.env.CFLAGS.append('src/c/release.h')

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
    ctx.pbl_bundle(binaries=binaries, js=ctx.path.ant_glob(['src/pkjs/**/*.js', 'src/pkjs/**/*.json']), js_entry_file='src/pkjs/index.js')
