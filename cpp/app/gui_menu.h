#pragma once

#include <hello_imgui/runner_params.h>
#include <string>

void ShowMenus(HelloImGui::RunnerParams& params);

#ifdef EMSCRIPTEN
// Functions for showing messages and dialogs
void ShowMessagePopup(const std::string& title, const std::string& message, bool is_error);
void ShowExitConfirmation();
#endif

