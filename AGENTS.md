# Build

Use the [Justfile](./Justfile) to build the project:

```bash
# optional clean
just clean
# build
just build
```

or run in relevant emulators (preferred for instant visual feedback from the developer):

```bash
just run emery flint
```

# Settings

Working with settings requires many different parts:

1. [JavaScript Configuration Page](./pkjs/configPage.js) - This is the page that is shown in the Pebble app when you click on the settings of the watchface. It is written in JavaScript and uses the Pebble JS API to communicate with the watchface.
2. App Messaging - This is the part of the watchface that handles the communication between the watchface and the JavaScript Configuration Page. It receives the settings from the JavaScript Configuration Page and saves them in persistent storage.
   1. [C Implementation](./src/c/app_messaging.c) - This is the C implementation of the App Messaging. It uses the Pebble C API to receive messages from the JavaScript Configuration Page and save them in persistent storage.
   2. [package.json](./package.json) - Requires "pebble"->"messageKeys" to be set to the keys used in the JavaScript Configuration Page.
3. [Settings Storage](./src/c/clay_settings.c) - This is the C implementation of the persistent storage for the settings.