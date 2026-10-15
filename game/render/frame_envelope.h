// Native DRAWENV/DISPENV producer: page flip, drawing area and background clear.
#pragma once
#include "gpu_env.h"
#include <stdint.h>

class Core;

namespace spider::render {

class FrameEnvelope {
public:
  // Called before per-scene dispatch with the double-buffer context's blocks; a whole-guest
  // fallback frame skips it.
  void produce(Core *c, const DrawEnv &draw, const DispEnv &disp);

  unsigned long long produced() const {
    return mProduced;
  }
  unsigned long long clears() const {
    return mClears;
  }

  // PSXPORT_DEBUG=envcheck: on the reference leg, recomputes the words of the guest's DR_ENV at
  // DRAWENV+0x1C and compares. The summary carries `checked=` so a check that never ran shows.
  void verifyAgainstGuest(Core *c, uint32_t drawEnvAddr);
  unsigned long long checked() const {
    return mChecked;
  }
  unsigned long long mismatched() const {
    return mMismatched;
  }

private:
  // Last DISPENV programmed. Not libgpu's cache (0x800B0E94), never updated on the native leg;
  // its default dispW 0 differs from every real DISPENV, so frame one programs.
  DispEnv mLastDisp;
  unsigned long long mProduced = 0, mClears = 0, mChecked = 0, mMismatched = 0;
  static constexpr unsigned long long kReportEvery = 512;
  static constexpr int kMaxMismatchLines = 8;
};

} // namespace spider::render
