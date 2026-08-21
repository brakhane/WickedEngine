# Weighted Blended OIT — implementation plan

Goal: fix the outdated per-object CPU-sorted back-to-front transparency in
`wiRenderer.cpp`'s `DrawScene`/`RenderMeshes` by adding Weighted Blended OIT
(McGuire & Bavoil), as a self-contained, opt-in, low-invasiveness feature
(the codebase owner dislikes large refactors). No code has been written yet —
this file only records the investigation and the concrete plan.

Status: **investigation done, zero files modified**. Resume by executing the
steps below in order.

## Design decisions already made

- Technique: Weighted Blended OIT (2 extra render targets: accumulation
  RGBA16F, revealage R8; additive/multiplicative blending; no sorting
  needed; single geometry pass, no per-pixel linked lists / depth peeling).
- Applies **uniformly to all transparent blend modes** (`BLENDMODE_ALPHA`,
  `ADDITIVE`, `PREMULTIPLIED`, `MULTIPLY`, `INVERSE` — i.e. any
  `blendmode != BLENDMODE_OPAQUE` reaching `RENDERPASS_MAIN` with
  `transparency == true`), not just plain alpha. This avoids having to
  split the render queue into "OIT-eligible" vs "not" and issue two
  separate draw+composite passes. Trade-off (documented, not hidden):
  additive/multiply materials will look slightly different under OIT
  since WBOIT assumes "over" compositing — this is the standard, accepted
  limitation of the technique and matches how most engines apply it.
- Gated behind a runtime toggle, **off by default**, so existing behavior
  is 100% unchanged unless a user opts in. No existing PSOs, blend states,
  or draw calls are altered — everything is additive.
- Sorting (`RenderQueue::sort_transparent`, wiRenderer.cpp:320-323) is left
  untouched. It's harmless under OIT (order doesn't affect correctness) and
  still gives useful mesh/LOD batching, so there's no reason to touch it —
  keeps the diff smaller.
- No new `.hlsl` files needed: the existing permutation mechanism already
  compiles `objectPS.hlsl` multiple times with different `defines` lists
  (see wiRenderer.cpp:1185-1209). OIT is just one more `defines` combination
  (`TRANSPARENT` + `OIT`) over the same source file.

## Key architecture facts gathered (with file:line)

- `DrawScene()` — wiRenderer.cpp:7396. Builds `renderQueue`/`renderQueue_transparent`,
  sorts, calls `RenderMeshes(vis, renderQueue, renderPass, filterMask, cmd, flags)`.
- `RenderQueue`/`RenderBatch` sort keys — wiRenderer.cpp:230-336. Transparent sort
  is back-to-front by per-instance distance only (wiRenderer.cpp:7457), no OIT
  anywhere in the codebase (confirmed via grep, zero hits for OIT/weighted-blended/
  linked-list terms).
- `ObjectRenderingVariant` bitfield — wiRenderer.cpp:347-362. Union of
  `{renderpass:4, shadertype:8, blendmode:4, cullmode:2, tessellation:1,
  alphatest:1, sample_count:4, mesh_shader:1}` = 25 bits used of 32 →
  **room for a new 1-bit `oit` field**.
- `GetObjectPSO()` — wiRenderer.cpp:363-367. PSOs are cached in
  `wi::unordered_map<uint32_t, PipelineState> PSO_object[RENDERPASS_COUNT][SHADERTYPE_COUNT][OBJECT_MESH_SHADER_PSO_COUNT]`
  keyed by `variant.value` — **adding a bit to the variant just changes the
  hash key, no array resizing needed.**
