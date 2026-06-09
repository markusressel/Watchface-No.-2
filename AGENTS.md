# Build

Use the [Justfile](./Justfile) to build the project:

```shell
# optional clean
just clean
# build
just build
```

or run in relevant emulators (preferred for instant visual feedback from the developer):

```shell
just run emery flint
```

Tasks too complex to be written directly into the Justfile should use Python 3 like [run.py](scripts/run.py).

# Test

Run all tests:

```shell
just test
```

## JavaScript Configuration Page

Setup a node.js environment

```shell
npm init -y
npm install --save-dev jest
```

Run JS tests:

```shell
just test-js   # JS Only
```

## C Implementation

`gcc` must be available for C tests.

Run C tests:

```shell
just test-c    # C Only
```

# Settings

Working with settings requires many different parts:

1. [JavaScript Configuration Page](./pkjs/configPage.js) - This is the page that is shown in the Pebble app when you click on the settings of the watchface. It is written in
   JavaScript and uses the Pebble JS API to communicate with the watchface.
    - `MESSAGE_KEY_*` consts are generated based on the entries in the `package.json` file. To ensure they are generated correctly after changes were made to the `package.json` **a
      clean build is required**.
2. App Messaging - This is the part of the watchface that handles the communication between the watchface and the JavaScript Configuration Page. It receives the settings from the
   JavaScript Configuration Page and saves them in persistent storage.
    1. [C Implementation](./src/c/app_messaging.c) - This is the C implementation of the App Messaging. It uses the Pebble C API to receive messages from the JavaScript
       Configuration Page and save them in persistent storage.
    2. [package.json](./package.json) - Requires "pebble"->"messageKeys" to be set to the keys used in the JavaScript Configuration Page.
3. [Settings Storage](./src/c/clay_settings.c) - This is the C implementation of the persistent storage for the settings.

# Platform limitations

## Watch Hardware

### Stack size

The stack of the Pebble ecosystem is limited, storing big amounts of data in it results in an OS crash (stackoverflow).
To avoid these issues use heap allocation for large data structures and avoid recursion.

## App-Side

### JavaScript Engine (pkjs)

This project uses babel to transpile modern JS features into a compatible style for the Pebble SDK build environment.
Source code is written in `src/js-modern`, transpiled code lands in `build/generated/pkjs`, which is picked up by the pebble sdk.

# Codestyle

## C

- You may use `const` on variables and function parameters where possible and reasonable, except:
    - Pebble SDK methods

## JavaScript

- prefer `let` over `var`
- write js docs for parameters and return values including `{type}` information (f.ex. `{number}`) to describe the expected input and output types.

# Developer Options

Can be found in [developer_options.h](./src/c/developer_options.h).
