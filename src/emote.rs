use crate::config::Config;
use egui::{Color32, Sense, Stroke, Vec2, WidgetInfo, WidgetType};

pub struct Emote<'a> {
    unicode: &'static str,
    image: &'a egui::Image<'a>,
    name: &'static str,
    copy: bool,
}

pub fn emote<'a>(
    unicode: &'static str, image: &'a egui::Image<'a>, name: &'static str,
    copy: bool,
) -> Emote<'a> {
    Emote { unicode, image, name, copy }
}

impl egui::Widget for Emote<'_> {
    fn ui(self, ui: &mut egui::Ui) -> egui::Response {
        let (rect, mut rs) = ui.allocate_exact_size(
            Vec2::splat(Config::EMOT_BOX),
            Sense::HOVER | Sense::CLICK,
        );
        rs.widget_info(|| WidgetInfo::new(WidgetType::ImageButton));

        if ui.is_rect_visible(rect) {
            let stroke = if rs.hovered() {
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

        if self.copy {
            rs = rs.on_hover_text(self.name);
        }

        if let Some(cursor) = ui.visuals().interact_cursor {
            if rs.hovered() {
                ui.ctx().set_cursor_icon(cursor);
            }
        }

        if self.copy && rs.clicked() {
            crate::utils::copy_text(self.unicode);
        }

        rs
    }
}
