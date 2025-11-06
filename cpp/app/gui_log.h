#pragma once

#include <string>
#include <vector>

// Log system
struct LogEntry {
  std::string message;
  std::string timestamp;
  enum Type { Info, Success, Error } type;
};

void AddLogEntry(LogEntry::Type type, const std::string& message);
void ShowLogWindow();

