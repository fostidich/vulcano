#include <string>
#include <vector>

class Terrain {
  protected:
    void load(const std::string &path);
    float getHeight(float x, float z) const;

  public:
    std::vector<float> xCoords;
    std::vector<float> zCoords;
    std::vector<float> heights;
    int nx = 0, nz = 0;
};
