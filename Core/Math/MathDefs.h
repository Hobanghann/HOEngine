#pragma once

#include <limits>

#include "Macros.h"

namespace ho
{
namespace math
{
INLINE constexpr float FLOAT_MAX = std::numeric_limits<float>::max();
INLINE constexpr float FLOAT_MIN = std::numeric_limits<float>::min();
INLINE constexpr float FLOAT_INF = std::numeric_limits<float>::infinity();
INLINE constexpr float FLOAT_NAN = std::numeric_limits<float>::quiet_NaN();

// Basic scalar/vector comparison epsilon.
// Used for generic floating point equality checks where values are expected
// to be of similar magnitude (not squared values).
INLINE constexpr float EPSILON_CMP = 1e-5f;

// Squared epsilon for squared-length or squared-distance comparisons.
// Avoids taking sqrt() when comparing vector lengths or distances.
// This is simply EPSILON_CMP * EPSILON_CMP.
INLINE constexpr float EPSILON_CMP2 = EPSILON_CMP * EPSILON_CMP;

// Epsilon for values expected to be close to unit length or unit scale.
// Example: checking if a normalized vector has length := 1,
// or if a scale factor is := 1.0.
// This value can be adjusted for stricter or looser unit checks.

INLINE constexpr float EPSILON_UNIT = 1e-3f;

// Epsilon for determining whether a vector can be safely normalized.
// If the vector's length is below this threshold, normalization should be
// avoided to prevent division by very small numbers.
INLINE constexpr float EPSILON_NORMALIZABLE = 1e-6f;

// Epsilon for checking if a point lies on a plane.
// Used to compare the absolute signed distance from the point to the plane to
// zero.
INLINE constexpr float EPSILON_POINT_ON_PLANE = 1e-5f;

// Epsilon for checking if a matrix is orthogonal or orthonormal.
INLINE constexpr float EPSILON_ORTHO = 1e-3f;

// Epsilon for offset used in depth testing
INLINE constexpr float EPSILON_DEPTH_TEST = 1e-3f;

// Epsilon for rasterization comparisons.
// Used where floating-point error accumulates during rasterization
// (edge tests, barycentric coords, NDC/screen-space boundaries).
// Looser tolerance to ensure stable pixel coverage near edges.
INLINE constexpr float EPSILON_RASTERIZATION = 1e-4f;

INLINE constexpr float PI = 3.14159265358979323846f;
INLINE constexpr float PI2 = 1.57079632679489661923f;
INLINE constexpr float PI4 = 0.785398163397448309616f;
INLINE constexpr float INV_PI = 0.318309886183790671538f;
INLINE constexpr float DEG1_IN_RAD = PI / 180.0f;
INLINE constexpr float RAD1_IN_DEG = 1.0f / DEG1_IN_RAD;

enum class eClockDirection
{
    ClockWise = 0,
    CounterClockWise,
};

enum class eEulerOrder
{
    XYZ = 0,
    XZY,
    YZX,
    YXZ,
    ZXY,
    ZYX,
};

enum class eSide
{
    Outside = 0,
    Inside,
    On,
    Intersect,
};
} // namespace math
} // namespace ho