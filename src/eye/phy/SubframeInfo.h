#pragma once

#include "SubframePower.h"
#include "DCICollection.h"

class SubframeInfo {
public:
    SubframeInfo(const srslte_cell_t& cell);
    ~SubframeInfo();
    SubframePower& getSubframePower() { return subframePower; }
    const SubframePower& getSubframePower() const { return subframePower; }
    DCICollection& getDCICollection() { return dciCollection; }
    const DCICollection& getDCICollection() const { return dciCollection; }
private:
    SubframePower subframePower;
    DCICollection dciCollection;
};
