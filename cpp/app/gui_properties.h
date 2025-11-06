#pragma once

#include "processX/flowsheet.h"
#include "gui_common.h"
#include <imgui.h>

void ShowValveProperties(px::Valve& valve);
void ShowMixerProperties(px::Mixer& mixer);
void ShowSplitterProperties(px::Splitter& splitter);
void ShowStreamProperties(px::Stream& stream);
void ShowSelectedUnitProperties();

