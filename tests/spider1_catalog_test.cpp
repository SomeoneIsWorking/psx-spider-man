// The host catalog is built from title.json; it must agree with the runtime and, when the
// player's executable is provisioned, with the PS-X EXE header.

#include "spider1_catalog.h"
#include "spider1_runtime.h"

#include "testutil.h"
#include "title_availability.h"

#include <memory>
#include <string_view>
#include <vector>

namespace {

void test_the_catalog_lists_spider_man_1_only() {
  const std::unique_ptr<psx::host::TitleCatalog> catalog = spider::spider1::createCatalog();
  CHECK(catalog->productName() == "Spider-Man");
  CHECK(catalog->provisioningRoot() == "scratch/assets");
  CHECK_EQ(catalog->titles().size(), 1u);
  CHECK(catalog->titles().front().slug == "spiderman1");
  CHECK(catalog->titles().front().serial == "SLUS_008.75");
}

void test_the_catalog_and_the_runtime_share_one_identity() {
  const std::unique_ptr<psx::host::TitleCatalog> catalog = spider::spider1::createCatalog();
  const psx::host::TitleIdentity &identity = catalog->titles().front();
  const spider::spider1::Spider1Runtime spider1;
  const spider::ExecutableIdentity &runtime = spider1.executableIdentity();
  CHECK(identity.serial == runtime.serial);
  CHECK_EQ(identity.fileSize, runtime.fileSize);
  CHECK(identity.sha256 == runtime.sha256);
  CHECK_STREQ(identity.discEnvVar, "PSXPORT_SPIDERMAN_DISC");
  CHECK(identity.discExecutablePath == "SLUS_008.75");
}

void test_a_provisioned_executable_passes_the_hosts_selection() {
  const std::unique_ptr<psx::host::TitleCatalog> catalog = spider::spider1::createCatalog();
  const std::vector<psx::host::TitleAvailability> titles =
      psx::host::TitleAvailabilityProbe(catalog->provisioningRoot(), catalog->titles()).probe();
  CHECK_EQ(titles.size(), 1u);
  if (titles.front().status == psx::host::AvailabilityStatus::NotProvisioned) {
    return;
  }
  CHECK_MSG(titles.front().available(), titles.front().reason.c_str());
}

} // namespace

int main() {
  RUN(the_catalog_lists_spider_man_1_only);
  RUN(the_catalog_and_the_runtime_share_one_identity);
  RUN(a_provisioned_executable_passes_the_hosts_selection);
  return pt_summary();
}
