#pragma once

#include "hello_imgui/icons_font_awesome_6.h"
#include <string>

// Window titles with icons - use these consistently throughout the application
namespace WindowTitles {
  // Main application windows
  inline const std::string UnitOperations = std::string(ICON_FA_CUBES) + " Unit Operations";
  inline const std::string Flowsheet = std::string(ICON_FA_SITEMAP) + " Flowsheet";
  inline const std::string Properties = std::string(ICON_FA_SLIDERS) + " Properties";
  inline const std::string FluidPackages = std::string(ICON_FA_FLASK) + " Fluid Packages";
  inline const std::string Chat = std::string(ICON_FA_COMMENTS) + " Chat";
  inline const std::string Log = std::string(ICON_FA_COMMENT_DOTS) + " Log";
}

