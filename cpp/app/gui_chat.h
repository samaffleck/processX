#pragma once

#include <string>
#include <vector>

// Chat message structure
struct ChatMessage {
  std::string content;
  enum MessageType { User, LLM, LogInfo, LogSuccess, LogError } type;
  std::string timestamp;
  // Token usage information (only for LLM messages)
  int prompt_tokens = 0;
  int completion_tokens = 0;
  int total_tokens = 0;
};

void ShowChatWindow();
void AddChatMessage(const std::string& message, bool is_user);
void AddLogToChat(const std::string& message, ChatMessage::MessageType log_type);

