# Tutorial: your first BotStadia bots

This takes you from a clean machine to three working bots for **dance-off**: two humanoid robots dancing side by side, scored on hitting each card's pose on the beat. You'll build, run, check and benchmark a Rust bot and a C bot, then train a policy and do the same.

It takes about half an hour, most of it downloads and training.

## 1. Install the CLI

macOS on Apple Silicon, or Linux on x86_64 or arm64:

```sh
curl -fsSL https://cdn.botstadia.mediabucket.io/install.sh | sh
```

It installs `botstadia` in `~/.botstadia/bin`, checking the download against its checksum, and tells you the line to add to your `PATH`. Then clone this template and check your machine:

```sh
git clone https://github.com/botstadia/botstadia-template my-bots
cd my-bots
botstadia doctor
```

`doctor` lists what's ready and how to fix what isn't. Only the language toolchains you'll use are needed.

You'll also want [Task](https://taskfile.dev) for the shortcuts below. Every shortcut is just a `botstadia` command, so you can skip it.

## 2. Add the toolchains

```sh
task setup
```

That's three commands, which you can also run one at a time:

- `botstadia setup --lang rust` adds Rust's `wasm32-wasip2` target. Rust itself comes from [rustup](https://rustup.rs).
- `botstadia setup --lang c` downloads wasi-sdk, the C compiler for WebAssembly.
- `botstadia setup --lang policy`, in `bots/policy`, makes a Python environment there with `botstadia-train`, Stable-Baselines3 and PyTorch. It needs Python 3.11 or later.

## 3. Build and check the Rust bot

```sh
cd bots/rust
botstadia build
botstadia check
```

`build` compiles the bot and writes its bundle, `out/my-rust.zip`: the exact file you'd upload. `check` runs the same checks upload verification does: the bundle is well formed, the bot starts within its init budget, and it plays 200 turns within its fuel budget. It also shows how much fuel a turn takes.

Open `src/lib.rs`. A bot is two functions: `init`, once before the first turn, and `on_tick`, each turn, which takes this turn's input and returns its output. `src/interface.rs` is generated from the arena's declaration and names every value: `input.joints`, `input.cards`, `cards::NEXT_SECONDS` and so on.

## 4. Play a session

```sh
botstadia run --against scripted
```

That's a real session on the same session host enclaves run, with fuel budgets on. It prints the result and writes the replay and metrics to `out/runs/seed-1/`. The same seed always gives the same session, so `--seed 7` gives a different routine.

To see inside your bot:

- `--bot-logs` shows everything it prints to stderr, tagged with its seat and turn. Uncomment the `eprintln!` in `src/lib.rs` to try it.
- `--step` pauses after every turn. Press Enter for the next, a number to run to that turn, `c` to run to the end, or `q` to stop.
- `--until-turn 500` runs to turn 500, then pauses the same way.

Pausing sits between turns, so it costs your bot nothing.

## 5. Benchmark it

```sh
botstadia bench
```

`bench` plays your bot against every bot published for the arena, 20 sessions each with your seat drawn per session, and prints win rates and ratings. The starter beats `noop` and `random` every time and loses most sessions to `scripted`.

## 6. The C bot

```sh
cd ../c
botstadia build && botstadia check && botstadia bench
```

`bot.c` is the same strategy as the Rust bot, and benches the same. `interface.h` names every value, and `agent.h` and `agent.c` are the bot interface, generated from BotStadia's WIT. Use any language that compiles to a WebAssembly component the same way.

## 7. Train the policy bot

```sh
cd ../policy
.venv/bin/python train.py --steps 1000000
```

Read `train.py` first. It's short, and yours to change:

1. **Imitation.** It records the arena's `scripted` bot over 40 sessions and fits the policy network to what it did. Most sessions push the robot a little off course while keeping the scripted bot's choices as the answers, so the network learns to recover rather than to follow one path exactly.
2. **Reinforcement.** PPO plays against the published bots on the arena's shaping reward plus the outcome, training the critic alone for a while first so it doesn't undo the imitation.
3. **Export.** It writes `policy.onnx` and checks it twice: PyTorch against ONNX Runtime, and against the real policy shell over a local session.

Imitation alone takes a couple of minutes and already beats `noop` and `random`. A million steps of PPO take five minutes or so on a recent laptop. Then:

```sh
botstadia build && botstadia check && botstadia bench
```

`build` wraps `policy.onnx` with the policy shell, the published WebAssembly program that runs every trained policy, so you write no code at all. Inference costs fuel like any other work, and `check` shows how much.

`BotstadiaVectorEnv` is a standard Gymnasium vector environment, so you can train with any library. `env.sb3()` gives the same sessions as a Stable-Baselines3 environment, and `export.torch_module` exports any PyTorch network.

## 8. Make it yours

- **Name it.** A bot's name is in its `bot.toml`. Names are yours alone, so change `my-rust` and the rest before you upload.
- **Another arena.** `botstadia create-bot my-racer --arena racing --kind rust` makes a new bot project for any arena.
- **A new arena version.** `botstadia create-bot --refresh` in a bot's folder rewrites its generated files and never touches your own.

Uploading isn't in this release of the CLI yet. Everything here is the same local loop you'll use once it is.
