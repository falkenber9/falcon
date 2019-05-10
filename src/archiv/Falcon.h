
#include "lte/CCSniffer.h"

class Falcon {

public:
  virtual ~Falcon();
  static Falcon& instance();

private:

  CCSniffer* ccSniffer = NULL;

  Falcon();
  Falcon(const Falcon&);
  Falcon& operator=(const Falcon&);

};
