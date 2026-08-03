#pragma once
#include "glm/fwd.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// UBO is a struct used for storing transform matrices for each object.
// A UBO object will be assigned to each object/mesh in the scene.
// The GPU then keeps the UBOs updated whenever object moves (model matrix
// M/`mMat` is updated), camera moves (view matrix V/`View`), or screen
// changes (projection matrix P/`Prj`, e.g. due to screen resize).
// Such UBOs will be mapped in a location where vertex shaders are able to
// access them.
struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
};

// GUBO is a struct used to store a direct light model shared by all the scene.
// It will be mapped to a location where fragment shaders are able to access it.
// It also stores the frame's camera location, so that it will be possible to
// compute specular highlights (shiny surfaces).
struct GlobalUniformBufferObject {
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
};

// A vertex is a useful struct for storing the required data for each single
// vertex (point in the 3D world scene).
// It includes the position (x, y, z) and the coordinates they map on 2D texture
// images (u, v).
// They can store also other information such as tangents or normals.
struct Vertex {
    glm::vec3 pos;
    glm::vec2 UV;
};
