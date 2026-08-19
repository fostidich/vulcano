#include "SimpleTextMaker.hpp"
#include "modules/TextMaker.hpp"

void SimpleTextMaker::txtPrint(const std::string &s, int id, float x, float y) {
    const bool italic           = false;
    const bool bold             = false;
    const bool small            = x != 0.0f;
    const TextAlignment tal     = x < 0.0f ? TAL_LEFT : (x > 0.0f ? TAL_RIGHT : TAL_CENTER);
    const TextRegistrationH trh = x < 0.0f ? TRH_LEFT : (x > 0.0f ? TRH_RIGHT : TRH_CENTER);
    const TextRegistrationV trv = y < 0.0f ? TRV_TOP : (y > 0.0f ? TRV_BOTTOM : TRV_MIDDLE);
    this->print(x, y, s, id, "CO", italic, bold, small, tal, trh, trv);
}

void SimpleTextMaker::printTopLeft(const std::string &s) { txtPrint(s, 1, -0.99f, -0.99f); }

void SimpleTextMaker::printTopRight(const std::string &s) { txtPrint(s, 2, 0.99f, -0.99f); }

void SimpleTextMaker::printTopCenter(const std::string &s) { txtPrint(s, 3, 0.0f, -0.85f); }

void SimpleTextMaker::printBottomLeft(const std::string &s) { txtPrint(s, 4, -0.99f, 0.99f); }

void SimpleTextMaker::printBottomRight(const std::string &s) { txtPrint(s, 5, 0.99f, 0.99f); }

void SimpleTextMaker::printBottomCenter(const std::string &s) { txtPrint(s, 6, 0.0f, 0.85f); }

void SimpleTextMaker::printCenter(const std::string &s) { txtPrint(s, 7, 0.0f, 0.0f); }
