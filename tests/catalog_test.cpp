// The host catalog is built from both title.json manifests; when a player's executable is
// provisioned, it must pass the host's selection.

#include "product/catalog.h"

#include "spiderman1/runtime.h"
#include "spiderman2/runtime.h"

#include "testutil.h"
#include "title_availability.h"

#include <memory>
#include <vector>

namespace {

void test_the_catalog_lists_both_titles_in_order() {
  const std::unique_ptr<psx::host::TitleCatalog> catalog = spider::createCatalog();
  CHECK(catalog->productName() == "Spider-Man");
  CHECK(catalog->provisioningRoot() == "scratch/assets");
  CHECK_EQ(catalog->titles().size(), 2u);
  CHECK(catalog->titles()[0].slug == "spiderman1");
  CHECK(catalog->titles()[0].serial == "SLUS_008.75");
  CHECK_STREQ(catalog->titles()[0].discEnvVar, "PSXPORT_SPIDERMAN_DISC");
  CHECK(catalog->titles()[1].slug == "spiderman2");
  CHECK(catalog->titles()[1].serial == "SLUS_013.78");
  CHECK_STREQ(catalog->titles()[1].discEnvVar, "PSXPORT_SPIDERMAN2_DISC");
  CHECK_EQ(catalog->titles()[1].entry, 0x80093C68u);
}

void test_each_entry_names_its_titles_runtime() {
  const std::unique_ptr<psx::host::TitleCatalog> catalog = spider::createCatalog();
  CHECK(dynamic_cast<spider::spider1::Runtime *>(&catalog->runtime(0)) != nullptr);
  CHECK(dynamic_cast<spider::enterelectro::Runtime *>(&catalog->runtime(1)) != nullptr);
}

// Both titles own a widescreen projection policy and no interpolation.
void test_both_titles_own_a_widescreen_projection_without_interpolation() {
  const std::unique_ptr<psx::host::TitleCatalog> catalog = spider::createCatalog();
  for (std::size_t index = 0; index < catalog->titles().size(); ++index) {
    const psx::game::GameRuntime &runtime = catalog->runtime(index);
    CHECK(runtime.guestWidescreenProjection() != nullptr);
    CHECK(!runtime.interpolates());
  }
}

void test_a_provisioned_executable_passes_the_hosts_selection() {
  const std::unique_ptr<psx::host::TitleCatalog> catalog = spider::createCatalog();
  const std::vector<psx::host::TitleAvailability> titles =
      psx::host::TitleAvailabilityProbe(catalog->provisioningRoot(), catalog->titles()).probe();
  CHECK_EQ(titles.size(), 2u);
  for (const psx::host::TitleAvailability &title : titles) {
    if (title.status != psx::host::AvailabilityStatus::NotProvisioned) {
      CHECK_MSG(title.available(), title.reason.c_str());
    }
  }
}

} // namespace

int main() {
  RUN(the_catalog_lists_both_titles_in_order);
  RUN(each_entry_names_its_titles_runtime);
  RUN(both_titles_own_a_widescreen_projection_without_interpolation);
  RUN(a_provisioned_executable_passes_the_hosts_selection);
  return psx::test::summary();
}
