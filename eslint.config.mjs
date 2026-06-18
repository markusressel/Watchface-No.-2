import {defineConfig} from "eslint/config";
import globals from "globals";
import babelParser from "@babel/eslint-parser";
import path from "node:path";
import {fileURLToPath} from "node:url";
import js from "@eslint/js";
import {FlatCompat} from "@eslint/eslintrc";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const compat = new FlatCompat({
    baseDirectory: __dirname,
    recommendedConfig: js.configs.recommended,
    allConfig: js.configs.all
});

export default defineConfig([{
    extends: compat.extends("eslint:recommended"),

    files: ["src/js-modern/**/*.js"],

    languageOptions: {
        globals: {
            ...globals.browser,
            ...globals.node,
            Pebble: "readonly",
            __PEBBLE_RELEASE__: "readonly",
        },

        parser: babelParser,
        ecmaVersion: 2020,
        sourceType: "module",

        parserOptions: {
            requireConfigFile: false,
        },
    },

    rules: {
        "no-unused-vars": "off",        // Completely ignore unused variables
        "no-prototype-builtins": "off", // Ignore direct .hasOwnProperty() calls
        "no-undef": "error",
    },
}]);