#include "Vulcano.hpp"

using namespace glm;
using namespace std;

// void Vulcano::updateSceneShadows(GlobalUniformBufferObject &gubo) {
//     // The sun position is placed at Y 320 (higher than mountains), its look at
//     // point is placed at the center of the player camera frustum, the extent of
//     // the ortho projection is computed based on the size of the frustum, so to
//     // include all of it (width, height, near/far plane), plus some extra.
//
//     // The inverse of the view projection matrix takes a point in the frustum
//     // coordinates and puts them back to the original world coordinates.
//     const mat4 invCam = glm::inverse(this->ViewPrj);
//     gubo.lightDir     = normalize(gubo.lightDir);
//
//     // Find frustum corners and center in world coordinates.
//     vector<vec4> corners;
//     vec3 frustumCenter = vec3(0.0f);
//
//     // Calculate camera frustum corners and center
//     for (usize x = 0; x < 2; ++x) {
//         for (usize y = 0; y < 2; ++y) {
//             for (usize z = 0; z < 2; ++z) {
//                 vec4 pt = invCam * vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, (float)z, 1.0f);
//                 pt /= pt.w;
//                 corners.push_back(pt);
//                 frustumCenter += vec3(pt);
//             }
//         }
//     }
//     frustumCenter /= 8.0f;
//
//     // Compute bounding radius R: this is the radius of the sphere that
//     // originating from frustum center will safely comprise all corners, plus
//     // extra outside space to cope with objects that are casting shadows from
//     // outside frustum.
//     float R = 0.0f;
//     for (const auto &c : corners)
//         R = std::max(R, length(vec3(c) - frustumCenter));
//     R *= 1.3f;
//
//     // Move sun backwards along light direction until its altitude is safely above terrain.
//     const float targetHeight = 320.0f;
//     float deltaHabove        = targetHeight - frustumCenter.y;
//     float deltaHbelow        = frustumCenter.y - -64.0f;
//     float sinGroundSun       = std::max(0.1f, -gubo.lightDir.y);
//     float distAbove          = std::max(R, deltaHabove / sinGroundSun);
//     float distBelow          = std::max(R, deltaHbelow / sinGroundSun);
//     vec3 lightPos            = frustumCenter - gubo.lightDir * distAbove;
//
//     // Construct view-projection matrix from the sun
//     vec3 up        = (abs(gubo.lightDir.y) > 0.99f) ? vec3(0.0f, 0.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);
//     mat4 lightView = lookAt(lightPos, frustumCenter, up);
//     mat4 lightPrj  = ortho(-R, R, -R, R, 1.0f, distAbove + distBelow);
//     lightPrj[1][1] *= -1;
//     gubo.lightSpaceMat = lightPrj * lightView;
// }

void Vulcano::updateSceneShadows(GlobalUniformBufferObject &gubo) {
    gubo.lightDir = normalize(gubo.lightDir);

    // Center the shadow map on the player.
    // Everything within the set radius of the player is casting shadows.
    const float R           = 256.0f;
    const vec3 shadowCenter = this->player.eyePosition;

    // Position the sun safely high above the player
    const float sunDistance = 512.0f;
    const vec3 lightPos     = shadowCenter - gubo.lightDir * sunDistance;

    // Up vector with zenith guard
    const vec3 up  = (abs(gubo.lightDir.y) > 0.99f) ? vec3(0.0f, 0.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);
    mat4 lightView = lookAt(lightPos, shadowCenter, up);

    // Parallel projection centered on the player
    mat4 lightPrj = ortho(-R, R, -R, R, 1.0f, sunDistance + R + 128.0f);
    lightPrj[1][1] *= -1;

    gubo.lightSpaceMat = lightPrj * lightView;
}
