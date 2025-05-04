#pragma once

#include <fstream>
#include <optional>
#include <regex>
#include <string>
#include <system_error>

class ErrorHandler {
  public:
    ErrorHandler() noexcept = default;
    ErrorHandler(int argc, char* argv[]);
    std::optional<std::error_code> tryOpenFile(const std::ifstream& file) const;
    std::optional<std::error_code> workingTimeChecker(const std::string& str) const;
    std::optional<std::error_code> eventChecker(const std::string& eventLine) const;
};
