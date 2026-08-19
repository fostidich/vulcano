#pragma once
#include "modules/TextMaker.hpp"

class SimpleTextMaker : public TextMaker {
  public:
    void printTopLeft(const std::string &s);
    void printTopRight(const std::string &s);
    void printTopCenter(const std::string &s);
    void printBottomLeft(const std::string &s);
    void printBottomRight(const std::string &s);
    void printBottomCenter(const std::string &s);
    void printCenter(const std::string &s);

  protected:
    void txtPrint(const std::string &s, int id, float x, float y);
};
