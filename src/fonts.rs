use std::sync::Arc;

fn add_font(
    fonts: &mut egui::FontDefinitions, name: &'static str, data: &'static [u8],
) {
    let fd = egui::FontData::from_static(data);

    fonts.font_data.insert(name.to_string(), Arc::new(fd));

    fonts
        .families
        .entry(egui::FontFamily::Proportional)
        .or_default()
        .insert(0, name.to_string());

    fonts
        .families
        .entry(egui::FontFamily::Monospace)
        .or_default()
        .push(name.to_string());
}

pub fn fonts_update(ctx: &egui::Context) {
    let mut fonts = egui::FontDefinitions::default();

    macro_rules! add_all {
        ($([$name:literal; $path:literal]),*) => {
            $(add_font(&mut fonts, $name, include_bytes!($path));)*
        };
    }

    add_all!(["notocj"; "/usr/share/fonts/noto/NotoColorEmoji.ttf"]);

    ctx.set_fonts(fonts);
}
