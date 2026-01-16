cmake_minimum_required(VERSION 3.10)
project(Watchface_No_2 C)

# 1. Automatic Source Discovery (No manual listing needed!)
file(GLOB_RECURSE SOURCES "src/*.c")

# 2. Tell CLion where the Pebble SDK lives
# Replace 'markus' with your username if it differs
set(PEBBLE_SDK_PATH "/home/markus/.pebble-sdk/SDKs/current")

include_directories(
        "${PEBBLE_SDK_PATH}/sdk-core/pebble/basalt/include"
        "${PEBBLE_SDK_PATH}/toolchain/arm-none-eabi/arm-none-eabi/include"
        "include"
        "build"
        "build/include"
)

# 3. Create a dummy target so CLion indexes the files
add_executable(Watchface_No_2 ${SOURCES})