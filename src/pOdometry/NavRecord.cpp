#include "NavRecord.h"

NavRecord::NavRecord() {
    m_x = 0;
    m_y = 0;
    m_depth = 0;
}

void NavRecord::setX(double value) {
    m_x = value;
}

void NavRecord::setY(double value) {
    m_y = value;
}

void NavRecord::setDepth(double value) {
    m_depth = value;
}

