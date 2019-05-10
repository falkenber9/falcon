#pragma once

#include <string>
#include <vector>

class CSV {
public:
  virtual ~CSV() {}
  virtual std::string toCSV(const char delim) const = 0;
  virtual std::string fromCSV(const std::string& str, const char delim) = 0;

  static std::string splitString(const std::string& str,
                                 const char delim,
                                 std::vector<std::string>& tokens,
                                 const int nTokens = 0);
};
