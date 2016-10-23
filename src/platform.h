#pragma once

#include <GLFW/glfw3.h>
#include "platform_time.h"
#include "platform_basic_types.h"
#include <stdio.h>
#include <vector>
#include <atomic>
#include "tinyformat/tinyformat.h"

#define VARSTR(v) (String(#v) + ":" + str(v) )
#define VARSTR2(v,v2) (String(#v) + ":" + str(v,v2) )

