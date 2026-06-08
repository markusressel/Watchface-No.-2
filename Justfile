setup:
    uv tool install pebble-tool --python 3.13
    pebble sdk install latest

[arg("platform", pattern="phone|basalt|chalk|diorite|emery|flint")]
screenshot platform:
    #!/usr/bin/env bash
    if [[ "{{ platform }}" == "phone" ]]; then \
      pebble screenshot --phone 192.168.2.159; \
    else \
      pebble screenshot --emulator "{{ platform }}"; \
    fi

generate:
    ./scripts/generate.py

build: generate
    ./scripts/build.py

release: generate
    PEBBLE_RELEASE=1 ./scripts/build.py --minify

clean-emulator platforms: kill wipe

[arg("mode", pattern="debug|release")]
[arg("platforms", pattern="phone|basalt|chalk|diorite|emery|flint")]
run mode *platforms:
    ./scripts/run.py {{ platforms }} --{{ mode }}

[arg("platform", pattern="basalt|chalk|diorite|emery|flint")]
app-config platform:
    pebble emu-app-config --emulator "{{ platform }}"

kill:
    pebble kill

wipe: kill
    pebble wipe

[arg("mode", pattern="debug|release")]
[arg("platform", pattern="phone|basalt|chalk|diorite|emery|flint")]
deploy mode platform:
    ./scripts/run.py {{ platform }} --logs --{{ mode }}

[arg("platform", pattern="phone|basalt|chalk|diorite|emery|flint")]
logs platform:
    #!/usr/bin/env bash
    if [[ "{{ platform }}" == "phone" ]]; then \
      pebble logs --phone 192.168.2.159; \
    else \
      pebble logs --emulator "{{ platform }}"; \
    fi

clean:
    pebble clean

test: test-c test-js

test-js:
    ./node_modules/.bin/jest tests/js

test-c:
    ./scripts/tests.py
