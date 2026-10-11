#pragma once

#include <KDecoration3/kdecoration3/scalehelpers.h>
#define ALIGN(a, b) KDecoration3::snapToPixelGrid(a, b)
#define ALIGNFLOOR(a, b) (std::floor((a) * (b)) / (b))

#define FLOOR(a, b) std::floor((a) * (b))
#define ROUND(a, b) std::round((a) * (b))
