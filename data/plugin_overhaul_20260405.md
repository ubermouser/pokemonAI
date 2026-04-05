# Bound Plugin System for NeoPkCU — Refined Design

## Problem Recap

NeoPkCU's flat `PluginSet` fires all entity plugins for every action. Entity plugins (bound to specific moves/abilities/items) currently self-filter. This is error-prone, wasteful, and doesn't scale.

We want **Option A** — a keyed `BoundPluginMap` that the engine queries at each trigger — but without heap allocation in the hot loop.

## Critical Insight: Bound Plugin Lists Are Single-Element

Each [`Pluggable`](file:///workspace/include/pokemonai/pluggable.h#L82-L141) stores exactly **one `plugin_t` per trigger slot** in a fixed `std::array<plugin_t, PLUGIN_MAXSIZE>`. A move cannot register two different functions for `PLUGIN_ON_EVALUATEMOVE`.

This means when we look up `boundPlugins_[trigger][&movePluggable]`, we get **0 or 1 elements**, not a list.

For any given `CALLPLUGIN` invocation, the total elements to iterate come from:

| Source | Max elements |
|--------|-------------|
| Global plugins (engine) | 0–3 |
| Actor's current move | 0–1 |
| Actor's ability | 0–1 |
| Actor's item | 0–1 |
| Target's ability (other_team) | 0–1 |
| Target's item (other_team) | 0–1 |
| **Total** | **3–8** |

## Generator/Coroutine Analysis

### C++20 Coroutine Generator — Not Viable

The project targets **C++17** (`CMAKE_CXX_STANDARD 17`). C++20 `co_yield` and C++23 `std::generator` are unavailable without a standard bump.

Even if available:
- **Heap allocation**: C++20 coroutines allocate a coroutine frame. Compilers *can* apply Heap Allocation Elision Optimization (HALO), but this is **unreliable** — it requires the coroutine lifetime to be provably bounded by the caller, the call to be inlined, and the compiler to opt in. GCC and Clang have inconsistent HALO support. In practice, most coroutine invocations allocate on the heap, **defeating the primary goal**.
- **Per-yield overhead**: Each `co_yield` involves saving coroutine state and a context switch through the promise. Estimated cost: **~10–30ns per element**. For 5-8 elements in a loop called 60-120× per turn × millions of turns, this adds up to significant overhead.
- **Conclusion**: Even with C++20, a coroutine generator would likely be *slower* than the current flat iteration due to frame allocation + per-yield suspend/resume overhead.

### Manual Merge Iterator (Stateful Struct) — Viable but Unnecessary

A stack-allocated struct that maintains positions into k pre-sorted sources and yields the minimum element on each `next()` call:

```cpp
struct PluginMergeIterator {
    std::array<const plugin_t*, 6> sources;
    size_t numSources;
    
    const plugin_t* next() {
        // find source with lowest-priority next element
        // advance that source, return element
    }
};
```

- **No heap allocation** ✓
- **Per-element overhead**: ~5–10ns for the k-way min scan (k≤6). Small but nonzero.
- **Complexity**: Moderate — need careful handling of exhausted sources, each source being either a span (for global) or a single optional element (for bound).
- This is what `boost::range::join` would give you, but join doesn't merge by priority — it concatenates.

### Stack-Allocated Fixed Buffer + Insertion Sort — Recommended

For 3–8 elements of 16 bytes each (128 bytes total), the simplest and fastest approach is:

```cpp
// Stack-allocated, fits in L1 cache line
std::array<plugin_t, 8> merged;
size_t count = 0;

// Copy global plugins (pre-sorted, 0-3 elements)
for (const auto& p : globalPlugins_[trigger]) {
    merged[count++] = p;
}

// Append bound plugins (0-1 element each)
if (auto* p = lookupBound(trigger, &currentMove))  merged[count++] = *p;
if (auto* p = lookupBound(trigger, &actorAbility))  merged[count++] = *p;
if (auto* p = lookupBound(trigger, &actorItem))     merged[count++] = *p;
if (auto* p = lookupBound(trigger, &targetAbility)) merged[count++] = *p;
if (auto* p = lookupBound(trigger, &targetItem))    merged[count++] = *p;

// Insertion sort: O(n²) but n≤8, so ≤28 comparisons worst case
// In practice, globals are already sorted and bound elements are 
// mostly priority 0, so ~5 comparisons on average
insertionSort(merged.data(), count);

// Iterate (same as current CALLPLUGIN)
for (size_t i = 0; i < count; ++i) {
    retValue |= dispatch(merged[i], args...);
    if (retValue > 1) break;
}
```

- **No heap allocation** ✓
- **Stack cost**: 128 bytes — fits in a single cache line
- **Sort cost**: Insertion sort on 5-8 elements ≈ **~5ns** (branch-predictor friendly, cache-hot)
- **Lookup cost**: 3-5 `unordered_map::find` calls ≈ **~15-25ns** total
- **Total overhead vs. current flat iteration**: ~20-30ns per invocation
- **Code simplicity**: Straightforward, no iterator state machine

## Comparison with Option A' (Per-Actor Cache)

| Factor | Stack Buffer (A) | Per-Actor Cache (A') |
|--------|-----------------|---------------------|
| Heap allocation | Never | Once per actor per turn |
| Cache invalidation | N/A (stateless) | On actor change, target change |
| Multi-target moves | Free (rebuilt each call) | Must invalidate + rebuild |
| Cache-busting moves (baton pass, u-turn, thief) | Free (stateless) | Must explicitly invalidate |
| Per-invocation cost | ~25ns (hash lookups + sort) | ~2ns (iterate pre-built list) |
| Amortized per-turn cost | ~25ns × 15 stages = **375ns** | ~25ns build + 2ns × 15 = **55ns** |
| Reuse across probabilistic states | No (rebuilt each time) | Yes (shared across branches) |
| Code complexity | Low | Moderate (lifecycle management) |

The per-actor cache (A') is ~7× faster per turn for a single actor, but the absolute difference is **~320ns per turn** — negligible relative to the overall `updateState` cost (which involves hash computation, probability arithmetic, and state copying measured in microseconds).

> [!IMPORTANT]
> The stack buffer approach trades ~320ns/turn of performance for significantly reduced complexity: no cache lifecycle, no invalidation for multi-target moves, no special handling for baton pass/u-turn/thief/transform, and no coupling between StackFrame and the plugin system. **Start with the stack buffer. If profiling shows the hash lookups in `CALLPLUGIN` are a bottleneck, upgrade to per-actor caching.**

## Proposed Implementation

### New Types

```cpp
// In pkCU_types.h or pluggable.h:

// Bound plugins: one plugin_t per Pluggable (not a vector, since Pluggable 
// stores at most one plugin per trigger)
using BoundPluginMap = std::array<
    std::unordered_map<const Pluggable*, plugin_t>,
    PLUGIN_MAXSIZE>;
```

### Changes to [neo_pkCU.h](file:///workspace/include/pokemonai/neo_pkCU.h)

Replace `PluginSet pluginSet_` with:

```cpp
PluginSet globalPlugins_;       // engine plugins only (always fire)
BoundPluginMap boundPlugins_;   // entity plugins keyed by source Pluggable*
```

### Changes to [neo_pkCU.cpp](file:///workspace/src/neo_pkCU.cpp) `initialize()`

```cpp
void NeoPkCU::initialize() {
  for (auto& set : globalPlugins_) set.clear();
  for (auto& map : boundPlugins_) map.clear();

  // Entity plugins → boundPlugins_
  for (const auto& [actor, pokemon] : nv_->yieldPokemon()) {
    for (const auto& [iMove, mNV] : pokemon.yieldMoves()) {
      const auto& move = mNV.getBase();
      for (size_t i = 0; i < PLUGIN_MAXSIZE; ++i) {
        plugin_t p = move.getPlugin(i);
        if (p.pFunction) { boundPlugins_[i][&move] = p; }
      }
    }
    if (pokemon.abilityExists()) {
      const auto& ability = pokemon.getAbility();
      for (size_t i = 0; i < PLUGIN_MAXSIZE; ++i) {
        plugin_t p = ability.getPlugin(i);
        if (p.pFunction) { boundPlugins_[i][&ability] = p; }
      }
    }
    if (pokemon.hasInitialItem()) {
      const auto& item = pokemon.getInitialItem();
      for (size_t i = 0; i < PLUGIN_MAXSIZE; ++i) {
        plugin_t p = item.getPlugin(i);
        if (p.pFunction) { boundPlugins_[i][&item] = p; }
      }
    }
  }

  // Engine plugins → globalPlugins_ (pre-sorted)
  if (pkdex) {
    const auto& extensions = pkdex->getExtensions();
    for (size_t i = 0; i < PLUGIN_MAXSIZE; ++i) {
      for (size_t j = 0; j < extensions.getNumPlugins(i); ++j) {
        globalPlugins_[i].push_back(extensions.getPlugin(i, j));
      }
    }
  }
  for (auto& set : globalPlugins_) std::sort(set.begin(), set.end());
  // ...
}
```

### New CALLPLUGIN Replacement

```cpp
// Helper: look up a bound plugin, return nullptr if not found
inline const plugin_t* lookupBound(
    const BoundPluginMap& bound, size_t trigger, const Pluggable* key) {
  if (!key) return nullptr;
  auto it = bound[trigger].find(key);
  if (it == bound[trigger].end()) return nullptr;
  if (!it->second.pFunction) return nullptr;
  return &it->second;
}

// Insertion sort for small arrays (n ≤ 8)
inline void insertionSortPlugins(plugin_t* arr, size_t n) {
  for (size_t i = 1; i < n; ++i) {
    plugin_t key = arr[i];
    size_t j = i;
    while (j > 0 && arr[j-1].priority > key.priority) {
      arr[j] = arr[j-1];
      --j;
    }
    arr[j] = key;
  }
}

#define CALLPLUGIN_BOUND(retValue, trigger, pluginFunction,             \
                         actor_move, actor_ability, actor_item,         \
                         target_ability, target_item, ...)              \
{                                                                       \
  std::array<plugin_t, 8> _merged;                                      \
  size_t _count = 0;                                                    \
  for (const auto& _p : cu_.globalPlugins_[(size_t)trigger]) {         \
    _merged[_count++] = _p;                                             \
  }                                                                     \
  if (auto* _p = lookupBound(cu_.boundPlugins_, trigger, actor_move))   \
    _merged[_count++] = *_p;                                            \
  if (auto* _p = lookupBound(cu_.boundPlugins_, trigger, actor_ability))\
    _merged[_count++] = *_p;                                            \
  if (auto* _p = lookupBound(cu_.boundPlugins_, trigger, actor_item))  \
    _merged[_count++] = *_p;                                            \
  if (auto* _p = lookupBound(cu_.boundPlugins_, trigger, target_ability))\
    _merged[_count++] = *_p;                                            \
  if (auto* _p = lookupBound(cu_.boundPlugins_, trigger, target_item)) \
    _merged[_count++] = *_p;                                            \
  insertionSortPlugins(_merged.data(), _count);                         \
  for (size_t _i = 0; _i < _count; ++_i) {                            \
    pluginFunction _fn = (pluginFunction)_merged[_i].pFunction;        \
    retValue = retValue | _fn(__VA_ARGS__);                             \
    if (retValue > 1) { break; }                                        \
  }                                                                     \
}
```

> [!WARNING]
> The `pluginTarget` field determines which team's pluggables to look up. The macro above takes separate actor and target pluggable pointers. At each call site, the engine must determine which move/ability/item pointers to pass based on context. Not every trigger has a "current move" (e.g., `PLUGIN_ON_ENDOFROUND` is per-pokemon, not per-move).

### Migration Strategy

1. Add `globalPlugins_` and `boundPlugins_` alongside existing `pluginSet_`
2. Create `CALLPLUGIN_BOUND` macro
3. Migrate one trigger at a time (start with `PLUGIN_ON_EVALUATEMOVE` since it has the most bound plugins and existing tests)
4. Remove self-filter checks from migrated plugins
5. Once all triggers migrated, remove `pluginSet_`

### Impact on Plugin Scripts

After migration, bound plugins no longer need identity checks:

```diff
 int move_heal50(PkCUEngine& cu, MoveVolatile mV, PokemonVolatile cPKV, PokemonVolatile tPKV) {
-  const Move* tMove = &mV.getBase();
-  if ((tMove != recover_t) && (tMove != milkDrink_t) && (tMove != slackOff_t) &&
-      (tMove != softBoiled_t) && (tMove != healOrder_t) && (tMove != roost_t)) {
-    return 0;
-  }
   cPKV.modPercentHP(0.50);
   return 1;
 }
```

> [!WARNING]  
> Some plugins check move identity for semantic reasons (e.g., `move_counterMirrorCoat` distinguishes Counter from Mirror Coat to decide physical vs. special). Those checks should remain — they're not mere filtering, they're behavioral branching.

## Open Questions

> [!IMPORTANT]
> 1. **Should we keep the old `CALLPLUGIN` macro for triggers that only have engine plugins** (like `PLUGIN_ON_MODIFYSPEED` which has only 1 engine plugin and 1 ability plugin)? Or unify everything through `CALLPLUGIN_BOUND`?

> [!IMPORTANT]
> 2. **Should `BoundPluginMap` use `const Pluggable*` as key or a cheaper identifier?** A `Pluggable*` is a stable pointer (moves/abilities/items are owned by the Pokedex and don't move). Hash of a pointer is fast. Alternative: use the move/ability/item name hash, but pointer identity is simpler and sufficient.

> [!IMPORTANT]
> 3. **Should we keep backward compatibility with LegacyPkCU?** The legacy engine has its own `PluginSets` and wouldn't use bound plugins. The `plugin` class and `Pluggable` class remain unchanged — only NeoPkCU's storage and invocation change.

## Verification Plan

### Automated Tests
- All existing tests pass with `ctest --preset conan-debug`
- Specifically: `MoveTest.Heal50`, all ability and item tests  
- After removing self-filter from `move_heal50`, verify it still only fires for heal-50 moves

### Manual Verification
- Count plugins at init: `globalPlugins_` + unique keys in `boundPlugins_` should equal previous `pluginSet_` count
- Trace plugin dispatch with `SPDLOG_TRACE` to verify only relevant plugins fire
