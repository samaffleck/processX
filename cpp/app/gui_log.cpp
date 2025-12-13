#include "gui_log.h"
#include "gui_common.h"
#include "gui_chat.h"
#include "gui_window_titles.h"
#include <imgui.h>
#include <chrono>
#include <sstream>
#include <iomanip>

static std::vector<LogEntry> log_messages;

void AddLogEntry(LogEntry::Type type, const std::string& message) {
  // Get current time
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    now.time_since_epoch()) % 1000;
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
  ss << "." << std::setfill('0') << std::setw(3) << ms.count();
  
  // Create log entry
  LogEntry entry;
  entry.message = message;
  entry.type = type;
  entry.timestamp = ss.str();
  
  // Add to log messages
  log_messages.push_back(entry);
  
  // Convert LogEntry::Type to ChatMessage::MessageType and add to chat
  ChatMessage::MessageType chat_type;
  switch (type) {
    case LogEntry::Info:
      chat_type = ChatMessage::LogInfo;
      break;
    case LogEntry::Success:
      chat_type = ChatMessage::LogSuccess;
      break;
    case LogEntry::Error:
      chat_type = ChatMessage::LogError;
      break;
  }
  AddLogToChat(message, chat_type);
}

void ShowLogWindow() {
  ImGui::Begin(WindowTitles::Log.c_str());
  
  // Auto-scroll to bottom if scrolled to bottom before
  static bool auto_scroll = true;
  
  // Options and Solve button
  if (ImGui::Button("Solve")) {
    AddLogEntry(LogEntry::Info, "Starting solve...");
    
    // Set up logging callback to capture CoolProp exceptions
    flowsheet.set_log_callback([](const std::string& message, bool is_error) {
      AddLogEntry(is_error ? LogEntry::Error : LogEntry::Info, message);
    });
    
    // First, assemble the system
    std::string assemble_error;
    if (!flowsheet.assemble(&assemble_error)) {
      AddLogEntry(LogEntry::Error, "Assembly failed: " + assemble_error);
    } else {
      AddLogEntry(LogEntry::Info, "System assembled successfully");
      
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
        AddLogEntry(LogEntry::Success, success_msg);
        if (!report.msg.empty()) {
          AddLogEntry(LogEntry::Info, "Message: " + report.msg);
        }
      } else {
        std::string error_msg = "Solve failed: " + report.msg;
        if (report.iters > 0) {
          error_msg += " (Iterations: " + std::to_string(report.iters) 
                      + ", Final residual: " + std::to_string(report.final_res) + ")";
        }
        AddLogEntry(LogEntry::Error, error_msg);
      }
    }
    
    // Clear the logging callback after solve completes
    flowsheet.clear_log_callback();
  }
  ImGui::SameLine();
  if (ImGui::Button("Clear")) {
    log_messages.clear();
  }
  ImGui::SameLine();
  ImGui::Checkbox("Auto-scroll", &auto_scroll);
  
  ImGui::Separator();
  
  // Log display
  ImGui::BeginChild("LogContent", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
  
  // Enable text wrapping - wrap at window edge (0 = auto)
  ImGui::PushTextWrapPos(0.0f);
  
  for (const auto& entry : log_messages) {
    // Color code by type
    ImVec4 color;
    switch (entry.type) {
      case LogEntry::Info:
        color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
        break;
      case LogEntry::Success:
        color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
        break;
      case LogEntry::Error:
        color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
        break;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextWrapped("[%s] %s", entry.timestamp.c_str(), entry.message.c_str());
    ImGui::PopStyleColor();
  }
  
  ImGui::PopTextWrapPos();
  
  // Auto-scroll to bottom
  if (auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10) {
    ImGui::SetScrollHereY(1.0f);
  }
  
  ImGui::EndChild();
  ImGui::End();
}