- PSO enums — `wiEnums.h`:
  - `RENDERPASS` enum: lines 48-58 (`RENDERPASS_MAIN` = 0).
  - `SHADERTYPE` enum: lines 107+. Object PS permutation ranges:
    `PSTYPE_OBJECT_PERMUTATION_BEGIN`/`_END` (line 152-153) and
    `PSTYPE_OBJECT_TRANSPARENT_PERMUTATION_BEGIN`/`_END` (line 154-155),
    each sized `SHADERTYPE_BIN_COUNT` (= 12, shaders/ShaderInterop_Renderer.h:512).
    **Add `PSTYPE_OBJECT_OIT_PERMUTATION_BEGIN`/`_END` right after line 155**,
    same `SHADERTYPE_BIN_COUNT` sizing.
  - `DSSTYPE` enum: lines 453-479. Reuse existing `DSSTYPE_DEPTHREAD`
    (depth test GREATER_EQUAL, write disabled, stencil disabled — exactly
    what unordered OIT accumulation needs) — **no new DSSTYPE needed.**
  - `BSTYPE` enum: lines 484-492. **Add `BSTYPE_OIT`.**
- Blend state construction — `SetUpStates()` wiRenderer.cpp:2242, blend
  states built from line 2480 (`BlendState bd; ... blendStates[BSTYPE_X] = bd;`
  pattern, one after another reusing `bd`). Add after `BSTYPE_TRANSPARENT`
  (built at wiRenderer.cpp:2487-2497):
  ```cpp
  bd.independent_blend_enable = true;
  bd.alpha_to_coverage_enable = false;
  // RT0: accumulation (RGBA16F), additive
  bd.render_target[0].blend_enable = true;
  bd.render_target[0].src_blend = Blend::ONE;
  bd.render_target[0].dest_blend = Blend::ONE;
  bd.render_target[0].blend_op = BlendOp::ADD;
  bd.render_target[0].src_blend_alpha = Blend::ONE;
  bd.render_target[0].dest_blend_alpha = Blend::ONE;
  bd.render_target[0].blend_op_alpha = BlendOp::ADD;
  bd.render_target[0].render_target_write_mask = ColorWrite::ENABLE_ALL;
  // RT1: revealage (R8), multiplicative (dst *= (1-src))
  bd.render_target[1].blend_enable = true;
  bd.render_target[1].src_blend = Blend::ZERO;
  bd.render_target[1].dest_blend = Blend::INV_SRC_COLOR;
  bd.render_target[1].blend_op = BlendOp::ADD;
  bd.render_target[1].src_blend_alpha = Blend::ZERO;
  bd.render_target[1].dest_blend_alpha = Blend::INV_SRC_COLOR;
  bd.render_target[1].blend_op_alpha = BlendOp::ADD;
  bd.render_target[1].render_target_write_mask = ColorWrite::ENABLE_ALL;
  blendStates[BSTYPE_OIT] = bd;
  bd.independent_blend_enable = false; // restore default for subsequent states in this function
  ```
  (Verify no later code in `SetUpStates()` assumes `bd.render_target[1]` is
  untouched/default — reset `bd = BlendState{}` or just `bd.independent_blend_enable = false`
  and re-set `bd.render_target[0]` before the next block if needed; check
  the state right after this insertion point still compiles/behaves correctly.)
- `BlendState`/`RenderTargetBlendState` struct — wiGraphics.h:627-645.
  Confirms `independent_blend_enable` and `render_target[8]` exist — per-RT
  blend is supported.
- `Blend` enum — wiGraphics.h:116-135 (`ZERO, ONE, SRC_COLOR, INV_SRC_COLOR,
  SRC_ALPHA, INV_SRC_ALPHA, ...`).
