# Project 3: Ants Vs. SomeBees — Code Analysis

> CS 61A (Fall 2024) tower-defense project. Endpoint: `cal/cs61a/fa24/ants`.
> This document maps the architecture, the game loop, and every problem stub in [ants.py](ants.py).
> **Current state: unsolved starter code.** Only `HarvesterAnt` and `ThrowerAnt` have `implemented = True`; all `*** YOUR CODE HERE ***` stubs are still present.

---

## 1. Overview

You build a tower-defense game where **Ants** (your colony) defend a tunnel against waves of **Bees**. Ants are deployed using *food*; bees advance one `Place` per turn toward the `AntHomeBase`, stinging any ant blocking the path. Win by killing every bee in the assault plan; lose if a bee reaches home base or (Problem 12) the Queen dies.

The project is deliberately built around three OOP themes:

| Theme | Where it shows up |
|---|---|
| Inheritance & class vs. instance attributes | `Insect` → `Ant`/`Bee` → subclasses; `damage`, `food_cost`, `implemented` |
| Polymorphism / method overriding | `action`, `reduce_health`, `add_to`, `throw_at`, `can_contain` |
| Cooperative multi-class design | `ContainerAnt` + `Place.add_insect` + `Ant.remove_from` interplay |

---

## 2. File Layout

```
Project/ants/
├── ants.py           # THE assignment file — all problems live here
├── ants_plans.py     # Difficulty presets (assault plans) + CLI game-state factory
├── gui.py            # Flask + Socket.IO web GUI (do not edit)
├── ucb.py            # CS61A debugging helpers (trace, interact)
├── ok                # CS61A autograder binary
├── ants.ok           # Autograder config (tests/*.py → ok_test)
├── tests/            # 00.py … 12.py, 08a/08b/08c.py, EC1–EC4, optional1/2
├── libs/             # Vendored Flask, Jinja2, Socket.IO, etc.
├── templates/index.html
└── static/           # script.js, socket.io.min.js, style.css, assets/
```

---

## 3. Class Hierarchy

```
Place ──────────────► Water            (only waterproof insects)
  └── AntHomeBase                      (game-over trigger for bees)

Insect  (health, place, id, damage)
 ├── Ant                              (implemented=False by default)
 │    ├── HarvesterAnt      ✔ implemented   food_cost=0  (+1 food/turn)
 │    ├── ThrowerAnt        ✔ implemented   food_cost=3  damage=1
 │    │    ├── ShortThrower             range ≤ 3
 │    │    ├── LongThrower              range ≥ 5
 │    │    ├── QueenAnt                 doubles ants behind her
 │    │    ├── SlowThrower     (EC1)
 │    │    ├── ScaryThrower    (EC2)
 │    │    └── LaserAnt        (EC4)    pierces all insects in front
 │    ├── FireAnt                       AoE damage on death
 │    ├── WallAnt                       high-HP blocker
 │    ├── HungryAnt                     eats a bee, then cooldown
 │    ├── NinjaAnt          (EC3)       ignores blocking, hits all locally
 │    ├── ContainerAnt  is_container=True
 │    │    ├── BodyguardAnt             guards one ant
 │    │    └── TankAnt                  guards + damages bees each turn
 │    └── ScubaThrower                  waterproof ThrowerAnt
 └── Bee  (is_waterproof=True, damage=1)
      ├── Wasp            damage=2
      │    └── Boss       damage_cap=8  (per-hit damage capped)
      └── (Slow / Scary states applied at runtime)
```

`ant_types()` walks `Ant.__subclasses__()` recursively and returns only classes with `implemented = True` — that flag is what makes an ant selectable in the GUI.

---

## 4. Core Mechanics

### Placement (`Place.add_insect` → `Insect.add_to`)
- `Place.add_insect` delegates to the insect's `add_to`, letting each subclass customize insertion.
- `Ant.add_to` puts itself in `place.ant`; if occupied it asserts unless a container handles it (**Problem 8b**).
- `Bee.add_to` appends to `place.bees` (a list — many bees per place).
- `Water.add_insect` (**Problem 10**) must kill non-waterproof insects on entry.

### Containment (`ContainerAnt`)
- `can_contain` / `store_ant` / `remove_ant` (**Problem 8a**) manage `self.ant_contained`.
- `ContainerAnt.remove_from` is special-cased: when the container dies, the **contained ant survives** and becomes `place.ant`.
- `ContainerAnt.action` forwards the turn to the contained ant.

