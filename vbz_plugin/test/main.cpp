#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"

#include "vbz_plugin.h"
#include "vbz_plugin_user_utils.h"

static bool plugin_init_result = vbz_register();
