#include "falcon/common/CSV.h"

#include <sstream>

using namespace std;

string CSV::splitString(const string& str,
                             const char delim,
                             vector<string>& tokens,
                             const int nTokens)
{
  stringstream src(str);
  string value;
  int i = 0;

  //nTokens = 0: read until end...
  while((i < nTokens || nTokens == 0) && getline(src, value, delim)) {
    tokens.push_back(value);
    i++;
  }

  string rest;
  getline(src, rest);
  return rest;
}
