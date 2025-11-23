#include "gui_chat.h"
#include "gui_common.h"
#include "gui_log.h"
#include <imgui.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <cmath>

#ifdef EMSCRIPTEN
#include <emscripten.h>

// Global storage for LLM response
static std::string pending_llm_message;
static std::string pending_llm_json;
static bool llm_response_ready = false;
static bool llm_should_solve = false; // Flag to trigger solve
static int pending_prompt_tokens = 0;
static int pending_completion_tokens = 0;
static int pending_total_tokens = 0;

// Update the callback function
extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void on_llm_response_received(const char* message, const char* simulation_json, int should_solve, 
                                 int prompt_tokens, int completion_tokens, int total_tokens) {
    if (message) {
      pending_llm_message = message;
      if (simulation_json) {
        pending_llm_json = simulation_json;
      } else {
        pending_llm_json = "{}";
      }
      llm_should_solve = (should_solve != 0);
      
      // Store token usage values directly
      pending_prompt_tokens = prompt_tokens;
      pending_completion_tokens = completion_tokens;
      pending_total_tokens = total_tokens;
      
      llm_response_ready = true;
    }
  }
}

// Update the EM_JS wrapper
EM_JS(void, call_llm_api, (const char* user_message, const char* flowsheet_json, const char* recent_errors), {
  const message = UTF8ToString(user_message);
  const json = UTF8ToString(flowsheet_json);
  const errors = UTF8ToString(recent_errors);
  
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

        // Get PDF context if available (set by React app when PDF is uploaded)
        const pdfContext = window.currentPdfContext || '';

        // Parse errors if provided
        let errorMessages = null;
        if (errors && errors.length > 0) {
          try {
            errorMessages = JSON.parse(errors);
          } catch (e) {
            console.warn('[callLLMAPI] Failed to parse error messages:', e);
            errorMessages = null;
          }
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
            pdfContext: pdfContext,
            recentErrors: errorMessages,
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
            simulationJson: '{}',
            shouldSolve: false
          };
        }

        // Extract text response, JSON, solve flag, and token usage
        let responseMessage = data.response || 'No response received';
        let simulationJson = '{}';
        let shouldSolve = data.shouldSolve || false;
        let tokenUsage = data.tokenUsage || null;

        // If there's updated JSON, include it
        if (data.hasJsonUpdate && data.editedJson) {
          simulationJson = JSON.stringify(data.editedJson);
        }

        return {
          message: responseMessage,
          simulationJson: simulationJson,
          shouldSolve: shouldSolve,
          tokenUsage: tokenUsage
        };
      } catch (error) {
        console.error('[callLLMAPI] Error:', error);
        return {
          message: 'Error: ' + (error.message || 'Failed to get response from AI'),
          simulationJson: '{}',
          shouldSolve: false,
          tokenUsage: null
        };
      }
    };
  }
  
  // Call the LLM API function
  window.callLLMAPI(message, json)
    .then(result => {
      const responseMessage = result.message || "No response";
      const simulationJson = result.simulationJson || "{}";
      const shouldSolve = result.shouldSolve ? 1 : 0;
      const tokenUsage = result.tokenUsage || null;
      
      // Parse token usage and extract individual values
      let promptTokens = 0;
      let completionTokens = 0;
      let totalTokens = 0;
      
      if (tokenUsage) {
        promptTokens = tokenUsage.prompt_tokens || 0;
        completionTokens = tokenUsage.completion_tokens || 0;
        totalTokens = tokenUsage.total_tokens || 0;
      }
      
      // Call the C++ callback function with solve flag and token usage
      Module._on_llm_response_received(
        Module.stringToNewUTF8(responseMessage),
        Module.stringToNewUTF8(simulationJson),
        shouldSolve,
        promptTokens,
        completionTokens,
        totalTokens
      );
    })
    .catch(error => {
      const errorMsg = "Error: " + error.toString();
      Module._on_llm_response_received(
        Module.stringToNewUTF8(errorMsg),
        Module.stringToNewUTF8("{}"),
        0,
        0,
        0,
        0
      );
    });
});

#endif

static std::vector<ChatMessage> chat_messages;
static char input_buffer[1024] = "";
static constexpr size_t MAX_CHAT_MESSAGES = 1000;

