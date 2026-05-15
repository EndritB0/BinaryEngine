#pragma once

#include <utility>

#define BIND_FUNCTION(function) [this](auto&&... args) -> decltype(auto) { return this->function(std::forward<decltype(args)>(args)...); }

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Core/Color.h"
