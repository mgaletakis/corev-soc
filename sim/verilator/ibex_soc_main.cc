// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "ibex_soc.h"

int main(int argc, char **argv) {
  ibexSoC soc(
      "TOP.ibex_soc.u_ram.u_ram.gen_generic.u_impl_generic",
      1024 * 1024);

  return soc.Main(argc, argv);
}
