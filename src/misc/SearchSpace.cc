#include "falcon/phy/falcon_phch/falcon_pdcch.h"

#include <iostream>

using namespace std;

#define NOF_CCE(cfi)  ((cfi>0&&cfi<4)?q->nof_cce[cfi-1]:0)
#define MAX_CANDIDATES_UE  (16+6) // From 36.213 Table 9.1.1-1 (Common+UE_spec.)

/**
 * @brief main This test function demonstrates that the UE-specific search space function
 * sometimes allows overlapping candidates with different aggregation Level L which start with the same CCE.
 * As a consequence, an accepted DCI with aggregation level L may occasionally be successfully decoded with L+1
 * if FEC fixes the disturbance of the additional 2^L CCE. However, if those additional CCEs contain another DCI,
 * that DCI may be left undiscovered (as the overlapping CCEs are marked as used by the previous DCI).
 * @param argc not used
 * @param argv not used
 * @return not used
 */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    // Cell setup
    uint32_t nof_prb = 100;
    uint32_t nof_rx_ant = 1;

    uint16_t rnti = 0x4711;

    // Initialize objects
    srslte_pdcch_t pdcch;
    srslte_regs_t regs;
    srslte_cell_t cell = {
      .nof_prb = nof_prb,
      .nof_ports = 1,
      .id = 1,
      .cp = SRSLTE_CP_NORM,
      .phich_length = SRSLTE_PHICH_NORM,
      .phich_resources = SRSLTE_PHICH_R_1,
    };

    if (srslte_regs_init(&regs, cell)) {
      fprintf(stderr, "Error initiating regs\n");
      exit(-1);
    }
    if (srslte_pdcch_init_ue(&pdcch, nof_prb, nof_rx_ant)) {
      cerr << "Error creating PDCCH object\n" << endl;
      exit(-1);
    }
    if (srslte_pdcch_set_cell(&pdcch, &regs, cell)) {
      fprintf(stderr, "Error setting cell in PDCCH object\n");
      exit(-1);
    }

    // Generate candidates
    srslte_dci_location_t loc[MAX_CANDIDATES_UE];

    for(uint32_t cfi=1; cfi<4; cfi++) {
        // Derive required parameters
        uint32_t nof_cce = srslte_pdcch_nof_cce(&pdcch, cfi);

        for(uint32_t nsubframe=0; nsubframe<10; nsubframe++) {
            //uint32_t num_candidates = srslte_pdcch_generic_locations_ncce(nof_cce, loc, MAX_CANDIDATES_UE, nsubframe, rnti);
            uint32_t num_candidates = srslte_pdcch_ue_locations_ncce(nof_cce, loc, MAX_CANDIDATES_UE, nsubframe, rnti);

            bool match = false;
            for(uint32_t outer=0; outer < num_candidates; outer++) {
                for(uint32_t inner=outer+1; inner < num_candidates; inner++) {
                    if(loc[outer].ncce == loc[inner].ncce) {
                        cout << "Match:"
                             << " RNTI: " << rnti
                             << " PRB: " << nof_prb
                             << " CFI: " << cfi
                             << " SF:" << nsubframe
                             << " (" << loc[outer].ncce << ", " << loc[outer].L << ") and (" << loc[inner].ncce << ", " << loc[inner].L << "), " << endl;
                        match = true;
                    }
                }
            }

            if(match) {
                uint32_t L_last = 0;
                cout << "Candidates (" << num_candidates << "): (ncce, L)";
                for(uint32_t cand=0; cand < num_candidates; cand++) {
                    if(L_last != loc[cand].L) cout << endl;
                    L_last = loc[cand].L;
                    cout << "(" << loc[cand].ncce << ", " << loc[cand].L << "), ";
                }
                cout << endl;
            }
        }
    }

    return 0;
}