- `PipelineStateDesc` — wiGraphics.h:665-681.
- PSO precompile loop (the big one) — wiRenderer.cpp:1891-2178. Iterates
  `renderPass -> shaderType -> mesh_shader -> [job] -> blendMode -> cullMode
  -> tessellation -> alphatest`. Relevant lines:
  - `transparency` bool: wiRenderer.cpp:1909.
  - PS selection: `GetPSTYPE(...)` call at wiRenderer.cpp:1942 (only call
    site in the whole file — safe to add a parameter with a default value).
  - `desc.bs` blend switch: wiRenderer.cpp:1952-1975 (by blendMode) then
    wiRenderer.cpp:1977-1987 (renderpass override for SHADOW/RAINBLOCKER only —
    RENDERPASS_MAIN not touched here, so blendMode's choice stands).
  - `desc.dss` switch: wiRenderer.cpp:1989-2023. RENDERPASS_MAIN case at
    1994-2003: `ADDITIVE → DEPTHREAD`, else `transparent ? DSSTYPE_TRANSPARENT : DSSTYPE_DEPTHREADEQUAL`.
  - `variant` construction: wiRenderer.cpp:2068-2076.
  - RENDERPASS_MAIN/PREPASS/PREPASS_DEPTHONLY PSO creation block:
    wiRenderer.cpp:2078-2108. Builds one `RenderPassInfo` with `rt_count=1`,
    `rt_formats[0] = format_rendertarget_main` (for RENDERPASS_MAIN), loops
    `msaa_support[] = {1,2,4,8}` creating one PSO per sample count, stored via
    `GetObjectPSO(variant)`.

  **Planned edit to this block**: add a new loop dimension
  `for (uint32_t oit = 0; oit <= (oit_capable ? 1u : 0u); ++oit)` wrapping
  from right after `const bool transparency = ...;` (line 1909) through the
  PSO-creation switch, where
  `bool oit_capable = renderPass == RENDERPASS_MAIN && transparency;`
  Inside, when `oit`:
  - `GetPSTYPE(..., oit)` picks the new `PSTYPE_OBJECT_OIT_PERMUTATION_BEGIN + shaderType` range.
  - override `desc.bs = &blendStates[BSTYPE_OIT];` (after the existing blendMode switch).
  - override `desc.dss = &depthStencils[DSSTYPE_DEPTHREAD];` (after the existing dss switch).
  - `variant.bits.oit = oit;`
  - in the RENDERPASS_MAIN branch of the PSO-creation switch, when `oit`:
    `renderpass_info.rt_count = 2; renderpass_info.rt_formats[0] = Format::R16G16B16A16_FLOAT; renderpass_info.rt_formats[1] = Format::R8_UNORM;`
    instead of the normal single-target formats (keep the same msaa_support loop).
  - Only touch the `case RENDERPASS_MAIN: case RENDERPASS_PREPASS: case RENDERPASS_PREPASS_DEPTHONLY:`
    block (wiRenderer.cpp:2080-2108) — PREPASS/PREPASS_DEPTHONLY never reach
    here with `oit=1` since `oit_capable` requires `renderPass == RENDERPASS_MAIN`
    and `transparency` (PREPASS already `continue`s on transparency at
    wiRenderer.cpp:1910-1911).
- `GetPSTYPE()` — wiRenderer.cpp:571-580 (571 is signature, 577-579 is the
  RENDERPASS_MAIN case). Add a `bool oit = false` default parameter; inside,
  `RENDERPASS_MAIN` case becomes:
  ```cpp
  case RENDERPASS_MAIN:
      realPS = SHADERTYPE((oit ? PSTYPE_OBJECT_OIT_PERMUTATION_BEGIN : (transparent ? PSTYPE_OBJECT_TRANSPARENT_PERMUTATION_BEGIN : PSTYPE_OBJECT_PERMUTATION_BEGIN)) + shaderType);
      break;
  ```
- Shader loading dispatch — wiRenderer.cpp:1185-1209. Two
  `wi::jobsystem::Dispatch(objectps_ctx, MaterialComponent::SHADERTYPE_COUNT, 1, ...)`
  blocks, one plain (1185-1195), one with `defines.push_back("TRANSPARENT")`
  (1197-1209), both compiling `"objectPS.cso"` (i.e. `objectPS.hlsl`) with
  per-shaderType defines from `MaterialComponent::shaderTypeDefines[args.jobIndex]`.
  **Add a third, identical block** after line 1209:
  ```cpp
  wi::jobsystem::Dispatch(objectps_ctx, MaterialComponent::SHADERTYPE_COUNT, 1, [](wi::jobsystem::JobArgs args) {
      auto defines = MaterialComponent::shaderTypeDefines[args.jobIndex];
      defines.push_back("TRANSPARENT");
      defines.push_back("OIT");
      LoadShader(
          ShaderStage::PS,
          shaders[PSTYPE_OBJECT_OIT_PERMUTATION_BEGIN + args.jobIndex],
          "objectPS.cso",
          ShaderModel::SM_6_0,
          defines
      );
  });
  ```
- Runtime PSO variant selection inside `RenderMeshes` — two sites:
  wiRenderer.cpp:3073-3090 (approx, single-draw path) and
  wiRenderer.cpp:3264-3290 (approx, wireframe/backside path — **check
  whether this second site is the wireframe-only path and doesn't need
  `oit` at all**, since wireframe rendering shouldn't go through OIT).
  Need to re-check these two sites' exact surrounding code (not fully read
  yet — do this first when resuming) to find where `variant.bits.blendmode`
  / `variant.bits.renderpass` get set per draw, and add
  `variant.bits.oit = (flags & DRAWSCENE_OIT) && renderPass == RENDERPASS_MAIN && variant.bits.blendmode != BLENDMODE_OPAQUE;`
  (or equivalent — must match `oit_capable` condition used at PSO-build time
  exactly, or `GetObjectPSO` will miss the cache and hit an invalid/empty PSO).
- `DrawScene()` flags — need to find the `DRAWSCENE_*` enum (grep
  `DRAWSCENE_TRANSPARENT` in wiRenderer.h) and add `DRAWSCENE_OIT` as a new
  bit; thread it through to `RenderMeshes` via the existing `flags` param
  (`DrawScene` already forwards `flags` into `RenderMeshes` at wiRenderer.cpp:7473).
- Shader-side entry point — `shaders/objectHF.hlsli`:
  - Entry signature at line 543: `float4 main(PixelInput input, in bool is_frontface : SV_IsFrontFace APPEND_COVERAGE_OUTPUT) : SV_Target`
    (PREPASS has its own signature above it, lines 536-541 — leave untouched).
    **Add an `#elif defined(OIT)` branch** with:
    ```hlsl
    struct PixelOutput_OIT
    {
        float4 accumulation : SV_Target0;
        float revealage : SV_Target1;
    };
    PixelOutput_OIT main(PixelInput input, in bool is_frontface : SV_IsFrontFace APPEND_COVERAGE_OUTPUT)
    ```
  - Return statement at lines 1140-1142 (`#else return color; #endif // PREPASS`).
    **Add an `#elif defined(OIT)` branch** before the plain `return color;`:
    ```hlsl
    #elif defined(OIT)
        PixelOutput_OIT Out;
        // McGuire & Bavoil weighted-blended OIT, "weight function 10":
        float oit_weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - dist * 0.9, 3.0), 1e-2, 3e3);
        Out.accumulation = float4(color.rgb * color.a, color.a) * oit_weight;
        Out.revealage = color.a;
        return Out;
    #else
        return color;
    #endif // PREPASS
    ```
    `dist` is already in scope at this point (used by `ApplyFog(dist, surface.V, color)` at
    line 1103) — it's the camera-space distance to the surface, good enough
    as the depth term for the weight heuristic. `color` at this point already
    has fog/saturation/lighting applied (lines 1077-1120), which is correct —
    OIT should accumulate the final shaded color same as the normal blend path.
  - `#define TRANSPARENT` region already exists (comment block at
    objectHF.hlsli:511-519 documents available switches) — add `OIT` to that
    doc comment when editing.
  - No changes needed to the shading logic itself (lighting, refraction,
    fog) — only the entry/exit points.
- Composite pass template — `PostProcess_MeshBlend_Resolve()` at
  wiRenderer.cpp:18860-18905 is the pattern to copy: `BindCommonResources`,
  `device->BindPipelineState(&PSO_X, cmd)`, push a `PostProcess` constant
  struct with resolution, `device->BindResource(&tex, slot, cmd)` for each
  input texture, then `device->RenderPassBegin(...); device->Draw(3,0,cmd); device->RenderPassEnd(...);`
  using a full-screen triangle (`VSTYPE_SCREEN`).
  Matching PSO creation pattern at wiRenderer.cpp:1584-1593
  (`PSO_mesh_blend_resolve`: `desc.vs = &shaders[VSTYPE_SCREEN]; desc.ps = &shaders[PSTYPE_POSTPROCESS_MESH_BLEND]; desc.bs = &blendStates[BSTYPE_TRANSPARENT]; desc.rs = &rasterizers[RSTYPE_DOUBLESIDED]; desc.dss = &depthStencils[DSSTYPE_DEPTHDISABLED]; device->CreatePipelineState(&desc, &PSO_mesh_blend_resolve);` —
  **note this uses the 2-arg `CreatePipelineState` (no explicit `RenderPassInfo`)**,
  confirming simple full-screen-triangle PSOs don't need explicit RT format
  declaration up front (unlike the big object-PSO permutation loop, which
  does specify it). Follow the same 2-arg pattern for the new composite PSO.
  **Planned**: add `PSTYPE_OIT_COMPOSITE` to `SHADERTYPE` enum (wiEnums.h,
  near `PSTYPE_POSTPROCESS_MESH_BLEND` line 200), a new
  `shaders/oit_composite.hlsl` pixel shader (reads accumulation + revealage
  as two bound textures via `BindResource` slots, outputs
  `float4(accum.rgb / max(accum.a, 1e-5), 1.0 - revealage)` — this reuses
  `BSTYPE_TRANSPARENT`'s existing SRC_ALPHA/INV_SRC_ALPHA blend equation
  correctly, see derivation below), a `PSO_oit_composite` PSO built exactly
  like `PSO_mesh_blend_resolve` above, and a new
  `void Composite_OIT(const Texture& accumulation, const Texture& revealage, CommandList cmd)`
  function mirroring `PostProcess_MeshBlend_Resolve`.

  **Blend derivation** (already verified correct): we want
  `result = (accum.rgb/accum.a)*(1-revealage) + background*revealage`.
  Standard "over" blend is `result = src*srcAlpha + dst*(1-srcAlpha)`. Setting
  `srcAlpha = 1-revealage` and `src.rgb = accum.rgb/accum.a` gives exactly
  the wanted formula — i.e. **the existing `BSTYPE_TRANSPARENT` blend state
  (SRC_ALPHA, INV_SRC_ALPHA, ADD) is exactly what's needed, no new blend
  state required for the composite step.**

## Render target + render path wiring (wiRenderPath3D.h / .cpp) — NOT YET INVESTIGATED IN DETAIL

Still to do when resuming:
1. Find where `rtMain`, `rtMain_render`, `depthBuffer_Main`, `rtSceneCopy`
   etc. are declared/created in `wiRenderPath3D.h`/`.cpp` (grep for
   `rtSceneCopy = {}` region, wiRenderPath3D.cpp:28 area, and the
   `device->CreateTextureZeroed(&desc, &rtSceneCopy)` creation code around
   line 184) and add `rtOIT_accumulation` / `rtOIT_revealage` textures there,
   sized at internal resolution, created only when OIT is enabled (check for
   an existing pattern of conditionally-created resources, e.g. how
   `rtShadow`/`motionblurResources` are conditionally created based on a
   toggle — wiRenderPath3D.cpp:679-682 shows the pattern:
   `else { motionblurResources = {}; }`).
2. The transparent draw call site is wiRenderPath3D.cpp:2156-2203 (function
   containing this was being read but not fully identified — re-locate via
   `grep -n "DRAWSCENE_TRANSPARENT" wiRenderPath3D.cpp` → lines 1381, 2177,
   2191, 2739). The block at 2156-2203 is inside the function that also
   handles ocean rendering (2137-2154) and `RenderSceneMIPChain` (2158) for
   screen-space refraction. It currently:
   - Begins one `RenderPassBegin(rp, rp_count, cmd)` targeting `rtMain_render`
     (+ resolve target if MSAA) + `depthBuffer_Main` (LOAD/STORE, full
     read-write) — this same renderpass stays open through transparent,
     foreground, debug world, wireframe, light visualizers, soft particles,
     gaussian splats, sprites, volumetric light upsample, light shafts, lens
     flares, ending at `device->RenderPassEnd(cmd)` (line 2249).
   - Two `DrawScene(..., DRAWSCENE_TRANSPARENT | ...)` calls: "Regular"
     (2173-2181) and "Foreground" (2187-2194), each with a different
     viewport depth range but both writing directly into the currently bound
     `rtMain_render`.

   **Planned wiring**: guard with `if (GetOITEnabled()) { ... } else { <existing code unchanged> }`.
   In the OIT branch:
   a. Open a **separate** `RenderPassBegin` targeting
      `rtOIT_accumulation` (LoadOp::CLEAR, clear value `{0,0,0,0}` — set via
      `TextureDesc::clear` at creation time, see wiGraphics.h:552-573) and
      `rtOIT_revealage` (LoadOp::CLEAR, clear value `{1,1,1,1}`), plus
      `depthBuffer_Main` bound **read-only**
      (`RenderPassImage::DepthStencil(&depthBuffer_Main, LoadOp::LOAD, StoreOp::STORE, ResourceState::DEPTHSTENCIL, ResourceState::DEPTHSTENCIL_READONLY, ResourceState::DEPTHSTENCIL)` —
      layout_after must be `DEPTHSTENCIL` again so the next renderpass, which
      expects `layout_before == DEPTHSTENCIL`, isn't surprised).
   b. Call `DrawScene(visibility_main, RENDERPASS_MAIN, cmd, DRAWSCENE_TRANSPARENT | DRAWSCENE_TESSELLATION | DRAWSCENE_OCCLUSIONCULLING | DRAWSCENE_MAINCAMERA | DRAWSCENE_OIT)`
      (regular) then the foreground-viewport variant with `DRAWSCENE_FOREGROUND_ONLY | DRAWSCENE_OIT` instead of `DRAWSCENE_TESSELLATION | DRAWSCENE_OCCLUSIONCULLING`
      (matching the existing two calls) — both writing into the *same*
      accumulation/revealage targets (this is fine/expected: WBOIT
      accumulation is order-independent, so regular + foreground transparent
      geometry can share one accumulation buffer with no extra composite step).
   c. `RenderPassEnd(cmd)`.
   d. Re-open the existing main `RenderPassBegin(rp, rp_count, cmd)` (same as
      today, targeting `rtMain_render` + depth) and, as the **first** thing
      inside it (before debug world / particles / etc., so later
      alpha-blended effects like lens flares still layer on top correctly),
      call the new `Composite_OIT(rtOIT_accumulation, rtOIT_revealage, cmd)`
      — this does **not** need its own `RenderPassBegin`/`End` since it's a
      plain triangle draw into the already-bound `rtMain_render`, unlike
      `PostProcess_MeshBlend_Resolve` which opens its own pass externally
      (that function takes `renderpass_images`/`count` as parameters *because*
      it's called standalone elsewhere — here we want to draw inside an
      already-open pass, so skip the Begin/End wrapper, just bind
      PSO/resources and `Draw(3,0,cmd)` directly).
   e. Continue the rest of the block (debug world, wireframe, particles,
      etc.) completely unchanged.
   f. Everything else in the function (ocean block 2137-2154,
      `RenderSceneMIPChain` 2158, the distortion-particles block after
      `RenderPassEnd` at 2249, `Postprocess_Downsample4x` at 2301) stays as-is.
3. Need to identify the exact enclosing function name/signature (was mid-read,
   stopped around line 2100-2300) — re-open wiRenderPath3D.cpp around line
   2000-2100 to find the function header this block belongs to.
4. Public toggle: add `void SetOIT(bool value); bool GetOITEnabled();` to
   `wiRenderer.h` (mirror an existing simple bool toggle's declaration style —
   e.g. search `SetOcclusionCullingEnabled`/`GetOcclusionCullingEnabled` for
   the exact pattern used in this codebase) and the matching static bool +
   functions in `wiRenderer.cpp`.
5. Decide whether the toggle should also be exposed in the Editor UI
   (`Editor/` — not investigated at all yet). Given "no huge changes," a
   first pass can skip editor UI and only expose the C++ toggle; adding an
   editor checkbox is a small separate follow-up if wanted.

## Open questions / risks to resolve when resuming

- Confirm `RenderPassInfo` struct fields (`rt_count`, `rt_formats[]`,
  `ds_format`, `sample_count`) exact names/types — only inferred from usage
  in the existing loop (wiRenderer.cpp:2084-2106), not read directly from
  its declaration yet.
- No shader compiler (`dxc`/`glslangValidator`) is available in this
  environment, and no GPU — the HLSL changes to `objectHF.hlsli` cannot be
  compiled or run here. Treat the shader edit as carefully-reasoned but
  **unverified**; flag this clearly to the user and recommend they build +
  run locally (DX12/Vulkan) before relying on it. `cmake`/`g++` are
  available, so at minimum a C++-only configure/compile pass of
  `wiRenderer.cpp`/`wiRenderPath3D.cpp` should be attempted to catch syntax
  errors in the native-side changes.
- Double-check the second `ObjectRenderingVariant variant = {}` site
  (~wiRenderer.cpp:3264) really is wireframe-only (in which case it must
  NOT set `.oit`) — this wasn't confirmed, only guessed from the earlier
  grep showing `PSO_object_wire*` symbols nearby.
- Verify stencil is intentionally not written during the OIT pass (reusing
  `DSSTYPE_DEPTHREAD` disables stencil entirely, whereas the existing
  `DSSTYPE_TRANSPARENT` writes stencil ref `STENCILREF_DEFAULT` for every
  transparent pixel — used for outline/selection highlighting in the editor
  and possibly TAA masking). This is a known, minor behavior change for
  transparent objects specifically when OIT is enabled — call it out to the
  user rather than silently accepting it.

## Resume checklist (in order)

1. Read `RenderPassInfo` struct definition (wiGraphics.h) to confirm field names.
2. Re-locate the two `ObjectRenderingVariant variant = {}` sites inside
   `RenderMeshes` (~3073, ~3264) and read enough surrounding code to know
   exactly how/where to set `.oit`.
3. Re-locate the enclosing function name for the wiRenderPath3D.cpp:2100-2250
   block (grep for the function signature above line ~2000).
4. Find `SetOcclusionCullingEnabled`/`GetOcclusionCullingEnabled` (or similar)
   in `wiRenderer.h`/`.cpp` as the toggle template to copy.
5. Make the `wiEnums.h` edits (PSTYPE_OBJECT_OIT_PERMUTATION_*, BSTYPE_OIT,
   PSTYPE_OIT_COMPOSITE, DRAWSCENE_OIT — the last one lives in `wiRenderer.h`,
   not wiEnums.h, find its enum there).
6. Make the `wiRenderer.cpp` edits: `ObjectRenderingVariant.oit` bit,
   `GetPSTYPE` oit param, blend state `BSTYPE_OIT` construction, PS-loading
   dispatch block, big PSO-precompile loop `oit` dimension, `RenderMeshes`
   variant.bits.oit wiring, new `PSO_oit_composite` + `Composite_OIT()`
   function (modeled on `PostProcess_MeshBlend_Resolve`), `SetOIT`/`GetOITEnabled`.
7. Make the `shaders/objectHF.hlsli` edits (entry signature + return branch).
8. Write `shaders/oit_composite.hlsl` (new file — reads accumulation +
   revealage via `BindResource` slots 0/1, outputs the composited color;
   model its resource-binding conventions on an existing simple postprocess
   shader, e.g. whatever `mesh_blendPS.cso`'s source file is — find it by
   grepping `PSTYPE_POSTPROCESS_MESH_BLEND` shader source filename).
9. Make the `wiRenderPath3D.h`/`.cpp` edits: new render targets, new
   renderpass block, composite call, guarded by `GetOITEnabled()`.
10. Attempt a C++ compile (cmake configure + build of at least the
    `WickedEngine` static lib target) to catch syntax errors — no shader
    compiler available, so HLSL stays unverified.
11. Report to the user clearly which parts are implemented-but-unverified
    (shader math/output correctness needs a real GPU run) vs. structurally
    verified (C++ compiles).

## Branch / worktree info

- Worktree path: `.claude/worktrees/oit-transparency/WickedEngine`
- Branch: `worktree-oit-transparency`
- Base: branched from `wip/sdl2-v3` (the user's working branch at the time)
- This file is the only change committed so far.