// Helper function to escape JSON string
static std::string escape_json_string(const std::string& str) {
  std::ostringstream oss;
  for (char c : str) {
    switch (c) {
      case '"': oss << "\\\""; break;
      case '\\': oss << "\\\\"; break;
      case '\b': oss << "\\b"; break;
      case '\f': oss << "\\f"; break;
      case '\n': oss << "\\n"; break;
      case '\r': oss << "\\r"; break;
      case '\t': oss << "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          // Control character - escape as \uXXXX
          oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') 
              << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
        } else {
          oss << c;
        }
        break;
    }
  }
  return oss.str();
}

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
  // Track previous message count for auto-scroll detection
  static size_t previous_message_count = 0;
  
  // Check for LLM response (Emscripten only)
  #ifdef EMSCRIPTEN
  if (llm_response_ready) {
    // Remove the "Thinking..." message if it exists
    if (!chat_messages.empty() && chat_messages.back().type == ChatMessage::LLM && 
        chat_messages.back().content == "Thinking...") {
      chat_messages.pop_back();
    }
    
    // Add the actual response with token usage
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    ChatMessage chat_msg;
    chat_msg.content = pending_llm_message;
    chat_msg.type = ChatMessage::LLM;
    chat_msg.timestamp = ss.str();
    chat_msg.prompt_tokens = pending_prompt_tokens;
    chat_msg.completion_tokens = pending_completion_tokens;
    chat_msg.total_tokens = pending_total_tokens;
    
    chat_messages.push_back(chat_msg);
    
    // Limit chat size
    if (chat_messages.size() > MAX_CHAT_MESSAGES) {
      chat_messages.erase(chat_messages.begin());
    }
    
    // Process and load flowsheet JSON if provided
    if (pending_llm_json.length() > 2) { // More than just "{}"
      bool load_success = LoadFlowsheetFromJSONString(pending_llm_json);
      if (load_success) {
        AddLogToChat("Flowsheet updated from LLM response", ChatMessage::LogSuccess);
      } else {
        AddLogToChat("Failed to load flowsheet from LLM response", ChatMessage::LogError);
      }
    }
    
    // Trigger solve if requested by LLM
    if (llm_should_solve) {
      AddLogToChat("Starting solve (triggered by LLM)...", ChatMessage::LogInfo);
      
      // Set up logging callback
      flowsheet.set_log_callback([](const std::string& message, bool is_error) {
        AddLogToChat(message, is_error ? ChatMessage::LogError : ChatMessage::LogInfo);
      });
      
      // Assemble the system
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
        options.verbose = false;
        
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
      
      // Clear the logging callback
      flowsheet.clear_log_callback();
    }
    
    // Reset flags
    llm_response_ready = false;
    llm_should_solve = false;
    pending_llm_message.clear();
    pending_llm_json.clear();
    pending_prompt_tokens = 0;
    pending_completion_tokens = 0;
    pending_total_tokens = 0;
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
    // Reset message count tracking when clearing
    previous_message_count = 0;
  }
  
  ImGui::Separator();
  
  // Account for status bar at the bottom of the viewport
  // Status bar height is approximately 1.0 * frame height + padding
  float status_bar_height = ImGui::GetFrameHeight() * 1.0f + ImGui::GetStyle().FramePadding.y * 2.0f;
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  
  // Get window position and size
  ImVec2 window_pos = ImGui::GetWindowPos();
  ImVec2 window_size = ImGui::GetWindowSize();
  float window_bottom = window_pos.y + window_size.y;
  float viewport_bottom = viewport->Pos.y + viewport->Size.y;
  
  // Calculate how much of the window extends into the status bar area
  float reserved_bottom_space = 0.0f;
  if (window_bottom > viewport_bottom - status_bar_height) {
    // Window extends into status bar area
    float overlap = window_bottom - (viewport_bottom - status_bar_height);
    if (overlap > 0.0f) {
      reserved_bottom_space = overlap;
    }
  }
  
  // Calculate input box height first (before creating messages area)
  float line_height = ImGui::GetTextLineHeight();
  float available_width = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2.0f;
  
  int explicit_lines = 1; // Count explicit newlines
  for (size_t i = 0; i < strlen(input_buffer); ++i) {
    if (input_buffer[i] == '\n') {
      explicit_lines++;
    }
  }
  
  // Estimate wrapped lines based on text length and available width
  if (strlen(input_buffer) > 0 && available_width > 0) {
    ImVec2 text_size = ImGui::CalcTextSize("M"); // Average character width
    float avg_char_width = text_size.x;
    float estimated_chars_per_line = available_width / avg_char_width;
    
    if (estimated_chars_per_line > 0) {
      size_t char_count = strlen(input_buffer);
      int estimated_wrapped_lines = static_cast<int>(std::ceil(char_count / estimated_chars_per_line));
      explicit_lines = std::max(explicit_lines, estimated_wrapped_lines);
    }
  }
  
  // Calculate actual input box height
  float input_box_height = line_height + ImGui::GetStyle().FramePadding.y * 2.0f; // Base height
  input_box_height += (explicit_lines - 1) * line_height; // Additional lines
  
  // Clamp height between min and max
  float min_height = line_height + ImGui::GetStyle().FramePadding.y * 2.0f;
  float max_height = line_height * 6.0f + ImGui::GetStyle().FramePadding.y * 2.0f; // Max 6 lines
  input_box_height = std::clamp(input_box_height, min_height, max_height);
  
  // Calculate space needed for input area (input box + send button + spacing)
  float send_button_height = ImGui::GetFrameHeight();
  float input_area_height = input_box_height + send_button_height + ImGui::GetStyle().ItemSpacing.y;
  
  // Calculate available height for messages (using actual input area height)
  // Also account for status bar if window is at the bottom
  float messages_height = ImGui::GetContentRegionAvail().y - input_area_height - reserved_bottom_space;
  messages_height = std::max(messages_height, 50.0f); // Minimum height for messages area
  
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
      
      // Display token usage for LLM messages
      if (msg.type == ChatMessage::LLM && msg.total_tokens > 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f)); // Gray for token info
        ImGui::Text("  Tokens: %d prompt + %d completion = %d total", 
                    msg.prompt_tokens, msg.completion_tokens, msg.total_tokens);
        ImGui::PopStyleColor();
      }
    } else {
      ImGui::TextWrapped("[%s] %s", msg.timestamp.c_str(), msg.content.c_str());
    }
    ImGui::PopStyleColor();
    ImGui::Spacing();
  }
  
  // Auto-scroll to bottom only when new messages are added and user is at bottom
  size_t current_message_count = chat_messages.size();
  bool new_messages_added = current_message_count > previous_message_count;
  previous_message_count = current_message_count;
  
  // Check if user is at bottom (within 1 pixel tolerance)
  bool is_at_bottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;
  
  // Only auto-scroll if new messages were added AND user is already at bottom
  if (new_messages_added && is_at_bottom) {
    ImGui::SetScrollHereY(1.0f);
  }
  
  ImGui::EndChild();
  
  ImGui::Separator();
  
  // Input area at bottom - multiline input that wraps and expands
  // Note: input_box_height was already calculated above for the messages area sizing
  ImGui::PushItemWidth(-1);
  
  // Use InputTextMultiline for wrapping text
  // Note: InputTextMultiline doesn't support hint, so we'll draw it manually
  bool input_activated = ImGui::InputTextMultiline(
    "##ChatInput",
    input_buffer,
    sizeof(input_buffer),
    ImVec2(-1, input_box_height),
    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_WordWrap
  );
  
  // Draw placeholder text when buffer is empty
  if (strlen(input_buffer) == 0 && !ImGui::IsItemActive()) {
    ImVec2 pos = ImGui::GetItemRectMin();
    pos.x += ImGui::GetStyle().FramePadding.x;
    pos.y += ImGui::GetStyle().FramePadding.y;
    ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), "Ask something");
  }
  
  ImGui::PopItemWidth();
  
  // Send button below the input box
  ImGui::BeginDisabled(strlen(input_buffer) == 0);
  bool submit_clicked = ImGui::Button("Send", ImVec2(-1, 0));
  ImGui::EndDisabled();
  
  // Handle submit (Enter key or button click, but not Ctrl+Enter)
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
    
    // Extract recent error messages - prioritize the MOST RECENT errors
    // Look through all messages backwards (most recent first) to find errors
    std::vector<std::string> recent_errors;
    const size_t max_errors_to_capture = 10; // Limit to avoid overwhelming LLM
    
    // Search backwards through messages (most recent first)
    for (int i = static_cast<int>(chat_messages.size()) - 1; 
         i >= 0 && recent_errors.size() < max_errors_to_capture; 
         --i) {
      const auto& msg = chat_messages[static_cast<size_t>(i)];
      if (msg.type == ChatMessage::LogError) {
        // Insert at beginning to maintain reverse chronological order
        // (most recent first, but we'll reverse later to show oldest first)
        recent_errors.insert(recent_errors.begin(), msg.content);
      }
    }
    
    // Reverse to show oldest error first, most recent last (helps LLM understand progression)
    std::reverse(recent_errors.begin(), recent_errors.end());

    // DEBUG: Log error count
    EM_ASM({
      console.log('[C++ DEBUG] Captured ' + $0 + ' error messages from chat history (total messages: ' + $1 + ')');
    }, recent_errors.size(), chat_messages.size());

    // Serialize the errors as JSON array with proper escaping
    std::string errors_json = "[]";
    if (!recent_errors.empty()) {
      std::ostringstream oss;
      oss << "[";
      for (size_t i = 0; i < recent_errors.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << escape_json_string(recent_errors[i]) << "\"";
      }
      oss << "]";
      errors_json = oss.str();
    }
    
    // Call the EM_JS function with message, JSON, and errors
    call_llm_api(user_message.c_str(), flowsheet_json.c_str(), errors_json.c_str());
    
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

