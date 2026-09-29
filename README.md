# Social Behavior Framework (SBF)

Production-ready Unreal Engine 5.7 / 5.8 plugin implementing three interconnected
NPC subsystems unified by a single editor window and a single runtime facade:

| Subsystem | Purpose |
|-----------|---------|
| **Social Graph** (`USBF_SocialGraph`) | Hierarchy of NPC relationships (father/son, boss/subordinate, ...) with inherited ally/enemy logic and colored in-world visualization. |
| **Behavior Map** (`USBF_BehaviorMap`) | Time-driven schedule (Home / Work / Leisure) with a configurable time scale and per-weekday leisure overrides. |
| **Character Trait** (`USBF_CharacterTrait`) | GameplayTag-driven reaction hierarchy (`Threat.Life`, `Work.Late`, `Offender.Detected`, ...) with priority/weight selection and optional override subtrees. |

Runtime facade: `USBF_ManagerSubsystem` (World Subsystem). Editor UX: `Window -> Social Behavior Framework`.

---

## 1. Installation

1. Copy the `SocialBehaviorFramework` folder into your project's `Plugins/` directory
   (`MyProject/Plugins/SocialBehaviorFramework`).
2. Regenerate project files and rebuild:
   - **Windows**: `GenerateProjectFiles.bat`, then build `MyProjectEditor Win64 Development`.
   - **Linux / Mac**: `./GenerateProjectFiles.sh` and build accordingly.
