#pragma once

#include "debug/assert.h"

#define test_fail(fmt, ...) assert_fail(fmt, ##__VA_ARGS__)