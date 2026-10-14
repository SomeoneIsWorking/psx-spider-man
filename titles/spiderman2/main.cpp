#include "enter_electro_port.h"
#include "enter_electro_runtime.h"

int main(int argc, char **argv) {
  spider::enterelectro::EnterElectroRuntime runtime;
  return spider::enterelectro::runPort(runtime, argc, argv);
}