3. In `Edit -> Plugins`, enable **Social Behavior Framework** (AI category) and restart the editor.
4. The plugin ships `Config/DefaultGameplayTags.ini` with the default tag hierarchy
   (`SBF.Relation.*`, `SBF.Trait.*`, `Threat.*`, `Work.*`, `Offender.*`, `Ally.*`).
   Custom tags must be registered there (or in your project's `DefaultGameplayTags.ini`)
   before they can be referenced.

Supported platforms: Win64, Linux, Mac (desktop). Unreal Engine 5.7 and 5.8.

## 2. Project Settings

`Project Settings -> Game -> Social Behavior Framework`:

- `RealSecondsPerGameMinute` - one game minute per N real seconds (default 1.0).
- `StartHour` / `StartMinute` / `StartDay` - simulated clock start.
- `bEnableWeekdays` + `WeekendDays` - weekend days skip work schedules (default Sat/Sun).
- `TimeScale`, `bPaused` - global clock multiplier / freeze.
- `DefaultSocialGraph` / `DefaultBehaviorMap` / `DefaultCharacterTrait` - fallback assets
  used when an NPC component does not override them.

## 3. Quick Start

### 3.1 Social Graph
1. Open `Window -> Social Behavior Framework`, tab **Social Graph**.
2. Click **New Graph** (creates `/Game/SBF/SBF_Graph` with default Friend/Enemy/Family relations).
3. Place 3+ actors (NPC pawns) in the level, select one at a time and use the canvas
   right-click menu **Add NPC Node** (auto-binds to the selected actor) or
   **Bind to Selected Actor** on an existing node.
4. Right-click a node: **Set Parent** (father/son), **Relation** (assign a relation asset),
   **Create Edge To** (explicit relationship).
5. Edit a relation asset (e.g. `SBF_Relation_1`): use the **Ally / Enemy / Neutral** buttons
   in its details panel (self-inclusion convention), or fill `AllyTags` / `EnemyTags` manually.
6. Enable **Draw in Viewport** in the toolbar: edges appear between NPCs colored by
   resolved hostility (green = ally, red = enemy, gray = neutral), both in the editor
   viewport and in PIE.
7. **Save** the graph; `Validate` checks cycles, dangling references and missing relations.

### 3.2 Behavior Map
1. Tab **Behavior Map**: **New Map** -> `SBF_BehaviorMap`.
2. Set `HomeLocation`, `WorkLocation`, `LeisureLocation` and the `WorkHours` range
   (default 09:00-18:00). Add `LeisureByWeekday` overrides when needed.
   The timeline preview shows the full week.
3. Add an `USBF_NPCComponent` to your NPC (details panel "SBF Assets" section) and assign
   the map (or set the project default in Project Settings).
4. Press **Preview in PIE**: the NPC moves Home -> Work -> Leisure automatically
   (`bAutoMove`), following `1 game minute = RealSecondsPerGameMinute` real seconds.
5. The component writes `BB_TargetLocation` (Vector) and `BB_CurrentActivity` (Name) into
   the AI blackboard and broadcasts `OnScheduleChanged`.

### 3.3 Character Trait
1. Tab **Character Trait**: **New Trait** (`SBF_Trait`) and **New Library**
   (`SBF_TagLibrary`); the library is pre-filled with the default trigger tags
   (Add / Remove / Rename / Load Defaults in the tag list).
2. In the trait's details view add reactions: `TriggerTag = Threat.Life`,
   `Priority = 100`, optional `OverrideSubtree` and `BlackboardKeysToSet` (e.g. `bThreatened`).
3. Assign the trait to the NPC component (details panel or project default).
4. In gameplay (PIE, console, Blueprint or a BT task) call
   `USBF_ManagerSubsystem::NotifyTag(NPC, "Threat.Life")` - the NPC's
   `USBF_CharacterTraitComponent` fires the best reaction (Priority desc, Weight desc).
5. In the Behavior Tree, `SBF Apply Trait` writes the reaction keys and pushes the
   `OverrideSubtree` on the execution stack until it finishes.

## 4. Behavior Tree Nodes

All nodes appear in the BT context menu under the **SBF** category with descriptions:

| Node | Type | Behavior |
|------|------|----------|
| `BTTask_SBF_EvaluateSocial` | Task | Writes `BB_IsHostile`, `BB_IsAlly`, `BB_RelationTag` for a blackboard target actor. |
| `BTTask_SBF_UpdateSchedule` | Task | Forces a schedule re-evaluation, writes `BB_TargetLocation` / `BB_CurrentActivity`. |
| `BTTask_SBF_ApplyTrait` | Task | Applies the best pending trait reaction (keys + optional override subtree). |
| `BTDecorator_SBF_IsHostile` | Decorator | Gates on hostile stance towards the blackboard target. |
| `BTDecorator_SBF_IsAlly` | Decorator | Gates on allied stance towards the blackboard target. |
| `BTDecorator_SBF_TagReaction` | Decorator | Gates on a pending trait reaction (with `bInverse`). |
| `BTService_SBF_TimeSync` | Service | Periodically writes `BB_GameHour`, `BB_Weekday`, `BB_IsWeekend`. |

## 5. Hostility Inheritance Rules

- A **direct** edge (hierarchy or explicit) between two nodes short-circuits the query.
- **Hostility propagates** up the chain to the nearest common ancestor: an enemy hop
  anywhere on the path makes the pair hostile.
- **Ally stance propagates** only along an unbroken chain of relations with
  `bInheritsHostility = true` ("ally of my father is my ally").
- A relation asset marks its own stance by including its `RelationTag` in `AllyTags`
  (allied) or `EnemyTags` (hostile) - the details panel buttons do this for you.
- Results are cached per graph (`TMap<TPair<FGuid,FGuid>, ESBF_Hostility>`) and
  invalidated on any structural change. Memory footprint per 1000 NPCs stays well
  under 1 MB (one graph cache + weak component registrations, no per-NPC timers).

## 6. Unit Tests

Automation tests (module `SocialBehaviorFrameworkTests`):

- `SBF.SocialGraph.HostilityInheritance` - direct stance, inherited ally, unconditional
  hostility propagation, broken inheritance, explicit-edge override, cache invalidation,
  manager facade queries with spawned actors.
- `SBF.TimeSubsystem.DayWeekBoundaries` - minute/hour/day delegates, pause and zero-scale
  freeze, midnight rollover, weekend detection.

Run them from:

- `Session Frontend -> Automation -> SBF.*`,
- the editor console: `Automation RunTests SBF`,
- the command line:
  `UnrealEditor.exe MyProject.uproject -ExecCmds="Automation RunTests SBF; Quit" -unattended -nop4 -nullrhi`.

## 7. Localization

All UI strings use `LOCTEXT` (English). Run `Localization -> Gather Text` in the editor
to produce localization targets; add Russian (and other) translations to the resulting
`.po`/manifest files. A RU string table is a drop-in for the gathered manifest.

## 8. Code Notes

- Doxygen-style comments on all public APIs; `LogSBF` log category for diagnostics.
- No per-NPC timers or polling: schedules react to `OnMinuteTick`, traits to tag
  notifications; BT services are the only periodic nodes.
- `USBF_TimeSubsystem` is a `UTickableWorldSubsystem` that only advances the clock while
  unpaused; all NPCs share one clock.
- Editor visualization (`FSBF_Visualizer`) draws in the editor viewport via the line
  batcher and in PIE via `DrawDebugLine`.
