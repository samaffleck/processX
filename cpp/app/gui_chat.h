#pragma once

#include <string>
#include <vector>

// Chat message structure
struct ChatMessage {
  std::string content;
  enum MessageType { User, LLM, LogInfo, LogSuccess, LogError } type;
  std::string timestamp;
};

void ShowChatWindow();
void AddChatMessage(const std::string& message, bool is_user);
void AddLogToChat(const std::string& message, ChatMessage::MessageType log_type);

