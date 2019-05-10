#include "SpectrumAdapter.h"


void SpectrumAdapter::update() {
    // currently, only push() is implemented

    emit signal_update();

    // emit signals to gui here
}

void SpectrumAdapter::push(const ScanLineLegacy* data) {
    //ScanLine* scanline = dynamic_cast<ScanLine*>(data);

    switch (data->type) {
    case(ScanLineType_t::SCAN_LINE_UPLINK):
        if(emit_uplink) emit update_ul(data);
        else delete data;
        break;
    case(ScanLineType_t::SCAN_LINE_DOWNLINK):
        if(emit_downlink) emit update_dl(data);
        else delete data;
        break;
    case(ScanLineType_t::SCAN_LINE_SPECTRUM):
        if(emit_spectrum) emit update_spectrum(data);
        else delete data;
        break;
    case(ScanLineType_t::SCAN_LINE_SPECTRUM_DIFF):
        if(emit_difference) emit update_spectrum_diff(data);
        else delete data;
        break;
    case(ScanLineType_t::SCAN_LINE_RNTI_HIST):
        if(emit_plot1) emit update_rnti_hist(data);
        else delete data;
        break;
    }
}

void SpectrumAdapter::notifyDetach() {

}
