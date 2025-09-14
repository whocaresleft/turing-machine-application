use super::definitions::{State, LSymbol};

pub enum Error {
    StateTooSmall,
    StateTooBig,
    SymbolTooSmall,
    SymbolTooBig,
    TransitionExists,
    TransitionDidNotExist,
    AlreadyFinal,
    WasNotFinal
}
pub type CreationResult<T> = Result<T, Error>;
pub type TransitionInsertResult<T> = Result<T, Error>;
pub type TransitionGetResult<T> = Result<T, Error>;
pub type TransitionRemoveResult<T> = Result<T, Error>;
pub type FinalStateInsertResult<T> = Result<T, Error>;
pub type FinalStateGetResult<T> = Result<T, Error>;
pub type FinalStateRemoveResult<T> = Result<T, Error>;



pub struct TuringMachine <const K: usize>{
    
    biggest_state_index: State,
    biggest_symbol_index: LSymbol, // Up to 0xFD. biggest +1 (max 0xFE) is RIGHT >, biggest +2 (max 0xFF) is LEFT <

    final_states: std::collections::HashSet<State>,

    transitions: std::collections::HashMap<(State, [LSymbol; K]), (State, [LSymbol; K])>
}

impl<const K: usize> TuringMachine<K> {

    pub fn new(state_count: usize, symbol_count: usize) -> CreationResult<Self> {

        let biggest_state = match state_count {
            0x00 => return Err(Error::StateTooSmall),
            0x01..=0x100 => state_count - 1,
            _ => return Err(Error::StateTooBig),
        } as u8;
        let biggest_symbol = match symbol_count {
            0x00 => return Err(Error::SymbolTooSmall),
            0x01..=0xFE => symbol_count - 1,
            _ => return Err(Error::StateTooBig),
        } as u8;

        Ok(TuringMachine {
            biggest_state_index: biggest_state,
            biggest_symbol_index: biggest_symbol,
            final_states: std::collections::HashSet::new(),
            transitions: std::collections::HashMap::new()
        })
    }

    pub fn add_transition(&mut self, q: State, x: [LSymbol; K], a: [LSymbol; K], t: State) -> TransitionInsertResult<()> {

        if q > self.biggest_state_index || t > self.biggest_state_index { return Err(Error::StateTooBig) }

        for s in x { if s > self.biggest_symbol_index { return Err(Error::SymbolTooBig) } }
        for s in a { if s > self.biggest_symbol_index { return Err(Error::SymbolTooBig) } }

        let (_in, _out) = ( (q, x), (t, a) );
        if self.transitions.contains_key(&_in) { return Err(Error::TransitionExists) }
        self.transitions.insert(_in, _out);
        Ok(())
    }

    pub fn get_transition(&self, q: State, x: &[LSymbol; K]) -> 
    TransitionGetResult<
        (State, [LSymbol; K])
    > {
        if q > self.biggest_state_index { return Err(Error::StateTooBig) }
        for s in x { if *s > self.biggest_symbol_index { return Err(Error::SymbolTooBig) } }

        match self.transitions.get(&(q, *x)) {
            None => Err(Error::TransitionDidNotExist),
            Some(out) => Ok(*out)
        }
    }

    pub fn remove_transition(&mut self, q: State, x: &[LSymbol; K]) -> TransitionRemoveResult<()> {
        match self.transitions.remove(&(q, *x)) {
            None => Err(Error::TransitionDidNotExist),
            Some(_) => Ok(())
        }
    }

    pub fn sx(&self) -> LSymbol {
        return self.biggest_symbol_index + 2;
    }

    pub fn dx(&self) -> LSymbol {
        return self.biggest_symbol_index + 1;
    }

    pub fn add_final_state(&mut self, state: State) -> FinalStateInsertResult<()> {
        if state > self.biggest_state_index { return Err(Error::StateTooBig) }
        if self.final_states.insert(state) {
            Ok(())
        } else {
            Err(Error::AlreadyFinal)
        }
    }

    pub fn remove_final_state(&mut self, state: State) -> FinalStateRemoveResult<()> {
        if state > self.biggest_state_index { return Err(Error::StateTooBig) }
        if self.final_states.remove(&state) {
            Ok(())
        } else {
            Err(Error::WasNotFinal)
        }
    }
    
    pub fn is_final_state(&self, state: State) -> FinalStateGetResult<bool> {
        if state > self.biggest_state_index { return Err(Error::StateTooBig) }
        Ok(self.final_states.contains(&state))
    }

    pub fn to_string(&self) -> String {
        let mut repr: String = String::new();

        let mut first = true;
        repr.push_str("States Q = {");
        for i in 0..self.biggest_state_index {
            if !first {repr.push_str(", ");}
            repr.push('q');
            repr.push_str(&i.to_string());
            first = false;
        }
        repr.push_str("}\n|Q| = ");
        repr.push_str(&self.biggest_state_index.to_string());
        repr.push_str("\n\nFinal States F = {");
        first = true;
        for q in self.final_states.iter() {
            if !first {repr.push_str(", ");}
            repr.push('q');
            repr.push_str(q.to_string().as_str());
            first = false;
        }

        repr.push_str("}\n\nNumber of symbols |S| = ");
        repr.push_str(&self.biggest_symbol_index.to_string());
        repr.push_str("\nRight (R): ");
        repr.push_str(&format!("{}", self.dx()));
        repr.push_str("\nLeft (L): ");
        repr.push_str(&format!("{}", self.sx()));
        repr.push_str("\n\n");

        repr.push_str("Number of tapes: ");
        repr.push_str(&format!("{}", K));
        repr.push_str("\n\nTransitions:\n");
        for t in self.transitions.iter() {

            repr.push_str(&(t.0.0).to_string());
            repr.push_str(" (");
            first = true;
            for s in t.0.1 {
                if !first { repr.push_str(", "); }
                repr.push_str(&s.to_string());
                first = false;
            }
            repr.push_str(") (");
            first = true;
            for s in t.1.1 {
                if !first { repr.push_str(", "); }
                repr.push_str(&s.to_string());
                first = false;
            }
            repr.push_str(") ");
            repr.push_str(&(t.1.0).to_string());
            repr.push('\n'); 
        }

        repr
    }
}