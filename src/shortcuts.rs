macro_rules! shortcuts {
    ($($var:ident : $mod:ident + $key:ident,)*) => {
        $(
        pub const $var: egui::KeyboardShortcut = egui::KeyboardShortcut::new(
            egui::Modifiers::$mod,
            egui::Key::$key
        );
        )*
    };
}

shortcuts! {
    QUIT: NONE + Q,
    // FULLSCREEN: NONE + F,
    // OPEN_FILE: CTRL + O,
}
