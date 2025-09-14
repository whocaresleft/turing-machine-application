mod turing;
use turing::{alphabet::*, tape::*, turing_machine::*, definitions::*};

fn main() {
    let mut tape = Tape::with_size(1, false);
    match tape.move_sx() {
        Ok(_) => { println!("Moved left") }
        Err(_) => { println!("Could not move left") }
    }
}
