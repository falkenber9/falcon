#include "SubframeInfo.h"

SubframeInfo::SubframeInfo(const srslte_cell_t& cell) :
  subframePower(cell),
  dciCollection(cell)
{

}

SubframeInfo::~SubframeInfo() {

}
