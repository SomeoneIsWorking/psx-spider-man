#pragma once

#include <string_view>

namespace spider {

// Where every title's executable is provisioned, as <root>/<id>/<serial>; tools/title_catalog.py
// names the same root for the Python tools.
inline constexpr std::string_view kProvisioningRoot = "scratch/assets";

} // namespace spider
