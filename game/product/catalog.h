#pragma once

#include "title_catalog.h"

#include <memory>

namespace spider {

namespace spider1 {
class Runtime;
} // namespace spider1
namespace enterelectro {
class Runtime;
} // namespace enterelectro

// Spider-Man and Spider-Man 2: Enter Electro as the product's catalog, in that order.
class Catalog final : public psx::host::TitleCatalog {
public:
  Catalog();
  ~Catalog() override;

  std::string_view productName() const override;
  std::filesystem::path provisioningRoot() const override;
  std::span<const psx::host::TitleIdentity> titles() const override;
  psx::game::GameRuntime &runtime(std::size_t index) const override;

private:
  std::unique_ptr<spider1::Runtime> spider1_;
  std::unique_ptr<enterelectro::Runtime> enterElectro_;
};

// The product's catalog, run by the framework main (psxport_add_product).
std::unique_ptr<psx::host::TitleCatalog> createCatalog();

} // namespace spider
