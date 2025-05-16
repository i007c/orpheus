use egui::{Color32, Sense, Stroke, Vec2, WidgetInfo, WidgetType};

use crate::config::Config;

pub struct Emoji<'a> {
    unicode: &'static str,
    image: &'a egui::Image<'a>,
    copy: bool,
}

pub fn emoji<'a>(
    unicode: &'static str, image: &'a egui::Image<'a>, copy: bool,
) -> Emoji<'a> {
    Emoji { unicode, image, copy }
}

impl<'a> egui::Widget for Emoji<'a> {
    fn ui(self, ui: &mut egui::Ui) -> egui::Response {
        let desired_size = Vec2::splat(Config::EMOT_BOX);

        let (rect, response) =
            ui.allocate_exact_size(desired_size, Sense::HOVER | Sense::CLICK);
        response.widget_info(|| WidgetInfo::new(WidgetType::ImageButton));

        if ui.is_rect_visible(rect) {
            // let visuals = ui.style().interact(&response);

            let stroke = if response.hovered() {
                Stroke::new(2.0, Color32::WHITE)
            } else {
                Stroke::NONE
            };

            ui.painter().rect(
                rect,
                0.0,
                Color32::from_rgb(4, 4, 4),
                stroke,
                epaint::StrokeKind::Inside,
            );

            let imgrect = rect.shrink2(Vec2::splat(Config::EMOT_SHRINK));
            self.image.paint_at(ui, imgrect);
        }

        if let Some(cursor) = ui.visuals().interact_cursor {
            if response.hovered() {
                ui.ctx().set_cursor_icon(cursor);
            }
        }

        if self.copy && response.clicked() {
            println!("emoji: {}", self.unicode);
            crate::utils::copy_text(self.unicode);
        }

        response

        // const SIZE: egui::Vec2 = vec2(50.0, 50.0);
        // let ub = egui::UiBuilder::new()
        //     .id_salt(("emoji", self.unicode))
        //     .sense(egui::Sense::click());

        // let res = ui.add(egui::Button::image(self.image.clone()));

        //
        // res

        // let res = ui.allocate_ui(
        //     SIZE,
        //     |ui| {
        //         let (r, rs) =
        //             ui.allocate_exact_size(SIZE, egui::Sense::click());
        //         // ui.allocate_exact_size
        //         self.image.paint_at(ui, r);
        //
        //         if rs.clicked() {
        //             println!("emoji: {}", self.unicode);
        //             ui.ctx().copy_text(self.unicode.to_string());
        //         }
        //     }, // egui::Sense::CLICK | egui::Sense::HOVER,
        // );
        // // egui::Frame::show(res.ctx.ui, ui, add_contents)
        // // let response = ui.scope_builder(ub, |ui| {
        // //     let response = ui.response();
        // //     let visuals = ui.style().interact(&response);
        // //     self.image.paint_at(ui, rect);
        // //     // let text_color = visuals.text_color();
        // //
        // //     egui::Frame::canvas(ui.style())
        // //         .fill(visuals.bg_fill.gamma_multiply(0.3))
        // //         .stroke(visuals.bg_stroke)
        // //         .inner_margin(ui.spacing().menu_margin)
        // //         .show(ui, |ui| {
        // //             ui.set_width(ui.available_width());
        // //         });
        // // });
        // // let response = response.response;
        //
        // // if res.response.clicked() {}
        //
        // res.response
    }
}
