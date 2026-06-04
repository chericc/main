fn first_word(s: &str) -> &str
{
    match s.find(' ') {
        Some(pos) => &s[..pos],
        None => s
    }
}

pub fn test()
{
    let text = "hello world!";
    let word = first_word(text);
    crate::log!("word: {}", word);
}