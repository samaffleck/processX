#include "gui_chat.h"
#include "gui_common.h"
#include "gui_log.h"
#include <imgui.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstring>

#ifdef EMSCRIPTEN
#include <emscripten.h>

// Global storage for LLM response (simplified approach)
static std::string pending_llm_message;
static std::string pending_llm_json;
static bool llm_response_ready = false;

// Callback function to be called from JavaScript
extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void on_llm_response_received(const char* message, const char* simulation_json) {
    if (message) {
      pending_llm_message = message;
      if (simulation_json) {
        pending_llm_json = simulation_json;
      } else {
        pending_llm_json = "{}";
      }
      llm_response_ready = true;
    }
  }
}

// EM_JS wrapper for LLM API call
// Expects window.callLLMAPI(message, flowsheetJson) to return Promise<{message: string, simulationJson: string}>
EM_JS(void, call_llm_api, (const char* user_message, const char* flowsheet_json), {
  const message = UTF8ToString(user_message);
  const json = UTF8ToString(flowsheet_json);
  
  // Define callLLMAPI inline if not already defined (fallback implementation)
  if (typeof window.callLLMAPI !== 'function') {
    window.callLLMAPI = async function(msg, flowsheetJson) {
      try {
        // Parse flowsheetJson if it's a string
        let jsonData = null;
        if (flowsheetJson && typeof flowsheetJson === 'string' && flowsheetJson.length > 0) {
          try {
            jsonData = JSON.parse(flowsheetJson);
          } catch (e) {
            console.warn('[callLLMAPI] Failed to parse flowsheetJson:', e);
            jsonData = null;
          }
        } else if (flowsheetJson && typeof flowsheetJson === 'object') {
          jsonData = flowsheetJson;
        }

        // Call the Next.js API endpoint
        const response = await fetch('/api/chat', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({
            message: msg,
            jsonData: jsonData,
            conversationHistory: [],
          }),
        });

        if (!response.ok) {
          throw new Error('API request failed: ' + response.status + ' ' + response.statusText);
        }

        const data = await response.json();

        if (data.error) {
          var errorMsg = 'Error: ' + data.error;
          if (data.details) {
            errorMsg += '\n\n' + data.details;
          }
          return {
            message: errorMsg,
            simulationJson: '{}'
          };
        }

        // Extract text response and JSON
        let responseMessage = data.response || 'No response received';
        let simulationJson = '{}';

        // If there's updated JSON, include it
        if (data.hasJsonUpdate && data.editedJson) {
          simulationJson = JSON.stringify(data.editedJson);
        }

        return {
          message: responseMessage,
          simulationJson: simulationJson
        };
      } catch (error) {
        console.error('[callLLMAPI] Error:', error);
        return {
          message: 'Error: ' + (error.message || 'Failed to get response from AI'),
          simulationJson: '{}'
        };
      }
    };
  }
  
  // Call the LLM API function
  window.callLLMAPI(message, json)
    .then(result => {
      const responseMessage = result.message || "No response";
      const simulationJson = result.simulationJson || "{}";
      
      // Call the C++ callback function
      Module._on_llm_response_received(
        Module.stringToNewUTF8(responseMessage),
        Module.stringToNewUTF8(simulationJson)
      );
    })
    .catch(error => {
      const errorMsg = "Error: " + error.toString();
      Module._on_llm_response_received(
        Module.stringToNewUTF8(errorMsg),
        Module.stringToNewUTF8("{}")
      );
    });
});

#endif

static std::vector<ChatMessage> chat_messages;
static char input_buffer[1024] = "";
static constexpr size_t MAX_CHAT_MESSAGES = 1000;

