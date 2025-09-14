use super::definitions::{LSymbol, BLANK};

const INITIAL_SIZE_GUESS: usize = 500;

pub struct Tape {

    content: Vec<LSymbol>,
    head: usize,

    extend_on_end: bool
}

impl Tape {

    pub fn new (extend_on_end: bool) -> Self {
        Tape::with_size(INITIAL_SIZE_GUESS, extend_on_end)
    }

    pub fn with_size(size: usize, extend_on_end: bool) -> Self {
        let cnt: Vec<LSymbol> = vec![BLANK; size];
        Tape::with_content(cnt, extend_on_end)
    }

    pub fn with_content(content: Vec<LSymbol>, extend_on_end: bool) -> Self {
        Tape {
            content: content, 
            head: 0,
            extend_on_end: extend_on_end
        }
    }

    pub fn read(&self) -> LSymbol {
        self.content[self.head]
    }

    pub fn write(&mut self, x: LSymbol) {
        self.content[self.head] = x;
    }

    pub fn move_sx(&mut self) -> Result<(), ()> {
        if self.head <= 0 { return Err(()) }
        self.head -= 1;
        Ok(())
    }

    pub fn move_dx(&mut self) -> Result<(), ()> {
        if self.head == self.content.len() {
            if !self.extend_on_end { return Err(()) }
            else {
                self.content.push(BLANK);
            }
        }
        self.head += 1;
        Ok(())
    }

    pub fn head_position(&self) -> usize {
        self.head
    }

    pub fn size(&self) -> usize {
        self.content.len()
    }

    pub fn content(&self) -> &[LSymbol] {
        &self.content
    }

    pub fn does_extend_on_end(&self) -> bool {
        self.extend_on_end
    }
}