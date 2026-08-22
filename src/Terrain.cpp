// Query max and min Y of the terrain OBJ file:
// - cat assets/models/Terrain.obj | grep -E "^v " | cut -d" " -f 3 | sort -g | head -1
// - cat assets/models/Terrain.obj | grep -E "^v " | cut -d" " -f 3 | sort -g | tail -1
//
// Query Z and X count:
//- cat assets/models/Terrain.obj | grep -E "^v " | cut -d" " -f 2 | sort -g | uniq | wc -l
//- cat assets/models/Terrain.obj | grep -E "^v " | cut -d" " -f 4 | sort -g | uniq | wc -l
//
// The logic of this file is tuned for the specific terrain OBJ file used in
// this application.
// Such terrain model is an exact 257x257 XZ grid, with Ys spanning between
// about (-10, 256) meters high.
// The grid is dense, meaning that for each X and Z found in the model, a Y is
// always defined.
// If the model is scaled on the XZ coordinates, such scale quantity needs to
// be set in the header, so that the height check works properly.
// Also, since the model is assumed to be spanning from XZ (0, 0) to XZ
// (256, 256), the height check is done assuming also that the model was
// translated to align with world origin, thus with XZ spanning between -128
// and 128.

#include "Terrain.hpp"
#include "Types.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

err Terrain::load(const string &path) {
    std::ifstream file(path);
    if (!file.is_open()) return 1;

    this->nx = 257;
    this->nz = 257;
    this->heights.assign(nx * nz, -64.0f);

    string line;
    int vertexCount = 0;
    while (getline(file, line)) {
        if (line.rfind("v ", 0) == 0) {
            std::istringstream ss(line.substr(2));
            float x, y, z;
            if (ss >> x >> y >> z) {
                const int ix = static_cast<int>(round(x));
                const int iz = static_cast<int>(round(z));
                if (ix >= 0 && ix < nx && iz >= 0 && iz < nz)
                    this->heights[ix * nz + iz] = y;
            }
        }
    }

    this->loaded = true;
    return 0;
}

float Terrain::getHeight(float x, float z) const {
    if (!loaded) return -64.0f;

    // Convert world coordinates to 0-256 grid space
    float px = clamp((x / scale) + 128.0f, 0.0f, 256.0f);
    float pz = clamp((z / scale) + 128.0f, 0.0f, 256.0f);

    usize gx = clamp((int)floor(px), 0, nx - 2);
    usize gz = clamp((int)floor(pz), 0, nz - 2);
    float u  = px - gx;
    float v  = pz - gz;

    // Fetch four neighboring grid heights
    float h00 = heights[gx * nz + gz];
    float h10 = heights[(gx + 1) * nz + gz];
    float h01 = heights[gx * nz + (gz + 1)];
    float h11 = heights[(gx + 1) * nz + (gz + 1)];

    // Bilinear interpolation
    return (1.0f - u) * (1.0f - v) * h00 +
           u * (1.0f - v) * h10 +
           (1.0f - u) * v * h01 +
           u * v * h11;
}
