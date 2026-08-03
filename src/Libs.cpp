// This module contains the implementation of the library, to speed up the
// compilation of the main file

// IWYU pragma: begin_keep

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define STARTER_IMPLEMENTATION
#include "modules/Starter.hpp"

#define TEXTMAKER_IMPLEMENTATION
#include "modules/TextMaker.hpp"

#define SCENE_IMPLEMENTATION
#include "modules/Scene.hpp"

#define ANIMATIONS_IMPLEMENTATION
#include "modules/Animations.hpp"

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// IWYU pragma: end_keep
