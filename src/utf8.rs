pub struct Utf8CharIterator<'a> {
    string: &'a str,
    index: usize,
}

impl<'a> Utf8CharIterator<'a> {
    pub fn new(string: &'a str) -> Self {
        Self {
            string,
            index: 0,
        }
    }
}

impl Iterator for Utf8CharIterator<'_> {
    type Item = (usize, usize, char);

    fn next(&mut self) -> Option<Self::Item> {
        let start = self.index;

        for i in (start+1).. {
            if i > self.string.len() {
                return None;
            }

            if self.string.is_char_boundary(i) && i != 0 {
                let end = i;
                let ch = self.string[start..end].chars().next().unwrap();

                self.index = i;
                return Some((start, end, ch));
            }
        }

        unreachable!();
    }
}
