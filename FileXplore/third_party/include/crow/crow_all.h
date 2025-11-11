#pragma once

#ifdef _WIN32
    #define NOMINMAX
    #define _WIN32_WINNT 0x0601
    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif

#include "app.h"
#include "http_server.h"
#include "routing.h"
#include "middleware_context.h"
#include "http_request.h"
#include "http_response.h"
#include "json.h"