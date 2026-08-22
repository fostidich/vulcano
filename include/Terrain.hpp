#pragma once
#include <string>
#include <vector>

class Terrain {
  public:
    const int load(const std::string &path);
    float getHeight(float x, float z) const;
    float scale = 16.0f;

  protected:
    std::vector<float> heights;
    int nx      = 257;
    int nz      = 257;
    bool loaded = false;
};
