#include "falcon/common/FileSink.h" 

template <>
FileSink<float>::FileSink() : isOpen(false), type(SRSLTE_FLOAT_BIN) { }

template <>
FileSink<_Complex float>::FileSink() : isOpen(false), type(SRSLTE_COMPLEX_FLOAT_BIN) { }
