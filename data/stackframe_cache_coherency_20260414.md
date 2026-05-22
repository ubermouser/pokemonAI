# StackFrame Cache Coherency Analysis

## Problem Summary

The `StackFrame` struct contains **4 `std::unordered_map<Actor, ...>`** fields:

| Field | Value Type | Size of Value |
|---|---|---|
| `actions` | `Action` | 2 bytes (bit-packed uint16_t) |
| `moveBrackets` | `MoveBracket` | 12 bytes (int32_t + 2×uint32_t) |
| `targets` | `std::vector<Actor>` | 24 bytes (vector header) + heap alloc |
| `damageComponents` | `DamageComponents_t` | ~32 bytes |

Each `unordered_map` is an associative hash table with:
- **Heap-allocated bucket arrays** and **heap-allocated nodes** (each entry is a separate allocation)
- **Pointer chasing** on every lookup (`operator[]`, `.at()`, `.find()`)
- **Hashing overhead** — `std::hash<Actor>` is called on every access

The key type `Actor` is just `{uint32_t iTeam_, uint32_t iTeammate_}` — 8 bytes total. The unique index space is `iTeam * 6 + iTeammate` = **at most 12 values** (2 teams × 6 teammates). In practice, maps contain 1–6 entries (typically 2–6 for multi-battles).

## Why This Dominates Profiling

### 1. Every stage function hits multiple maps

A single call to e.g. `getPKV()` chains through:
```
getPKV(iBase_)
  → getCActor(iBase_)           // reads stackFrame_[iBase_].moveOrder[iActor] ✓ (vector, ok)
    → getBase(iState).teammate(actor)  // reads stack_ ✓
```
But then `getMV()` does:
```
getMV(iState)
  → getCActor(iState)           // re-reads moveOrder
  → frame.actions.at(actor)     // ← HASH + POINTER CHASE through unordered_map
  → getBase(iState).teammate(actor).getMV(action)
```

And `getDamageComponent()` does:
```
getDamageComponent(iStack)
  → getCActor(iStack)           // re-reads moveOrder again
  → stackFrame_[iStack].damageComponents.at(actor)  // ← ANOTHER hash + chase
```

A typical stage function like `evaluateMove_damage_modifyAttackPower` calls `getPKV()`, `getTPKV()`, `getMV()`, `getDamageComponent()`, `getBase().flagsFor(getCActor())`. That's **4+ unordered_map lookups** per stage, across ~30 stages per actor per turn. For a 2v2 battle that's **240+ hash-table lookups per environment per turn**, and there can be many environments on the stack.

### 2. StackFrame copies are expensive

`nPlicateState` copies the entire `StackFrame` (line 196: `stackFrame_.push_back(baseFrame)`). Each copy deep-copies **4 unordered_maps** — each allocating new bucket arrays and nodes. This happens at every branching point (hit chance, crit chance, secondary effects, speed ties).

### 3. The maps are too small for hashing to pay off

With 2–6 entries, the hash table has more overhead than a linear scan. The bucket array, load factor management, and per-node heap allocations all dwarf the actual data.

## Proposed Solution: Flat Array Storage

Since `Actor::index()` maps to `[0, 12)`, replace all four maps with fixed-size arrays indexed by `Actor::index()`:

```cpp
static constexpr size_t MAX_ACTORS = 12;  // 2 teams × 6 teammates

struct StackFrame {
    size_t iStack;
    StageType stage;
    size_t iActor;
    size_t iTarget;
    
    std::vector<Actor> moveOrder;
    
    // Flat arrays — indexed by Actor::index()
    std::array<Action, MAX_ACTORS> actions;
    std::array<MoveBracket, MAX_ACTORS> moveBrackets;
    std::array<std::vector<Actor>, MAX_ACTORS> targets;  // see note below
    std::array<DamageComponents_t, MAX_ACTORS> damageComponents;
};
```

### Benefits

| Metric | `unordered_map` | `std::array` |
|---|---|---|
| Lookup | Hash + bucket scan + pointer chase | Single indexed load |
| Cache behavior | 2–3 cache misses per lookup | 0–1 cache miss (data inline) |
| Copy cost | N heap allocations per map × 4 maps | Trivial memcpy (~600 bytes) |
| Total struct size | ~4 heap allocs + overhead per map | ~600 bytes inline (excluding vectors) |

### Sizing

| Field | Per-element | × 12 | Total |
|---|---|---|---|
| `actions` | 2B | 24B | 24 bytes |
| `moveBrackets` | 12B | 144B | 144 bytes |
| `damageComponents` | ~32B | 384B | 384 bytes |
| `targets` (vector headers) | 24B | 288B | 288 bytes |
| **Total inline** | | | **~840 bytes** |

This fits comfortably in **2 cache lines** for the hot fields (`actions` + `moveBrackets`), vs. the current design which scatters data across many heap allocations.

> [!NOTE]
> The `targets` array still contains `std::vector<Actor>` elements, which will have their own heap allocations. However, this is a net improvement because
> 1. the vector headers are now contiguous, and
> 2. targets are typically small (1-3 actors), so could optionally be replaced with a `std::array<Actor, 3>` + count in the future.

