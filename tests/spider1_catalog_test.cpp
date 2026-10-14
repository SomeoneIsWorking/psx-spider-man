// The host catalog is built from title.json; it must agree with the runtime and, when the
// player's executable is provisioned, with the PS-X EXE header.

#include "spider1_catalog.h"

#include "testutil.h"
#include "title_availability.h"

#include <string_view>
#include <vector>

namespace {

void test_the_catalog_lists_spider_man_1_only() {
  const spider::spider1::Spider1Catalog catalog;
  CHECK_EQ(catalog.titles().size(), 1u);
  CHECK(catalog.titles().front().slug == "spiderman1");
  CHECK(catalog.titles().front().serial == "SLUS_008.75");
}

void test_the_catalog_and_the_runtime_share_one_identity() {
  const spider::spider1::Spider1Catalog catalog;
  const psx::host::TitleIdentity &identity = catalog.titles().front();
  const spider::ExecutableIdentity &runtime = catalog.runtime(0).executableIdentity();
  CHECK(identity.serial == runtime.serial);
  CHECK_EQ(identity.fileSize, runtime.fileSize);
  CHECK(identity.sha256 == runtime.sha256);
  CHECK_STREQ(catalog.runtime(0).discEnvVar(), "PSXPORT_SPIDERMAN_DISC");
}

void test_a_provisioned_executable_passes_the_hosts_selection() {
  const spider::spider1::Spider1Catalog catalog;
  const std::vector<psx::host::TitleAvailability> titles =
      psx::host::TitleAvailabilityProbe(catalog.provisioningRoot(), catalog.titles()).probe();
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
