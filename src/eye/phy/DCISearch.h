#pragma once

#include "falcon/phy/falcon_ue/falcon_ue_dl.h"
#include "SubframeInfo.h"
#include "PhyCommon.h"
#include "MetaFormats.h"

class DCISearch {
public:
    DCISearch(srslte_ue_dl_t& ue_dl,
              const DCIMetaFormats& metaFormats,
              RNTIManager& rntiManager,
              SubframeInfo& subframeInfo,
              uint32_t sf_idx,
              uint32_t sfn);

    int search();
    DCIBlindSearchStats& getStats();

    void setShortcutDiscovery(bool enable);
    bool getShortcutDiscovery() const;
private:
    int inspect_dci_location_recursively(srslte_dci_msg_t *dci_msg,
                                         uint32_t cfi,
                                         falcon_cce_to_dci_location_map_t *cce_map,
                                         falcon_dci_location_t *location_list,
                                         uint32_t ncce,
                                         uint32_t L,
                                         uint32_t max_depth,
                                         falcon_dci_meta_format_t **meta_formats,
                                         uint32_t nof_formats,
                                         uint32_t enable_discovery,
                                         const dci_candidate_t parent_cand[]);
    int recursive_blind_dci_search(srslte_dci_msg_t *dci_msg,
                                   uint32_t cfi);
    //falcon_ue_dl_t& q;
    srslte_ue_dl_t& ue_dl;
    const DCIMetaFormats& metaFormats;
    RNTIManager& rntiManager;
    DCICollection& dciCollection;
    SubframePower& subframePower;
    uint32_t sf_idx;
    uint32_t sfn;
    DCIBlindSearchStats stats;
    bool enableShortcutDiscovery;
};
