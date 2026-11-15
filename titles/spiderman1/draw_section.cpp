#include "spiderman1/draw_section.h"

#include "spiderman1/guest_layout.h"

#include "core.h"
#include "guest_redraw.h"

#include <algorithm>

namespace spider::spider1 {

std::optional<std::uint32_t> DrawSection::draw(Core &core,
                                               psx::present::GuestRedraw &redraw) const {
  const std::uint32_t orderingTable =
      core.mem_r32(core.mem_r32(drawBuffer) + drawBufferOrderingTable);
  const std::array<std::uint32_t, 2> compact{orderingTable, orderingTableEntries};
  const bool drawn =
      redraw.dispatch(renderWalk, {}) && redraw.dispatch(orderingTableCompact, compact);
  poses_.restore(core);
  if (!drawn) {
    return std::nullopt;
  }
  const std::uint32_t head = orderingTable + orderingTableHeadOffset;
  if (std::ranges::find(heads_, head) == heads_.end()) {
    heads_ = {heads_[1], head};
  }
  return head;
}

bool DrawSection::linked(std::uint32_t head) const {
  return head != 0u && std::ranges::find(heads_, head) != heads_.end();
}

} // namespace spider::spider1