## Migration Strategy

### Access Pattern Changes

All `.at(actor)` calls become `[actor.index()]`:

```diff
-frame.actions.at(actor)
+frame.actions[actor.index()]

-frame.moveBrackets[actor] = computeMoveBracket(actor);
+frame.moveBrackets[actor.index()] = computeMoveBracket(actor);

-frame.damageComponents.at(actor)
+frame.damageComponents[actor.index()]
```

### Initialization Changes

In `seedStack()`, instead of inserting only the active actors, you'd write to the array slots for each actor's index. Unused slots remain default-initialized (which is fine — they're never accessed because `moveOrder` controls iteration).

### Iteration Changes

Code that iterates over maps like `for (auto& [actor, targetList] : frame.targets)` must change to iterate via `moveOrder`:

```diff
-for (auto& [actor, targetList] : frame.targets) {
+for (const auto& actor : frame.moveOrder) {
+  auto& targetList = frame.targets[actor.index()];
```

Similarly in `evaluateMove_selectOrder`, iteration over `frame.moveBrackets` needs to use `moveOrder` or another actor list.

### Files Requiring Changes

| File | Changes |
|---|---|
| [neo_pkCU_engine.h](file:///workspace/include/pokemonai/neo_pkCU_engine.h) | StackFrame struct definition |
| [neo_pkCU_engine.cpp](file:///workspace/src/neo_pkCU_engine.cpp) | All accessor implementations + seedStack/handleActorSwitch |
| [neo_pkCU_engine_stages.cpp](file:///workspace/src/neo_pkCU_engine_stages.cpp) | All stage functions that access frame fields |

### Key Call Sites Needing Updates

In [neo_pkCU_engine.cpp](file:///workspace/src/neo_pkCU_engine.cpp):
- [seedStack()](file:///workspace/src/neo_pkCU_engine.cpp#L31-L48): `frame.actions`, `frame.targets`, `frame.damageComponents` initialization
- [handleActorSwitch()](file:///workspace/src/neo_pkCU_engine.cpp#L522-L541): `frame.targets`, `frame.actions`, `frame.moveBrackets`, `frame.damageComponents` swaps
- [getMV()](file:///workspace/src/neo_pkCU_engine.cpp#L579-L583): `frame.actions.at(actor)` lookup
- [getTMV()](file:///workspace/src/neo_pkCU_engine.cpp#L589-L593): `frame.actions.at(target)` lookup
- [getDamageComponent()](file:///workspace/src/neo_pkCU_engine.cpp#L596-L621): `frame.damageComponents.at(actor)` lookups
- [getTarget()](file:///workspace/src/neo_pkCU_engine.cpp#L651-L656): `frame.targets.at(actor)` lookup
- [getCAction()](file:///workspace/src/neo_pkCU_engine.cpp#L659-L661): `frame.actions.at(getCActor())` lookup

In [neo_pkCU_engine_stages.cpp](file:///workspace/src/neo_pkCU_engine_stages.cpp):
- [evaluateMove()](file:///workspace/src/neo_pkCU_engine_stages.cpp#L20-L177): `frame.actions.at(actor)`, `frame.targets.at(actor)` in trace logging
- [evaluateMove_computeBracket()](file:///workspace/src/neo_pkCU_engine_stages.cpp#L224-L227): `frame.moveBrackets[actor]` write
- [evaluateMove_selectOrder()](file:///workspace/src/neo_pkCU_engine_stages.cpp#L243-L328): iteration over `frame.moveBrackets` + writes
- [evaluateMove_modifyAction()](file:///workspace/src/neo_pkCU_engine_stages.cpp#L331-L338): `frame.actions.at(actor)` lookup
- [evaluateMove_validateForcedAction()](file:///workspace/src/neo_pkCU_engine_stages.cpp#L341-L358): `frame.actions.at(actor)`, `frame.targets[actor]` lookups
- [evaluateMove_postTurn()](file:///workspace/src/neo_pkCU_engine_stages.cpp#L670-L698): `frame.targets[actor]` lookup

## Open Questions

> [!IMPORTANT]
> **Default initialization**: The flat arrays will have slots for all 12 possible actors, but typically only 2–6 are in use. The unused slots will be default-initialized. Is this acceptable, or do you want a bitmask/presence flag to guard against accidental access to unused slots? (I'd recommend no guard — `moveOrder` already constrains valid indices.)

> [!IMPORTANT]
> **`targets` vectors**: The `targets` field contains `std::vector<Actor>` which still makes heap allocations. In a future pass, these could be replaced with `std::array<Actor, MAX_TARGETS> + size_t count` (where MAX_TARGETS is likely 3–6). Should this be done as part of this change, or deferred?

> [!IMPORTANT]
> **`ActionMap` typedef**: The typedef `using ActionMap = std::unordered_map<Actor, Action>` is used elsewhere (e.g. the engine constructor parameter). The `StackFrame::actions` change is internal, but the `actions_` member on the engine itself (line 360) is also an `ActionMap`. Should that be changed too, or only the per-frame storage?
