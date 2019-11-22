#pragma once

#include "falcon/phy/falcon_ue/falcon_ue_dl.h"
#include <vector>
#include <memory>
#include <string>

#define EXPECTED_NOF_DL_DCI_PER_SUBFRAME 10
#define EXPECTED_NOF_UL_DCI_PER_SUBFRAME 10
#define MAX_NOF_DL_PRB 100
#define MAX_NOF_UL_PRB 100

struct DCI_BASE {
    DCI_BASE();
    uint16_t rnti;
    srslte_dci_format_t format;
    uint32_t nof_bits;
    std::string hex;
    srslte_dci_location_t location;
    uint32_t histval;
};

struct DCI_DL : public DCI_BASE {
    DCI_DL();
    std::unique_ptr<srslte_ra_dl_dci_t> dl_dci_unpacked;
    std::unique_ptr<srslte_ra_dl_grant_t> dl_grant;
};

struct DCI_UL : public DCI_BASE {
    DCI_UL();
    std::unique_ptr<srslte_ra_ul_dci_t> ul_dci_unpacked;
    std::unique_ptr<srslte_ra_ul_grant_t> ul_grant;
};

/**
 * @brief The DCICollection class is a container for buffering DCI of a subframe
 */
class DCICollection {

public:
    DCICollection(const srslte_cell_t& cell);
    ~DCICollection();
    void setTimestamp(struct timeval timestamp);
    timeval getTimestamp() const;
    void setSubframe(uint32_t sfn, uint32_t sf_idx, uint32_t cfi);
    uint32_t get_sfn() const;
    uint32_t get_sf_idx() const;
    uint32_t get_cfi() const;
    const std::vector<DCI_DL>& getDCI_DL() const;
    const std::vector<DCI_UL>& getDCI_UL() const;
    const std::vector<uint16_t>& getRBMapDL() const;
    const std::vector<uint16_t>& getRBMapUL() const;
    bool hasCollisionDL() const;
    bool hasCollisionUL() const;
    void addCandidate(dci_candidate_t& cand,
                      const srslte_dci_location_t& location,
                      uint32_t histval);
    static std::vector<uint16_t> applyLegacyColorMap(const std::vector<uint16_t>& RBMap);
private:
    srslte_cell_t cell;
    uint32_t sfn;
    uint32_t sf_idx;
    uint32_t cfi;
    struct timeval timestamp;
    std::vector<DCI_DL> dci_dl_collection;
    std::vector<DCI_UL> dci_ul_collection;
    std::vector<uint16_t> rb_map_dl;
    std::vector<uint16_t> rb_map_ul;
    bool dl_collision;
    bool ul_collision;
};
