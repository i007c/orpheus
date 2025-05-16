use std::{
    io::Write,
    process::{Command, Stdio},
};

#[cfg(target_os = "linux")]
pub fn copy_text(text: &str) {
    let mut secondary = Command::new("xclip")
        .args(["-sel", "c"])
        .stdin(Stdio::piped())
        .spawn()
        .unwrap();

    let mut stdin = secondary.stdin.take().unwrap();
    stdin.write_all(text.as_bytes()).unwrap();
    drop(secondary);

    let mut primary = Command::new("xclip")
        .args(["-sel", "p"])
        .stdin(Stdio::piped())
        .spawn()
        .unwrap();

    let mut stdin = primary.stdin.take().unwrap();
    stdin.write_all(text.as_bytes()).unwrap();
    drop(primary);
}
