use crate::shortcuts as sc;

pub struct OrpheusApp {}

impl OrpheusApp {
    pub fn new(_: &eframe::CreationContext) -> Self {
        Self {}
    }
}

impl eframe::App for OrpheusApp {
    fn update(&mut self, ctx: &egui::Context, _: &mut eframe::Frame) {
        if ctx.input_mut(|i| i.consume_shortcut(&sc::QUIT)) {
            ctx.send_viewport_cmd(egui::ViewportCommand::Close);
        }

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.label("orpheus");
        });
    }
}
