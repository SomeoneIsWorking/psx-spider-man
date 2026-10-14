#pragma once

#include "title_catalog.h"

#include <memory>

namespace spider {

namespace spider1 {
class Spider1Runtime;
}
namespace enterelectro {
class EnterElectroRuntime;
}

// Spider-Man and Spider-Man 2: Enter Electro as the product's catalog, in that order.
class SpiderCatalog final : public psx::host::TitleCatalog {
public:
  SpiderCatalog();
  ~SpiderCatalog() override;

  std::string_view productName() const override;
  std::filesystem::path provisioningRoot() const override;
  std::span<const psx::host::TitleIdentity> titles() const override;
  GameRuntime &runtime(std::size_t index) const override;

private:
  std::unique_ptr<spider1::Spider1Runtime> spider1_;
  std::unique_ptr<enterelectro::EnterElectroRuntime> enterElectro_;
};

// The product's catalog, run by the framework main (psxport_add_product).
std::unique_ptr<psx::host::TitleCatalog> createCatalog();

} // namespace spider
