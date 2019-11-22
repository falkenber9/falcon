#pragma once

#include "eye/ArgManager.h"
#include "falcon/CCSnifferInterfaces.h"
#include "falcon/definitions.h"
#include "eye/phy/SubframeInfoConsumer.h"

#undef I // Fix complex.h #define I nastiness when using C++
#include <boost/thread.hpp>
#include "falcon/util/RNTIManager.h"

#define SPECTROGRAM_INTERVAL_US 10000
#define SPECTROGRAM_MAX_LINE_WIDTH 100

// Forward declaration to keep it opaque
class EyeCore;

class EyeThread :
        public Provider<ScanLineLegacy> {
public:
    EyeThread() :
        Provider<ScanLineLegacy>(),
        cancel(false),
        initialized(false),
        scanline_width(SPECTROGRAM_MAX_LINE_WIDTH),
        eye(NULL),
        theThread(NULL)
    {}
    virtual ~EyeThread();

    void init();
    void start(const Args& args);
    void stop();
    bool isInitialized();
    void test();
    inline int getScanLineWidth() {return scanline_width;};
    void attachConsumer(std::shared_ptr<SubframeInfoConsumer> consumer);
    RNTIManager &getRNTIManager();

private:
    void run();
    volatile bool cancel;
    bool initialized;
    int scanline_width;
    EyeCore* eye;
    boost::thread* theThread;
    std::shared_ptr<SubframeInfoConsumer> m_consumer = nullptr;
};

class DCIGUIConsumer : public SubframeInfoConsumer {
public:
    DCIGUIConsumer();
    DCIGUIConsumer(EyeThread &p_Thread);
    void setThread(EyeThread &p_Thread);
    virtual void consumeDCICollection(const SubframeInfo& subframeInfo) override;
private:
    FILE *dci_file;
    EyeThread *m_Thread = nullptr;
    uint32_t cfi_tmp[65536];
};
