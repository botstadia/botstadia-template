// A starter bot for dance-off: it reads the board and glides its whole body
// into each card's pose, arriving on the beat. It never tries to balance, so
// it falls now and then: beating it is your first job. interface.h names
// every input and output, and agent.h is the bot interface (generated from
// BotStadia's WIT); `botstadia create-bot --refresh` rewrites those for a new
// arena version and never touches this file.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "agent.h"
#include "interface.h"

// Start moving into a card's pose this many seconds before its beat.
#define LEAD 0.7f

// The joint targets sent last turn: where the glide starts from.
static float command[OUTPUT_TARGETS_LEN];
static bool gliding = false;

// Once, before the first turn, under the init budget.
void exports_agent_init(void) { gliding = false; }

// Each turn: this turn's input in, this turn's output back. An empty output
// is no action: the dancer holds its last accepted targets.
void exports_agent_on_tick(agent_list_u8_t *input, agent_list_u8_t *ret) {
    static bot_input_t in;
    bot_output_t out;
    ret->ptr = NULL;
    ret->len = 0;
    bool parsed = bot_input_parse(input->ptr, input->len, &in);
    agent_list_u8_free(input);
    if (!parsed) {
        return;
    }
    if (in.dance[INPUT_DANCE_DOWN_START] > 0.5f) {
        // Knocked down: forget the glide and start again from wherever the
        // stagehand stands us up.
        gliding = false;
        return;
    }
    if (!gliding) {
        for (int i = 0; i < OUTPUT_TARGETS_LEN; i++) command[i] = in.joints[i];
        gliding = true;
    }
    // The pose to be in next: the current card's, until its beat has passed
    // and the next one is close.
    float current = in.cards[INPUT_CARDS_CURRENT_SECONDS_START];
    float next = in.cards[INPUT_CARDS_NEXT_SECONDS_START];
    const float *goal = &in.cards[INPUT_CARDS_CURRENT_TARGETS_START];
    float seconds = current;
    if (current < 0.0f && next <= LEAD) {
        goal = &in.cards[INPUT_CARDS_NEXT_TARGETS_START];
        seconds = next;
    }
    // Cover the remaining distance evenly over the turns left before the
    // beat, so the pose lands on it; within the lead time only.
    float turns_left = seconds * (float)BOT_CONTROL_RATE;
    if (turns_left < 1.0f) turns_left = 1.0f;
    if (seconds <= LEAD) {
        for (int i = 0; i < OUTPUT_TARGETS_LEN; i++) command[i] += (goal[i] - command[i]) / turns_left;
    }
    // Print to see what your bot sees: `botstadia run --bot-logs`.
    // fprintf(stderr, "%.2fs to the beat\n", (double)seconds);
    for (int i = 0; i < OUTPUT_TARGETS_LEN; i++) out.targets[i] = command[i];
    ret->ptr = malloc(BOT_OUTPUT_BYTES);
    ret->len = bot_output_encode(&out, ret->ptr);
}
