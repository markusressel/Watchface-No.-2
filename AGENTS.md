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

# Codestyle

You may use `const` on variables and function parameters where possible and reasonable, except:

- Pebble SDK methods

# Developer Options

Can be found in [developer_options.h](./src/c/developer_options.h).