### Turn order (`GameState.simulate` — generator driving the GUI)
```
loop:
  1. beehive.strategy()      → spawn this time-step's bees at random entrances
  2. yield                   → player deploys ants
  3. ants_take_actions()     → every living ant acts
  4. time += 1
  5. yield                   → animation pause
  6. bees_take_actions(n)    → each living bee acts; dead bees removed;
                               if n == 0 raise AntsWinException
```
- A blocked bee (ant in the way) stings; otherwise it moves to `place.exit`.
- `AntHomeBase.add_insect` raises `AntsLoseException` when a bee arrives.
- `Boss.reduce_health` caps each hit at `damage_cap` (8) — designed to foil one-shot strategies.

### Health & death
`Insect.reduce_health(amount)` decrements health, and on `<= 0` calls `zero_health_callback()` then asks its place to remove it. Subclasses hook both:
- `FireAnt.reduce_health` (**P5**) — damage every bee in the place, plus a death burst.
- `QueenAnt.reduce_health` (**P12**) — losing the queen ends the game.

---

## 5. Problem Map

| # | Points | What to build | Location |
|---|---|---|---|
| 0 | 0 | Warm-up / setup | — |
| 1 | 1 | `HarvesterAnt.action` — `+1 food` | [ants.py:147](ants.py#L147) |
| 2 | 2 | `Place.__init__` — link `exit.entrance = self` | [ants.py:16](ants.py#L16) |
| 3 | 2 | `ThrowerAnt.nearest_bee` — nearest bee in range | [ants.py:165](ants.py#L165) |
| 4 | 2 | `ShortThrower` / `LongThrower` range limits | [ants.py:197](ants.py#L197) |
| 5 | 3 | `FireAnt.reduce_health` — AoE + death damage | [ants.py:234](ants.py#L234) |
| 6 | 2 | `WallAnt` class | [ants.py:246](ants.py#L246) |
| 7 | 2 | `HungryAnt` — eat one bee, cooldown | [ants.py:250](ants.py#L250) |
| 8a | — | `ContainerAnt.can_contain/store_ant/action` | [ants.py:264](ants.py#L264) |
| 8b | — | `Ant.add_to` container branch | [ants.py:119](ants.py#L119) |
| 8c | — | `BodyguardAnt` (`implemented`, `health`) | [ants.py:295](ants.py#L295) |
| 9 | 3 | `TankAnt` | [ants.py:306](ants.py#L306) |
| 10 | 1 | `Water.add_insect` — drown non-waterproof | [ants.py:313](ants.py#L313) |
| 11 | 2 | `ScubaThrower` — waterproof thrower | [ants.py:321](ants.py#L321) |
| 12 | 3 | `QueenAnt` — double damage behind, lose on death, single queen | [ants.py:325](ants.py#L325) |
| EC1 | 0 | `SlowThrower` + `Bee.slow` | [ants.py:356](ants.py#L356) |
| EC2 | 0 | `ScaryThrower` + `Bee.scare` | [ants.py:371](ants.py#L371) |
| EC3 | 0 | `NinjaAnt` + non-blocking `Bee.blocked` | [ants.py:386](ants.py#L386) |
| EC4 | 0 | `LaserAnt` — damage falloff by distance | [ants.py:403](ants.py#L403) |
| Opt1/2 | 0 | Optional extensions | — |

Default graded suite: `00–12` (per [ants.ok](ants.ok)); EC problems are unlockable extras.

---

## 6. Notable Gotchas

- **`Place.entrance` vs `Place.exit`** — entrances are set either by the layout builder or, for bee entrances, directly to the `Hive` in `GameState.configure`.
- **Mutating `place.bees` while iterating** — FireAnt/LaserAnt tests explicitly ask "can you iterate over a list while mutating it?" (answer: no — iterate over a copy).
- **`Ant.double()`** (§P12) must be idempotent — doubling twice must not stack; typically guarded with an `is_doubled` flag.
- **`random_bee`** in `ThrowerAnt.nearest_bee` is a placeholder to be replaced — it returns a random bee, not the nearest.
- **`Boss` damage cap** means high single-hit damage is wasted past 8; sustained damage or HungerAnt-style removal is needed.
- **`remove_ant` assertion** — containers must raise the documented `assert False` messages, not silently no-op.

---

## 7. Running It

```bash
# Run the autograder (from the ants/ directory)
python ok -q 01          # single problem
python ok               # all unlocked tests
python ok --local       # grade locally, no network

# Play in the browser (GUI uses Flask + websockets)
python gui.py -d easy    # test | easy | normal | hard | extra-hard
python gui.py -d normal -w --food 10   # wet layout, extra starting food
```

`ants_plans.py` builds the `AssaultPlan` per difficulty and returns a ready `GameState` from `create_game_state()`.
