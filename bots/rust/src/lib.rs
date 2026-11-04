//! A starter bot for dance-off: it reads the board and glides its whole body
//! into each card's pose, arriving on the beat. It never tries to balance, so
//! it falls now and then: beating it is your first job. `interface.rs` beside
//! this file names every input and output; `botstadia create-bot --refresh`
//! rewrites that file for a new arena version and never touches this one.

mod interface;

use std::cell::RefCell;

use botstadia_interface::bindings::agent::{Guest, export_agent};
use interface::{Input, Output, cards, dance};

/// Start moving into a card's pose this many seconds before its beat.
const LEAD: f32 = 0.7;

thread_local! {
    /// The joint targets sent last turn: where the glide starts from.
    static COMMAND: RefCell<Option<Vec<f32>>> = const { RefCell::new(None) };
}

struct Bot;

impl Guest for Bot {
    /// Once, before the first turn, under the init budget.
    fn init() {
        COMMAND.with(|command| *command.borrow_mut() = None);
    }

    /// Each turn: this turn's input in, this turn's output back. An empty
    /// output is no action: the dancer holds its last accepted targets.
    fn on_tick(input: Vec<u8>) -> Vec<u8> {
        let Some(input) = Input::parse(&input) else {
            return Vec::new();
        };
        let board = &input.cards;
        let down = input.dance[dance::DOWN.start] > 0.5;
        COMMAND.with(|command| {
            let mut command = command.borrow_mut();
            if down {
                // Knocked down: forget the glide and start again from
                // wherever the stagehand stands us up.
                *command = None;
                return Vec::new();
            }
            let from = command.get_or_insert_with(|| input.joints.clone());
            // The pose to be in next: the current card's, until its beat has
            // passed and the next one is close.
            let current_seconds = board[cards::CURRENT_SECONDS.start];
            let next_seconds = board[cards::NEXT_SECONDS.start];
            let (goal, seconds) = if current_seconds < 0.0 && next_seconds <= LEAD {
                (&board[cards::NEXT_TARGETS], next_seconds)
            } else {
                (&board[cards::CURRENT_TARGETS], current_seconds)
            };
            // Cover the remaining distance evenly over the turns left before
            // the beat, so the pose lands on it; within the lead time only.
            let turns_left = (seconds * interface::CONTROL_RATE as f32).max(1.0);
            if seconds <= LEAD {
                for (value, target) in from.iter_mut().zip(goal) {
                    *value += (target - *value) / turns_left;
                }
            }
            // Print to see what your bot sees: `botstadia run --bot-logs`.
            // eprintln!("{seconds:.2}s to the beat");
            Output {
                targets: from.clone(),
            }
            .encode()
        })
    }
}

export_agent!(Bot with_types_in botstadia_interface::bindings::agent);
