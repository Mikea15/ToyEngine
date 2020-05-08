#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
//#define GLM_MESSAGES

#define ENABLE_SIMD 0
#if ENABLE_SIMD
#define GLM_FORCE_SSE2
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif

#include <glm/glm.hpp>

#include <glm/mat4x4.hpp>

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

#include <glm/gtx/norm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glm/ext/vector_float3.hpp>