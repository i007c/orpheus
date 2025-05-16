use std::{
    io::Write,
    process::{Command, Stdio},
};

pub fn copy_text(text: &str) {
    let mut secondary = Command::new("xclip")
        .args(["-sel", "c"])
        .stdin(Stdio::piped())
        .spawn()
        .unwrap();

    let mut stdin = secondary.stdin.take().unwrap();
    stdin.write_all(text.as_bytes()).unwrap();

    let mut primary = Command::new("xclip")
        .args(["-sel", "p"])
        .stdin(Stdio::piped())
        .spawn()
        .unwrap();

    let mut stdin = primary.stdin.take().unwrap();
    stdin.write_all(text.as_bytes()).unwrap();
}
