use crate::{config::Config, emoji::emoji, shortcuts as sc};
use egui::{Color32, Vec2, scroll_area::ScrollBarVisibility};
use std::collections::HashMap;
use ttf_parser::{
    self as ttfp, GlyphId,
    gsub::{LigatureSubstitution, SubstitutionSubtable},
};

pub struct OrpheusApp<'a> {
    face: ttfp::Face<'static>,
    cache: Vec<Option<egui::Image<'a>>>,
    emoji_map: HashMap<&'static str, GlyphId>,
    ligs: Vec<LigatureSubstitution<'static>>,
    tab: usize,
}

impl<'a> OrpheusApp<'a> {
    pub fn new(cc: &eframe::CreationContext) -> Self {
        egui_extras::install_image_loaders(&cc.egui_ctx);

        cc.egui_ctx.style_mut(|style| {
            let w = 8.0;
            style.spacing.scroll.bar_width = w;
            style.spacing.scroll.floating_allocated_width = w - 2.0;
            style.spacing.scroll.handle_min_length = 24.0;
            style.visuals.slider_trailing_fill = true;

            style.wrap_mode = Some(egui::TextWrapMode::Extend);
            for (_, font_id) in style.text_styles.iter_mut() {
                font_id.size += 5.0;
            }
        });

        let font = include_bytes!("/usr/share/fonts/noto/NotoColorEmoji.ttf")
            as &[u8];

        let face = ttfp::Face::parse(font, 0).expect("bad font");

        let mut ligs = Vec::<LigatureSubstitution>::with_capacity(10);
        let gsub = face.tables().gsub.expect("no gsub");

        for l in gsub.lookups {
            for s in l.subtables.into_iter::<SubstitutionSubtable>() {
                let SubstitutionSubtable::Ligature(ls) = s else { continue };
                ligs.push(ls);
            }
        }

        let mut app = Self {
            cache: vec![None; face.number_of_glyphs() as usize],
            ligs,
            face,
            emoji_map: HashMap::with_capacity(5000),
            tab: 9,
        };
        app.init_emoji_map().expect("invalid emoji sets");

        app
    }

    fn emoji_to_glyph_id(&self, emoji: &str) -> Option<GlyphId> {
        let count = match emoji.chars().count() {
            0 => return None,
            1 => {
                let ch = emoji.chars().next()?;
                return self.face.glyph_index(ch);
            }
            c => c,
        };

        let mut chars = emoji.chars();
        let cov = self.face.glyph_index(chars.next()?)?;
        let first = self.face.glyph_index(chars.next()?)?;
        let cc = (count - 1) as u16;

        for ls in self.ligs.iter() {
            let Some(idx) = ls.coverage.get(cov) else { continue };
            let set = ls.ligature_sets.get(idx)?;
            'lig_set: for lig in set {
                let len = lig.components.len();
                if len != cc || lig.components.get(0)? != first {
                    continue;
                }

                let z = chars.clone().zip(lig.components.into_iter().skip(1));
                for (ch, cm) in z {
                    let gid = self.face.glyph_index(ch)?;
                    if gid != cm {
                        continue 'lig_set;
                    }
                }

                return Some(lig.glyph);
            }
        }

        None
    }

    fn init_emoji_map(&mut self) -> Option<()> {
        for emoji in crate::data::FAV {
            if let Some(gid) = self.emoji_to_glyph_id(emoji) {
                self.emoji_map.insert(emoji, gid);
            }
        }

        Some(())
    }

    pub fn emoji_glyph(&mut self, emoji: &'static str) -> Option<GlyphId> {
        if let Some(gid) = self.emoji_map.get(emoji) {
            return Some(*gid);
        }

        let gid = self.emoji_to_glyph_id(emoji)?;
        self.emoji_map.insert(emoji, gid);
        Some(gid)
    }

    pub fn emoji_image(&mut self, gid: GlyphId) -> Option<&egui::Image<'a>> {
        let idx = gid.0 as usize;
        if idx >= self.cache.len() {
            return None;
        }
        if self.cache[idx].is_some() {
            return self.cache[idx].as_ref();
        }

        let img = self.face.glyph_raster_image(
            GlyphId(idx as u16),
            Config::EMOT_BOX as u16 - 5,
        )?;
        let name = format!("bytes://emoji-{idx}.png");
        let img = egui::Image::from_bytes(name, img.data.to_vec());
        self.cache[idx] = Some(img);
        return self.cache[idx].as_ref();
    }
}

impl<'a> eframe::App for OrpheusApp<'a> {
    fn update(&mut self, ctx: &egui::Context, _: &mut eframe::Frame) {
        if ctx.input_mut(|i| i.consume_shortcut(&sc::QUIT)) {
            ctx.send_viewport_cmd(egui::ViewportCommand::Close);
        }

        if ctx.input_mut(|i| i.consume_shortcut(&sc::CQUIT)) {
            ctx.send_viewport_cmd(egui::ViewportCommand::Close);
        }

        let es = egui::ScrollArea::vertical()
            .auto_shrink([false, false])
            .max_width(Config::WINDOW_SIZE.x)
            .max_height(Config::SCROLL_HEIGHT)
            .stick_to_bottom(true)
            .stick_to_right(true)
            .scroll_bar_visibility(ScrollBarVisibility::AlwaysHidden);
        let eg = egui::Grid::new("main-grid")
            .min_row_height(Config::EMOT_BOX)
            .max_col_width(Config::EMOT_BOX)
            .min_col_width(Config::EMOT_BOX)
            .num_columns(Config::GRID)
            .spacing(Vec2::splat(Config::GAP));
        let tg = egui::Grid::new("tabs-grid")
            .min_row_height(Config::EMOT_BOX)
            .max_col_width(Config::EMOT_BOX)
            .min_col_width(Config::EMOT_BOX)
            .num_columns(Config::GRID)
            .spacing(Vec2::splat(Config::GAP));
        let c = egui::CentralPanel::default()
            .frame(egui::Frame::default().fill(Color32::DARK_RED));
        c.show(ctx, |ui| {
            // let total = self.face.number_of_glyphs() as usize;

            //
            ui.allocate_ui(
                egui::vec2(ui.available_width(), Config::SEARCH_HEIGHT),
                |ui| {
                    ui.set_max_height(Config::SEARCH_HEIGHT);
                    ui.set_min_height(Config::SEARCH_HEIGHT);
                    ui.label(format!("search: {}", self.emoji_map.len()));
                },
            );

            let tabs = crate::data::TABS[self.tab].1;
            es.show(ui, |ui| {
                eg.show(ui, |ui| {
                    for (i, code) in tabs.iter().enumerate() {
                        if i % Config::GRID == 0 && i != 0 {
                            ui.end_row();
                        }
                        let Some(g) = self.emoji_glyph(code) else { continue };
                        let Some(img) = self.emoji_image(g) else { continue };

                        ui.add(emoji(code, img, true));
                    }
                })
            });

            tg.show(ui, |ui| {
                for (i, (code, _)) in crate::data::TABS.iter().enumerate() {
                    let Some(gid) = self.emoji_glyph(code) else { continue };
                    let Some(img) = self.emoji_image(gid) else { continue };
                    if ui.add(emoji(code, img, false)).clicked() {
                        self.tab = i;
                    }
                }
            });
        });
    }
}
