// overlay_loader.h — Enter Electro's overlays, each published as a code image once its loader relocates it.
#pragma once

class Core;

namespace spider::enterelectro {

// FUN_8001E8E4 reads NAME.bin into a heap block, relocates it by NAME.rel through FUN_8001EEF0 and enters it.
// The relocator's return is where the block holds the finished module, so it becomes one image named by its
// bytes, a new generation per load.
class OverlayLoader {
public:
  static void install(Core &core);

private:
  static void relocated(Core *core);
};

} // namespace spider::enterelectro
