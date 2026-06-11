#!/usr/bin/env python3
import argparse
import json
import os.path
import re

AUTO_GENERATED_HEADER = """// ==========================================================
// AUTO-GENERATED FILE - DO NOT EDIT MANUALLY!
// This file is generated during compile time from src/js-modern/configPage.js
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
    print("Generating src/js-modern/generated/settings.js from src/js-modern/config/configPage.js...")

    with open('src/js-modern/config/configPage.js', 'r') as f:
        content = f.read()

    # Remove JavaScript module export to make it valid JSON
    json_content = content.replace('export default ', '').strip()
    if json_content.endswith(';'):
        json_content = json_content[:-1]

    # Remove trailing commas
    json_content = re.sub(r',\s*([\]}])', r'\1', json_content)

    try:
        config = json.loads(json_content)
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON from configPage.js: {e}")
        exit(1)

    items = extract_config_items(config)

    defaults_lines = []
    getters_setters_lines = []
    coerce_lines = []

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

        defaults_lines.append(f"        {key}: {default_str},")
        getters_setters_lines.append(f"    get {key}() {{ return this._settings.{key}; }}")
        getters_setters_lines.append(f"    set {key}(val) {{ this._settings.{key} = {js_type}(val); }}")
        getters_setters_lines.append("")
        coerce_lines.append(f"        this._settings.{key} = {js_type}(this._settings.{key});")


    js_lines = [
        AUTO_GENERATED_HEADER,
        "",
        "/**",
        " * @class Settings",
        " * @classdesc A class to manage user settings.",
        " */",
        "export default class Settings {",
        "    _DEFAULTS = {",
    ]

    js_lines.extend(defaults_lines)

    js_lines.extend([
        "    };",
        "",
        "    _settings;",
        "",
        "    constructor(persistedSettings) {",
        "        this._settings = Object.assign({}, this._DEFAULTS, persistedSettings || {});",
    ])

    js_lines.extend(coerce_lines)

    js_lines.extend([
        "    }",
        ""
    ])

    js_lines.extend(getters_setters_lines)

    js_lines.extend([
        "    toJSON() {",
        "        return this._settings;",
        "    }",
        "}"
    ])

    # Write the file to the src/js-modern/generated directory
    out_dir = os.path.join('src', 'js-modern', 'generated')
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    with open(os.path.join(out_dir, 'settings.js'), 'w') as f:
        f.write('\n'.join(js_lines))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate code and settings")
    args = parser.parse_args()
    generate_settings_js()