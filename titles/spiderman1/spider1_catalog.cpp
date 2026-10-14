#include "spider1_catalog.h"

#include "provisioning_root.h"
#include "single_title_catalog.h"
#include "spider1_runtime.h"

namespace spider::spider1 {
namespace {

constexpr psx::host::TitleIdentity kIdentity{
    .displayName = SPIDER_TITLE_LABEL,
    .serial = SPIDER_TITLE_SERIAL,
    .slug = SPIDER_TITLE_ID,
    .fileSize = SPIDER_TITLE_EXECUTABLE_SIZE,
    .sha256 = SPIDER_TITLE_EXECUTABLE_SHA256,
    .entry = SPIDER_TITLE_HEADER_ENTRY,
    .globalPointer = SPIDER_TITLE_HEADER_GP,
    .textAddress = SPIDER_TITLE_HEADER_TEXT_ADDRESS,
    .textSize = SPIDER_TITLE_HEADER_TEXT_SIZE,
    .stackAddress = SPIDER_TITLE_HEADER_STACK_ADDRESS,
    .stackOffset = SPIDER_TITLE_HEADER_STACK_OFFSET,
    .discEnvVar = SPIDER_TITLE_DISC_ENV,
    .discExecutablePath = SPIDER_TITLE_DISC_EXECUTABLE,
};

} // namespace

std::unique_ptr<psx::host::TitleCatalog> createCatalog() {
  return std::make_unique<psx::host::SingleTitleCatalog>(
      kIdentity, kProvisioningRoot, std::make_unique<Spider1Runtime>());
}

} // namespace spider::spider1
