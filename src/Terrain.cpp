#include "Terrain.hpp"
#include "Types.hpp"
#include <algorithm>
#include <fstream>
#include <glm/glm.hpp>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

err Terrain::load(const string &path) {
    std::ifstream file(path);
    if (!file.is_open()) return 1;

    map<pair<int, int>, float> heightsMap;
    vector<float> rawXs, rawZs;

    string line;
    while (getline(file, line)) {
        if (line.rfind("v ", 0) == 0) {
            std::istringstream ss(line.substr(2));
            float x, y, z;
            ss >> x >> y >> z;

            // Scale key to fixed-point integer
            const int kx         = static_cast<int>(round(x * 100.0f));
            const int kz         = static_cast<int>(round(z * 100.0f));
            heightsMap[{kx, kz}] = y;
            rawXs.push_back(x);
            rawZs.push_back(z);
        }
    }
    if (heightsMap.empty()) return 2;

    // Extract and sort unique X and Z coordinate values
    sort(rawXs.begin(), rawXs.end());
    sort(rawZs.begin(), rawZs.end());

    // Populate X and Y keys pruning points too close together on the same axis
    for (float x : rawXs)
        if (this->xCoords.empty() || abs(x - xCoords.back()) > 0.05f)
            this->xCoords.push_back(x);
    for (float z : rawZs)
        if (this->zCoords.empty() || abs(z - zCoords.back()) > 0.05f)
            this->zCoords.push_back(z);

    this->nx = xCoords.size();
    this->nz = zCoords.size();
    this->heights.resize(nx * nz, -64.0f);

    // Populate heights vector
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < nz; ++j) {
            int kx  = static_cast<int>(round(xCoords[i] * 100.0f));
            int kz  = static_cast<int>(round(zCoords[j] * 100.0f));
            auto it = heightsMap.find({kx, kz});
            if (it != heightsMap.end()) heights[i * nz + j] = it->second;
        }
    }
    return 0;
}

float Terrain::getHeight(float x, float z) const {
    if (heights.empty()) return -64.0f;

    // Clamp to terrain boundaries
    if (x <= xCoords.front()) x = xCoords.front();
    if (x >= xCoords.back()) x = xCoords.back();
    if (z <= zCoords.front()) z = zCoords.front();
    if (z >= zCoords.back()) z = zCoords.back();

    // Binary search for cell interval
    auto itX = std::lower_bound(xCoords.begin(), xCoords.end(), x);
    int i    = std::clamp(static_cast<int>(itX - xCoords.begin()) - 1, 0, nx - 2);
    auto itZ = std::lower_bound(zCoords.begin(), zCoords.end(), z);
    int j    = std::clamp(static_cast<int>(itZ - zCoords.begin()) - 1, 0, nz - 2);

    float x0 = xCoords[i], x1 = xCoords[i + 1];
    float z0 = zCoords[j], z1 = zCoords[j + 1];

    float u = (x1 > x0) ? (x - x0) / (x1 - x0) : 0.0f;
    float v = (z1 > z0) ? (z - z0) / (z1 - z0) : 0.0f;

    float h00 = heights[i * nz + j];
    float h10 = heights[(i + 1) * nz + j];
    float h01 = heights[i * nz + (j + 1)];
    float h11 = heights[(i + 1) * nz + (j + 1)];

    // Bilinear interpolation
    return (1.0f - u) * (1.0f - v) * h00 +
           u * (1.0f - v) * h10 +
           (1.0f - u) * v * h01 +
           u * v * h11;
}
