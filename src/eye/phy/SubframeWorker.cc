#include "SubframeWorker.h"
#include "DCISearch.h"
#include "SubframeInfo.h"

#include "falcon/prof/Lifetime.h"

#include <iostream>

/* Buffers for PCH reception (not included in DL HARQ) */
const static uint32_t  pch_payload_buffer_sz = 8*1024;  // cf. srslte: srsue/hdr/mac/mac.h

SubframeWorker::SubframeWorker(uint32_t idx,
                               uint32_t max_prb,
                               PhyCommon& common,
                               DCIMetaFormats& metaFormats) :
  sfb(common.nof_rx_antennas),
  idx(idx),
  max_prb(max_prb),
  common(common),
  metaFormats(metaFormats),
  sf_idx(0),
  sfn(0),
  updateMetaFormats(false),
  stats()
{
  srslte_ue_dl_init(&ue_dl, sfb.sf_buffer, max_prb, common.nof_rx_antennas);
  for (int i = 0; i< SRSLTE_MAX_CODEWORDS; i++) {
    pch_payload_buffers[i] = new uint8_t[pch_payload_buffer_sz];
    if (!pch_payload_buffers[i]) {
      std::cout << "Error allocating bpch_payload_buffers" << std::endl;
    }
  }
}

SubframeWorker::~SubframeWorker() {
  srslte_ue_dl_free(&ue_dl);
  for (int i = 0; i< SRSLTE_MAX_CODEWORDS; i++) {
    delete[] pch_payload_buffers[i];
    pch_payload_buffers[i] = nullptr;
  }
}

bool SubframeWorker::setCell(srslte_cell_t cell) {
  if (srslte_ue_dl_set_cell(&ue_dl, cell)) {
    return false;
  }
  return true;
}

void SubframeWorker::setRNTI(uint16_t rnti) {
  srslte_ue_dl_set_rnti(&ue_dl, rnti);
}

void SubframeWorker::setChestCFOEstimateEnable(bool enable, uint32_t mask) {
  srslte_chest_dl_cfo_estimate_enable(&ue_dl.chest, enable, mask);
}

void SubframeWorker::setChestAverageSubframe(bool enable) {
  srslte_chest_dl_average_subframe(&ue_dl.chest, enable);
}

void SubframeWorker::prepare(uint32_t sf_idx, uint32_t sfn, bool updateMetaFormats)
{
  this->sf_idx = sf_idx;
  this->sfn = sfn;
  this->updateMetaFormats = updateMetaFormats;
}

void SubframeWorker::work() {
  int n;
  { //PrintLifetime lt("###>> Subframe took: ");
    if(updateMetaFormats) {
      metaFormats.update_formats();
    }
    SubframeInfo subframeInfo(ue_dl.cell);
    DCISearch dciSearch(ue_dl,
                        metaFormats,
                        common.getRNTIManager(),
                        subframeInfo,
                        sf_idx, sfn);
    dciSearch.setShortcutDiscovery(common.getShortcutDiscovery());
    dciSearch.search();
    stats += dciSearch.getStats();  //worker-specific statistics
    common.addStats(dciSearch.getStats());  //common statistics
    common.consumeDCICollection(subframeInfo);
  }
///TODO:
/// optimize here - if current_rnti had been changed, this means that some RA-RNTI was found
/// and current_rnti is set to this RA-RNTI value.
/// The following approach performs once again the full chain fft-blindsearch-dciDecoding-pdschDecoding
/// This can be optimized provide the already decoded DCI and decode pdsch only....
  if (ue_dl.current_rnti != 0xffff) {
    bool acks [SRSLTE_MAX_CODEWORDS] = {false};
    //n = srslte_ue_dl_decode_broad(&ue_dl, &sf_buffer[args.time_offset], data, srslte_ue_sync_get_sfidx(&ue_sync), ue_dl.current_rnti);
    if(ue_dl.cell.nof_ports == 1) {
      /* Transmission mode 1 */
      n = srslte_ue_dl_decode(&ue_dl, pch_payload_buffers, 0, sfn * 10 + sf_idx, acks);
    }
    else {
      /* Transmission mode 2 */
      n = srslte_ue_dl_decode(&ue_dl, pch_payload_buffers, 1, sfn * 10 + sf_idx, acks);
    }
    uint8_t* pch_payload = pch_payload_buffers[0];
    uint16_t t_rnti;

    switch(n) {
      case 0:
        //        			printf("No decode\n");
        break;
      case 40:
        t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-2]) + pch_payload[(n/8)-1];
        //srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
        common.getRNTIManager().activateAndRefresh(t_rnti, 0, ActivationReason::RM_ACT_RAR);
        //rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);
        //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-2] + pch_payload[(n/8)-1]);
        //        			for (int k=0; k<(n/8); k++) {
        //        				printf("%02x ",pch_payload[k]);
        //        			}
        //        			printf("\n");
        break;
      case 56:
        t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-2]) + pch_payload[(n/8)-1];
        //srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
        common.getRNTIManager().activateAndRefresh(t_rnti, 0, ActivationReason::RM_ACT_RAR);
        //rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);
        //        			printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-2] + pch_payload[(n/8)-1]);
        //        			for (int k=0; k<(n/8); k++) {
        //        				printf("%02x ",pch_payload[k]);
        //        			}
        //        			printf("\n");
        break;
      case 72:
        t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-4]) + pch_payload[(n/8)-3];
        //srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
        common.getRNTIManager().activateAndRefresh(t_rnti, 0, ActivationReason::RM_ACT_RAR);
        //rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);
        //        			printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-4] + pch_payload[(n/8)-3]);
        //        			for (int k=0; k<(n/8); k++) {
        //        				printf("%02x ",pch_payload[k]);
        //        			}
        //        			printf("\n");
        break;
      case 120:
        t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-3]) + pch_payload[(n/8)-2];
        //srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
        common.getRNTIManager().activateAndRefresh(t_rnti, 0, ActivationReason::RM_ACT_RAR);
        //rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);

        t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-9]) + pch_payload[(n/8)-8];
        //srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
        common.getRNTIManager().activateAndRefresh(t_rnti, 0, ActivationReason::RM_ACT_RAR);
        //rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);
        //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-9] + pch_payload[(n/8)-8]);
        //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-3] + pch_payload[(n/8)-2]);
        //					for (int k=0; k<(n/8); k++) {
        //						printf("%02x ",pch_payload[k]);
        //					}
        //					printf("\n");
        break;
      default:
        //        			fprintf(stderr,"\n");
        //					for (int k=0; k<(n/8); k++) {
        //						fprintf(stderr,"%02x ",pch_payload[k]);
        //					}
        //					fprintf(stderr,"\n");
        break;
    }
  }
}

void SubframeWorker::printStats() {
  stats.print(common.getStatsFile());
}

DCIBlindSearchStats& SubframeWorker::getStats() {
  return stats;
}

cf_t* SubframeWorker::getBuffer(uint32_t antenna_idx) {
  return sfb.sf_buffer[antenna_idx];
}
