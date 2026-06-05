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

build:
    pebble build

release:
    PEBBLE_RELEASE=1 pebble build

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

test:
    ./node_modules/.bin/jest tests/js

test-c:
    mkdir -p ./tests/build
    gcc tests/c/util_test.c tests/c/util_host_shim.c -o tests/build/util_test
    ./tests/build/util_test
