#include "product/catalog.h"

#include "spiderman1/runtime.h"
#include "spiderman2/runtime.h"

#include <array>
#include <cstdlib>
#include <lucent/log.h>

namespace spider {
namespace {

// From each title's title.json.
constexpr std::array<psx::host::TitleIdentity, 2> kTitles{{
    {
        .displayName = SPIDER1_TITLE_LABEL,
        .serial = SPIDER1_TITLE_SERIAL,
        .slug = SPIDER1_TITLE_ID,
        .fileSize = SPIDER1_TITLE_EXECUTABLE_SIZE,
        .sha256 = SPIDER1_TITLE_EXECUTABLE_SHA256,
        .entry = SPIDER1_TITLE_HEADER_ENTRY,
        .globalPointer = SPIDER1_TITLE_HEADER_GP,
        .textAddress = SPIDER1_TITLE_HEADER_TEXT_ADDRESS,
        .textSize = SPIDER1_TITLE_HEADER_TEXT_SIZE,
        .stackAddress = SPIDER1_TITLE_HEADER_STACK_ADDRESS,
        .stackOffset = SPIDER1_TITLE_HEADER_STACK_OFFSET,
        .discEnvVar = SPIDER1_TITLE_DISC_ENV,
        .discExecutablePath = SPIDER1_TITLE_DISC_EXECUTABLE,
    },
    {
        .displayName = SPIDER2_TITLE_LABEL,
        .serial = SPIDER2_TITLE_SERIAL,
        .slug = SPIDER2_TITLE_ID,
        .fileSize = SPIDER2_TITLE_EXECUTABLE_SIZE,
        .sha256 = SPIDER2_TITLE_EXECUTABLE_SHA256,
        .entry = SPIDER2_TITLE_HEADER_ENTRY,
        .globalPointer = SPIDER2_TITLE_HEADER_GP,
        .textAddress = SPIDER2_TITLE_HEADER_TEXT_ADDRESS,
        .textSize = SPIDER2_TITLE_HEADER_TEXT_SIZE,
        .stackAddress = SPIDER2_TITLE_HEADER_STACK_ADDRESS,
        .stackOffset = SPIDER2_TITLE_HEADER_STACK_OFFSET,
        .discEnvVar = SPIDER2_TITLE_DISC_ENV,
        .discExecutablePath = SPIDER2_TITLE_DISC_EXECUTABLE,
    },
}};

} // namespace

Catalog::Catalog()
    : spider1_(std::make_unique<spider1::Runtime>()),
      enterElectro_(std::make_unique<enterelectro::Runtime>()) {}

Catalog::~Catalog() = default;

std::string_view Catalog::productName() const {
  return "Spider-Man";
}

std::filesystem::path Catalog::provisioningRoot() const {
  return "scratch/assets";
}

std::span<const psx::host::TitleIdentity> Catalog::titles() const {
  return kTitles;
}

psx::game::GameRuntime &Catalog::runtime(std::size_t index) const {
  switch (index) {
  case 0:
    return *spider1_;
  case 1:
    return *enterElectro_;
  default:
    lucent::error("host", "Spider-Man catalog has no title {}", index);
    std::abort();
  }
}

std::unique_ptr<psx::host::TitleCatalog> createCatalog() {
  return std::make_unique<Catalog>();
}

} // namespace spider
