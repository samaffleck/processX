#pragma once

#include <string>
#include <unordered_map>
#include <optional>

// Typedef
using var = std::optional<double>;
using comp_var = std::unordered_map<std::string, var>;
