#pragma once

#include <GLFW/glfw3.h>
#include "platform_time.h"
#include <string>
#include <stdio.h>
#include <vector>
#include <inttypes.h>

typedef uint64_t U64;
typedef std::string String;


#define VARSTR(v) (String(#v) + ":" + str(v))