void AddChatMessage(const std::string& message, bool is_user) {
  // Get current time
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    now.time_since_epoch()) % 1000;
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
  ss << "." << std::setfill('0') << std::setw(3) << ms.count();
  
  ChatMessage chat_msg;
  chat_msg.content = message;
  chat_msg.type = is_user ? ChatMessage::User : ChatMessage::LLM;
  chat_msg.timestamp = ss.str();
  
  chat_messages.push_back(chat_msg);
  
  // Limit chat size
  if (chat_messages.size() > MAX_CHAT_MESSAGES) {
    chat_messages.erase(chat_messages.begin());
  }
  
}

void AddLogToChat(const std::string& message, ChatMessage::MessageType log_type) {
  // Get current time
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    now.time_since_epoch()) % 1000;
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
  ss << "." << std::setfill('0') << std::setw(3) << ms.count();
  
  ChatMessage chat_msg;
  chat_msg.content = message;
  chat_msg.type = log_type;
  chat_msg.timestamp = ss.str();
  
  chat_messages.push_back(chat_msg);
  
  // Limit chat size
  if (chat_messages.size() > MAX_CHAT_MESSAGES) {
    chat_messages.erase(chat_messages.begin());
  }
  
}

void ShowChatWindow() {
  // Check for LLM response (Emscripten only)
  #ifdef EMSCRIPTEN
  if (llm_response_ready) {
    // Remove the "Thinking..." message if it exists
    if (!chat_messages.empty() && chat_messages.back().type == ChatMessage::LLM && 
        chat_messages.back().content == "Thinking...") {
      chat_messages.pop_back();
    }
    
    // Add the actual response
    AddChatMessage(pending_llm_message, false);
    
    // Process and load flowsheet JSON if provided
    if (pending_llm_json.length() > 2) { // More than just "{}"
      bool load_success = LoadFlowsheetFromJSONString(pending_llm_json);
      if (load_success) {
        AddLogToChat("Flowsheet updated from LLM response", ChatMessage::LogSuccess);
      } else {
        AddLogToChat("Failed to load flowsheet from LLM response", ChatMessage::LogError);
      }
    }
    
    // Reset flag
    llm_response_ready = false;
    pending_llm_message.clear();
    pending_llm_json.clear();
  }
  #endif
  
  ImGui::Begin("Chat");
  
  // Solve and Clear buttons at the top
  if (ImGui::Button("Solve")) {
    AddLogToChat("Starting solve...", ChatMessage::LogInfo);
    
    // Set up logging callback to capture CoolProp exceptions
    flowsheet.set_log_callback([](const std::string& message, bool is_error) {
      AddLogToChat(message, is_error ? ChatMessage::LogError : ChatMessage::LogInfo);
    });
    
    // First, assemble the system
    std::string assemble_error;
    if (!flowsheet.assemble(&assemble_error)) {
      AddLogToChat("Assembly failed: " + assemble_error, ChatMessage::LogError);
    } else {
      AddLogToChat("System assembled successfully", ChatMessage::LogInfo);
      
      // Configure solver options
      px::NewtonOptions options;
      options.max_iters = 50;
      options.tol_res = 1e-10;
      options.tol_step = 1e-12;
      options.fd_rel = 1e-6;
      options.fd_abs = 1e-8;
      options.verbose = false; // We'll log ourselves
      
      // Run solver
      px::NewtonReport report = flowsheet.solve(options);
      
      if (report.converged) {
        std::string success_msg = "Solve converged! Iterations: " + std::to_string(report.iters) 
                                  + ", Final residual: " + std::to_string(report.final_res);
        AddLogToChat(success_msg, ChatMessage::LogSuccess);
        if (!report.msg.empty()) {
          AddLogToChat("Message: " + report.msg, ChatMessage::LogInfo);
        }
      } else {
        std::string error_msg = "Solve failed: " + report.msg;
        if (report.iters > 0) {
          error_msg += " (Iterations: " + std::to_string(report.iters) 
                      + ", Final residual: " + std::to_string(report.final_res) + ")";
        }
        AddLogToChat(error_msg, ChatMessage::LogError);
      }
    }
    
    // Clear the logging callback after solve completes
    flowsheet.clear_log_callback();
  }
  ImGui::SameLine();
  if (ImGui::Button("Clear")) {
    chat_messages.clear();
  }
  
  ImGui::Separator();
  
  // Calculate available height for messages (leave space for input area)
  float input_height = ImGui::GetFrameHeightWithSpacing() * 2.5f; // Input + button + spacing
  float messages_height = ImGui::GetContentRegionAvail().y - input_height;
  
  // Messages area - remove horizontal scrollbar, text will wrap
  // Use same background as window (no different background color)
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
  ImGui::BeginChild("ChatMessages", ImVec2(0, messages_height), false);
  ImGui::PopStyleColor();
  
  for (const auto& msg : chat_messages) {
    // Style messages based on type
    ImVec4 color;
    const char* prefix = "";
    switch (msg.type) {
      case ChatMessage::User:
        color = ImVec4(0.2f, 0.6f, 1.0f, 1.0f); // Blue for user
        prefix = "You";
        break;
      case ChatMessage::LLM:
        color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Light gray for LLM
        prefix = "Assistant";
        break;
      case ChatMessage::LogInfo:
        color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White for info
        prefix = "Log";
        break;
      case ChatMessage::LogSuccess:
        color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green for success
        prefix = "Log";
        break;
      case ChatMessage::LogError:
        color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for error
        prefix = "Log";
        break;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    if (msg.type == ChatMessage::User || msg.type == ChatMessage::LLM) {
      ImGui::TextWrapped("[%s] %s: %s", msg.timestamp.c_str(), prefix, msg.content.c_str());
    } else {
      ImGui::TextWrapped("[%s] %s", msg.timestamp.c_str(), msg.content.c_str());
    }
    ImGui::PopStyleColor();
    ImGui::Spacing();
  }
  
  // Force auto-scroll to bottom (always enabled)
  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) {
    ImGui::SetScrollHereY(1.0f);
  } else {
    ImGui::SetScrollY(ImGui::GetScrollMaxY());
  }
  
  ImGui::EndChild();
  
  ImGui::Separator();
  
  // Input area at bottom - original style: single line input, button below
  ImGui::PushItemWidth(-1);
  
  // Use InputTextWithHint for placeholder text
  bool input_activated = ImGui::InputTextWithHint("##ChatInput", "Ask something", input_buffer, sizeof(input_buffer),
                                                   ImGuiInputTextFlags_EnterReturnsTrue);
  ImGui::PopItemWidth();
  
  // Check if Enter was pressed to submit (InputTextWithHint already handles Enter key)
  
  // Send button below the input box
  ImGui::BeginDisabled(strlen(input_buffer) == 0);
  bool submit_clicked = ImGui::Button("Send", ImVec2(-1, 0));
  ImGui::EndDisabled();
  
  // Handle submit (Enter key or button click)
  if ((input_activated || submit_clicked) && strlen(input_buffer) > 0) {
    std::string user_message = input_buffer;
    
    // Add user message to chat
    AddChatMessage(user_message, true);
    
    // Clear input
    memset(input_buffer, 0, sizeof(input_buffer));
    
    // Call LLM API (only in Emscripten builds)
    #ifdef EMSCRIPTEN
    // Get current flowsheet JSON
    std::string flowsheet_json = GetFlowsheetJSONString();
    
    // Call the EM_JS function with both message and JSON
    call_llm_api(user_message.c_str(), flowsheet_json.c_str());
    
    // Add a "thinking..." message (will be replaced when response arrives)
    AddChatMessage("Thinking...", false);
    #else
    // Non-Emscripten: just echo the message back as a placeholder
    AddChatMessage("Echo: " + user_message + " (LLM API only available in WASM build)", false);
    #endif
    
    // Focus back on input
    ImGui::SetKeyboardFocusHere(-1);
  }
  
  ImGui::End();
}

