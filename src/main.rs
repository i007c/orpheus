mod app;
// mod config;
// mod db;
// mod error;
// mod fonts;
mod shortcuts;
// mod tiles;
// mod utils;

// pub use error::Result;

fn main() {
    // unsafe { std::env::set_var("RUST_LOG", "info") };
    // pretty_env_logger::init();

    let native_options = eframe::NativeOptions {
        persistence_path: Some("./pref.json".into()),
        ..Default::default()
    };

    eframe::run_native(
        "00-team-test-app",
        native_options,
        Box::new(|cc| Ok(Box::new(app::OrpheusApp::new(cc)))),
    )
    .unwrap();
}
