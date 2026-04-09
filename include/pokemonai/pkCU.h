/**
 * @file pkCU.h
 * @brief Compatibility layer for the Pokemon battle engine.
 *
 * This file provides typedefs to point `PkCU` and `PkCUEngine` to their
 * legacy implementations, while also including the new `NeoPkCU` interface.
 */
#ifndef PKAI_CU_H
#define PKAI_CU_H

#include "neo_pkCU.h"

/**
 * @typedef PkCU
 * @brief Typedef for the active battle engine.
 */
using PkCU = NeoPkCU;

/**
 * @typedef PkCUEngine
 * @brief Typedef for the active battle engine's state machine.
 */
using PkCUEngine = NeoPkCUEngine;

#endif /* PKAI_CU_H */
