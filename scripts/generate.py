#!/usr/bin/env python3
import argparse
import json
import os.path
import re

AUTO_GENERATED_HEADER = """// ==========================================================
// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY!
// This file is generated during compile time from src/pkjs/configPage.js
// =========================================================="""

def extract_config_items(config):
    items = []
    if isinstance(config, list):
        for item in config:
            items.extend(extract_config_items(item))
    elif isinstance(config, dict):
        if 'messageKey' in config:
            items.append(config)
        if 'items' in config:
            items.extend(extract_config_items(config['items']))
    return items

def generate_settings_js():
    print("Generating src/pkjs/generated/settings.js from src/pkjs/config/configPage.js...")

    with open('src/pkjs/config/configPage.js', 'r') as f:
        content = f.read()

    # Remove JavaScript module export to make it valid JSON
    json_content = content.replace('module.exports = ', '').strip()
    if json_content.endswith(';'):
        json_content = json_content[:-1]

    # Remove trailing commas
    json_content = re.sub(r',\s*([\]}])', r'\1', json_content)

    try:
        config = json.loads(json_content)
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON from configPage.js: {e}")
        return

    items = extract_config_items(config)

    defaults_lines = []
    prototypes_lines = []

    for item in items:
        key = item['messageKey']
        default_val = item.get('defaultValue')
        item_type = item.get('type')

        if default_val is not None:
            if isinstance(default_val, bool):
                js_type = "Boolean"
                default_str = str(default_val).lower()
            elif isinstance(default_val, (int, float)):
                js_type = "Number"
                default_str = str(default_val)
            else:
                js_type = "String"
                default_str = f"'{default_val}'"
        else:
            if item_type == 'toggle':
                js_type = "Boolean"
                default_str = "false"
            elif item_type in ('slider', 'select'):
                js_type = "Number"
                default_str = "0"
            else:
                js_type = "String"
                default_str = "''"

        defaults_lines.append(f"    {key}: {default_str},")
        prototypes_lines.append(f"Object.defineProperty(Settings.prototype, '{key}', {{")
        prototypes_lines.append(f"  get: function() {{ return this._settings.{key}; }},")
        prototypes_lines.append(f"  set: function(val) {{ this._settings.{key} = {js_type}(val); }}")
        prototypes_lines.append(f"}});")

    js_lines = [
        AUTO_GENERATED_HEADER,
        "",
        "function objectAssign(target, varArgs) {",
        "  'use strict';",
        "  if (target == null) {",
        "    throw new TypeError('Cannot convert undefined or null to object');",
        "  }",
        "  let to = Object(target);",
        "  for (let index = 1; index < arguments.length; index++) {",
        "    let nextSource = arguments[index];",
        "    if (nextSource != null) {",
        "      for (let nextKey in nextSource) {",
        "        if (Object.prototype.hasOwnProperty.call(nextSource, nextKey)) {",
        "          to[nextKey] = nextSource[nextKey];",
        "        }",
        "      }",
        "    }",
        "  }",
        "  return to;",
        "}",
        "",
        "function Settings(persistedSettings) {",
        "  this._DEFAULTS = {",
    ]

    js_lines.extend(defaults_lines)

    js_lines.extend([
        "  };",
        "  this._settings = objectAssign({}, this._DEFAULTS, persistedSettings || {});",
        "}",
        ""
    ])

    js_lines.extend(prototypes_lines)
    
    js_lines.extend([
        "",
        "Settings.prototype.toJSON = function() {",
        "  return this._settings;",
        "};",
        "",
        "module.exports = function(persistedSettings) { return new Settings(persistedSettings); };"
    ])

    # Write the file to the src/pkjs/generated directory
    out_dir = os.path.join('src', 'pkjs', 'generated')
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    with open(os.path.join(out_dir, 'settings.js'), 'w') as f:
        f.write('\n'.join(js_lines))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate code and settings")
    args = parser.parse_args()
    generate_settings_js()