#pragma once

//#include <vector>
//#include "falcon/phy/falcon_phch/falcon_dci.h"

//// remove in future version
//struct srslte_dl_sf_cfg_t;
//struct srslte_dci_cfg_t;

//class DCIFormat {
//public:
//    DCIFormat(srslte_cell_t* cell,
//              srslte_dl_sf_cfg_t* sf,
//              srslte_dci_cfg_t* cfg,
//              srslte_dci_format_t format,
//              uint32_t globalIndex);
//    virtual ~DCIFormat();
//    uint32_t getSize() const { return size; }
//    srslte_dci_format_t getFormat() const { return format; }
//    uint32_t getGlobalIndex() const { return globalIndex; }

//private:
//    srslte_dci_format_t format;
//    uint32_t globalIndex;
//    uint32_t size;

//};

//class AbstractDCIFormatSet {
//public:
//    AbstractDCIFormatSet();
//    virtual ~AbstractDCIFormatSet();
//    virtual size_t getNumberOfFormats() const = 0;
//    virtual std::vector<DCIFormat> const& getFormats() const = 0;
//};

//class StaticDCIFormatSubset : public AbstractDCIFormatSet {
//public:
//    StaticDCIFormatSubset(const std::vector<bool> selectionMask);
//private:
//    std::vector<DCIFormat> formatVec;
//};

//class StaticDCIFormatSet : public AbstractDCIFormatSet {
//public:
//    StaticDCIFormatSet(const srslte_dci_format_t* formats, size_t nof_formats);
//    virtual ~StaticDCIFormatSet();
//    StaticDCIFormatSubset getPrimarySubset();
//    StaticDCIFormatSubset getSecondarySubset();
//private:
//    std::vector<DCIFormat> formatVec;
//};
