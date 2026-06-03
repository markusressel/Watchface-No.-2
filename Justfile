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

clean-emulator platforms: kill wipe

[arg("platforms", pattern="phone|basalt|chalk|diorite|emery|flint")]
run *platforms:
    ./scripts/run.py {{ platforms }}

[arg("platform", pattern="basalt|chalk|diorite|emery|flint")]
app-config platform:
    pebble emu-app-config --emulator "{{ platform }}"

kill:
    pebble kill

wipe: kill
    pebble wipe

[arg("platform", pattern="phone|basalt|chalk|diorite|emery|flint")]
deploy platform: (run platform) (logs platform)

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
