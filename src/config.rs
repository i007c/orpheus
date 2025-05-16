use egui::Vec2;

pub struct Config {}

impl Config {
    pub const EMOT_SHRINK: f32 = 6.0;
    pub const EMOT_BOX: f32 = 54.0;
    pub const GRID: usize = 10;
    pub const GAP: f32 = 2.0;
    pub const SEARCH_HEIGHT: f32 = 24.0;

    pub const GRID_BOX: f32 = Self::GRID as f32 * Self::EMOT_BOX
        + ((Self::GRID as f32 - 1.0) * Self::GAP);
    pub const SCROLL_HEIGHT: f32 =
        Self::GRID_BOX - Self::EMOT_BOX - Self::SEARCH_HEIGHT;
    pub const WINDOW_SIZE: egui::Vec2 = Vec2::new(
        Self::GRID_BOX,
        Self::SEARCH_HEIGHT + Self::SCROLL_HEIGHT + Self::EMOT_BOX + 6.0,
    );
}
