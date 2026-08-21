#pragma once
#include <string>
#include <vector>

class Terrain {
  public:
    const int load(const std::string &path);
    float getHeight(float x, float z) const;

  protected:
    std::vector<float> xCoords;
    std::vector<float> zCoords;
    std::vector<float> heights;
    int nx = 0, nz = 0;
};
