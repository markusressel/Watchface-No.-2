# Project: Watchface No. 2

A dot-styled, highly configurable watchface for the Pebble ecosystem.

# Directives for Gemini CLI

As a senior engineer agent, adhere to these mandates:

- **Reproduction First**: For bug fixes, always create a reproduction test case or script before implementing a fix.
- **Validation**: Every change must be verified using `just test` (or specific `test-c`/`test-js`) and confirmed by a successful build.
- **Surgical Updates**: Minimize changes to the absolute necessary. Adhere to existing patterns.
- **Source of Truth**:
  - `package.json` -> `pebble.messageKeys`: Authority for all AppMessage communication keys.
  - `src/js-modern/config/configPage.js`: Authority for settings UI and default values.
  - `src/c/developer_options.h`: Authority for debug and developer flags.
- **JS Development**: ONLY edit files in `src/js-modern`. Files in `build/generated/pkjs` are auto-generated.

# Environment Setup

```shell
# Install dependencies
npm install
just setup
```

# Build & Deploy

Use the [Justfile](./Justfile) for all operations:

```shell
# Standard build (runs generation and transpilation)
just build

# Clean and build
just clean && just build

# Deploy to emulator (e.g., emery) with logs
just deploy debug emery
```

## Build Flags (C Implementation)

- **Release Build**: `#define WF_RELEASE 1` (via `just release`)
- **Debug Build**: `#define WF_DEBUG 1` (default)
- **Emulator Build**: `#define WF_EMULATOR 1` (automatically set when running in emulator)

# Testing

```shell
# Run all tests
just test

# Language specific
just test-c
just test-js
```

- **C Tests**: Use Unity framework in `tests/c`. Requires `gcc`.
- **JS Tests**: Use Jest in `tests/js`.

# Settings Architecture

Modifying settings is a multi-step process:

1. **Define Key**: Add entry to `pebble.messageKeys` in `package.json`.
2. **UI & Default**: Add item to `src/js-config/configPage.js`.
3. **Generate Helper**: Run `just generate` (or `just build`) to update 
    * `src/js-modern/generated/settings.js` and
    * `src/js-modern/generated/configPage.js`
4. **C Receipt**: Update `src/c/app_messaging/app_messaging.c` to handle the new key.
5. **C Storage**: Update `src/c/settings/clay_settings.c` and `persist_keys.h` for persistence.

**Note**: A `just clean` is required if `package.json` message keys change to ensure they are picked up by the Pebble build system.

# Project Structure

- `src/c/`: Pebble C source code.
  - `app_messaging/`: Communication with JS.
  - `settings/`: Persistence and configuration storage.
  - `ui/`: Layout and drawing logic.
  - `ui/layer/`: Specific watchface components (time, weather, etc.).
- `src/js-modern/`: Modern JavaScript source (ES6+).
  - `config/`: Configuration page logic.
  - `weather/`: Weather provider implementations.
  - `generated/`: Auto-generated JS helpers (Do not edit).
- `scripts/`: Build and utility scripts (Python).
- `tests/`: Unity (C) and Jest (JS) test suites.

# Platform Limitations

## Watch Hardware (C)

- **Stack Size**: Extremely limited. Avoid large local arrays and deep recursion. Use heap allocation (`malloc`) for large structures.
- **Memory**: Be mindful of heap fragmentation.
- **Persistence**: Data per key must be limited to 256 bytes.

## App-Side (JavaScript)

- **Limited Environment**: No Node.js built-ins. Use standard ES5/ES6 features.
- **Dependencies**: NO external dependencies allowed in the final JS bundle, except for `@rebble/clay` for the config page. Babel transpiles modern features, but do not add new
  `npm` packages to the logic.

# Codestyle

## C

- Use `const` for variables and parameters where possible (except Pebble SDK callbacks).
- Use descriptive naming following `snake_case`.

## JavaScript

- Prefer `let`/`const` over `var`.
- Use descriptive naming following `CamelCase`.
- Use JSDoc for all functions to provide type information to CLion: `/** @param {type} name - description */`.
- Explicitly define return types in JSDoc.
