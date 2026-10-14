#pragma once

#include "title_catalog.h"

#include <memory>

namespace spider::spider1 {

// The product's catalog, run by the framework main (psxport_add_product): Spider-Man 1 alone until
// Spider-Man 2 joins it when its first game-owned call is ported (EE-02).
std::unique_ptr<psx::host::TitleCatalog> createCatalog();

} // namespace spider::spider1
