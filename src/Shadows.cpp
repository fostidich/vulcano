#include "Vulcano.hpp"

using namespace glm;
using namespace std;

void Vulcano::updateSceneShadows(GlobalUniformBufferObject &gubo) {
    gubo.lightDir = normalize(gubo.lightDir);

    // Center the shadow map on the player.
    // Everything within the set radius of the player is casting shadows.
    const float R           = 256.0f;
    const vec3 shadowCenter = this->player.eyePosition;

    // Position the sun safely high above the player
    const float sunDistance = 4 * R;
    const vec3 lightPos     = shadowCenter - gubo.lightDir * sunDistance;

    // Up vector with zenith guard
    const vec3 up  = (abs(gubo.lightDir.y) > 0.99f) ? vec3(0.0f, 0.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);
    mat4 lightView = lookAt(lightPos, shadowCenter, up);

    // Parallel projection centered on the player
    mat4 lightPrj = ortho(-R, R, -R, R, 1.0f, sunDistance + R);
    lightPrj[1][1] *= -1;

    gubo.lightSpaceMat = lightPrj * lightView;
}
