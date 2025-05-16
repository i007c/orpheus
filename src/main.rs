use config::Config;

mod app;
mod config;
mod data;
mod emote;
mod shortcuts;
mod utils;

fn main() {
    // unsafe { std::env::set_var("RUST_LOG", "info") };
    // pretty_env_logger::init();

    let native_options = eframe::NativeOptions {
        persistence_path: Some("./pref.json".into()),
        window_builder: Some(Box::new(|a| {
            a.with_position([1000.0, 200.0])
                .with_decorations(false)
                .with_inner_size(Config::WINDOW_SIZE)
                .with_active(true)
                .with_visible(true)
                .with_resizable(false)
                .with_always_on_top()
        })),
        ..Default::default()
    };

    eframe::run_native(
        "Orpheus",
        native_options,
        Box::new(|cc| Ok(Box::new(app::OrpheusApp::new(cc)))),
    )
    .unwrap();
}
