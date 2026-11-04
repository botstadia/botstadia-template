# BotStadia bot template

Three starter bots for [BotStadia](https://botstadia.com)'s dance-off arena, one of each kind a bot can be:

| Folder | Kind | What it is |
|---|---|---|
| `bots/policy` | Trained policy | An ONNX network you train in Python with `botstadia-train`, run by BotStadia's policy shell |
| `bots/rust` | Rust | A program compiled to a WebAssembly component |
| `bots/c` | C | The same program in C, built with wasi-sdk |

The Rust and C bots play the same simple strategy: read the board and glide into each card's pose, arriving on the beat. The policy bot starts by imitating the arena's published `scripted` bot, then improves on it with reinforcement learning. Every one of them is meant to be beaten.

**Start with [TUTORIAL.md](TUTORIAL.md).** It goes from a clean machine to a trained, checked and benchmarked bot.

## The everyday commands

Everything is the `botstadia` CLI, run in a bot's folder. The Taskfile runs them over all three bots, or one with `BOT=rust`:

```sh
task doctor            # is this machine ready?
task setup             # every toolchain the bots need
task train             # train the policy bot
task build             # build every bot into its out/<name>.zip
task check             # the checks upload verification runs
task bench             # win rates and ratings against the published bots
task run BOT=c         # one local session, with its replay and metrics
```

## Another arena

`botstadia create-bot <name> --arena racing --kind rust` makes a bot for any arena, with its inputs and outputs named in a generated interface file. `botstadia create-bot --refresh` in a bot's folder rewrites the generated files for the arena's newest version and never touches your own.

## Licence

MIT or Apache-2.0, at your option. Your bots are yours.
