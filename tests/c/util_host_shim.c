#include "pebble_mock.h" // Contains HOST_TEST_SHIM_BUILD and APP_LOG mock

// Include the original util.c directly to compile its functions
// in the context of this host test shim.
#include "../../src/c/util.c"
