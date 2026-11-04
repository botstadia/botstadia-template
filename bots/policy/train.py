"""Train a policy for dance-off, export it, and check it runs in the shell.

This file is yours: `botstadia create-bot --refresh` never touches it.

    python train.py --steps 2000000

1. Imitation: record the arena's published `scripted` bot and fit the policy
   to it, a warm start far ahead of random (`--teacher none` skips it).
2. Reinforcement: PPO against the published baselines, on the arena's
   shaping reward plus the outcome, the critic alone first.
3. Export: policy.onnx, checked against ONNX Runtime and the real policy
   shell. `botstadia build` then bundles it, and `botstadia bench` measures it.
"""

import argparse

from stable_baselines3 import PPO
from stable_baselines3.common.vec_env import VecNormalize

from botstadia_train import BotstadiaVectorEnv, export, imitation, opponents

ARENA = "dance-off"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--steps", type=int, default=1_000_000)
    parser.add_argument("--envs", type=int, default=8)
    parser.add_argument("--teacher", default="scripted", help="a published bot or a bundle to imitate first, or none")
    parser.add_argument("--record-seeds", type=int, default=40)
    parser.add_argument("--critic-steps", type=int, default=300_000)
    args = parser.parse_args()

    # Every seat but yours is a published baseline, drawn per episode.
    env = BotstadiaVectorEnv(ARENA, num_envs=args.envs, opponents=opponents.published(ARENA))
    venv = VecNormalize(env.sb3(), clip_obs=10.0)
    model = PPO(
        "MultiInputPolicy",
        venv,
        learning_rate=5e-5,
        n_steps=1024,
        batch_size=4096,
        n_epochs=5,
        clip_range=0.1,
        target_kl=0.02,
        policy_kwargs={"net_arch": {"pi": [256, 256], "vf": [256, 256]}, "log_std_init": -1.6},
        verbose=1,
    )
    if args.teacher != "none":
        observations, actions = imitation.record(env, args.teacher, seeds=args.record_seeds)
        imitation.fit_normalizer(venv, observations)
        imitation.warm_start(model, venv, observations, actions)
        imitation.warm_critic(model, args.critic_steps)
    model.learn(total_timesteps=args.steps, reset_num_timesteps=False)
    model.save("out/my-policy-ppo")
    venv.save("out/my-policy-normalize.pkl")
    venv.training = False
    export.sb3(model, venv, "policy.onnx")


if __name__ == "__main__":
    main()
