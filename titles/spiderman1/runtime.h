#pragma once

#include "runtime/runtime.h"
#include "spiderman1/debug_menu.h"
#include "stock_dma_callbacks.h"
#include "stock_libcd.h"
#include "stock_libgte.h"

class Core;

namespace spider::spider1 {

// Spider-Man (SLUS_008.75): crt0 replay and retail main as host turns, the stock libgte/libcd/DMA
// bindings, and the overlay loader's CD reads as code modules.
class Runtime final : public spider::Runtime {
public:
  Runtime();

  void startProgram(Core &core) override;
  void registerOverrides(Game &game) override;
  std::unique_ptr<psx::game::TitleSessionState> createSession(Game &game) override;
  std::unique_ptr<psx::game::FrameDriver> createFrameDriver(Game &game) override;
  const psx::game::TitleDebugMenu &debugMenu() const override {
    return debugMenu_;
  }
  // The overlay loader streams code through the stock CdRead; each read inside its heap is an
  // image.
  void stockCdReadLanded(Core &core, const psx::cd::StockReadLanding &landing) override;

private:
  psx::hle::StockLibgte libgte_;
  // The data-ready interrupt belongs to the guest's own source-2 handler 0x8008DA24: libstr's ready
  // callback 0x800860B4 calls CdReady 0x8008CBC4, which needs the response that handler stages.
  psx::cd::StockLibcd libcd_;
  psx::hle::StockDmaCallbacks dmaCallbacks_;
  DebugMenu debugMenu_;
};

} // namespace spider::spider1
