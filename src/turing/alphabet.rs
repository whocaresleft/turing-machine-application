use super::definitions::{RSymbol, LSymbol};

use std::collections::HashMap;
pub struct Alphabet {
    blank_r_symbol: RSymbol,
    l_to_r: HashMap<LSymbol, RSymbol>,
    r_to_l: HashMap<RSymbol, LSymbol>,
}

impl Alphabet {

    pub fn new(blank_r_symbol: RSymbol) -> Self {
        let mut alpha = Alphabet {
            blank_r_symbol: blank_r_symbol,
            l_to_r: HashMap::new(),
            r_to_l: HashMap::new()
        };

        alpha.add_symbol(blank_r_symbol).ok();
        
        alpha
    }

    pub fn add_symbol(&mut self, symbol: RSymbol) -> Result<(), ()> {
        if self.r_to_l.contains_key(&symbol) { return Err(()) }
        let current_count /* also new index */ = self.l_to_r.len() as u8;

        self.l_to_r.insert(current_count, symbol);
        self.r_to_l.insert(symbol, current_count);

        Ok(())
    }

    pub fn add_symbols(&mut self, symbols: &[RSymbol]) -> (u8, u8) {
        let (mut inserted, mut rejected) = (0u8, 0u8);
        for r_symbol in symbols {
            match self.add_symbol(*r_symbol) {
                Ok(())  => { inserted += 1 }
                Err(()) => { rejected += 1 }
            }
        }
        (inserted, rejected)
    }

    pub fn get_l_symbol(&self, r_key: &RSymbol) -> Option<LSymbol> {
        self.r_to_l.get(r_key).map(|l| *l)
    }
    pub fn get_r_symbol(&self, l_key: &LSymbol) -> Option<RSymbol> {
        self.l_to_r.get(l_key).map(|r| *r)
    }

    pub fn len(&self) -> usize { self.l_to_r.len() }
}