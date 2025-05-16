use crate::{config::Config, emote::emote, shortcuts as sc};
use egui::{
    Color32, Sense, Stroke, TextEdit, Vec2, scroll_area::ScrollBarVisibility,
};
use std::collections::HashMap;
use ttf_parser::{
    self as ttfp, GlyphId,
    gsub::{LigatureSubstitution, SubstitutionSubtable},
};

type Emo = (GlyphId, &'static str);

pub struct OrpheusApp<'a> {
    face: ttfp::Face<'static>,
    cache: Vec<Option<egui::Image<'a>>>,
    emoji_map: HashMap<&'static str, Emo>,
    ligs: Vec<LigatureSubstitution<'static>>,
    tab: usize,
    search: String,
}

impl OrpheusApp<'_> {
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
            search: String::new(),
            emoji_map: HashMap::with_capacity(5000),
            tab: 9,
        };
        app.init_emoji_map().expect("invalid emoji sets");

        app
    }

    fn emoji_to_glyph_id(&self, emoji: &str) -> Option<GlyphId> {
        let mut buf: [Option<GlyphId>; 20] = [None; 20];
        let mut count = 0usize;
        for (i, ch) in emoji.chars().enumerate() {
            if i >= buf.len() {
                println!("wtf this emoji has more than 20 chars");
                return None;
            }
            buf[i] = self.face.glyph_index(ch);
            count += 1;
        }

        let cov = buf[0]?;
        if count < 2 || buf[1].is_none() {
            return buf[0];
        }

        let mut gm = (GlyphId(0), 0);

        for ls in self.ligs.iter() {
            let Some(idx) = ls.coverage.get(cov) else { continue };
            let set = ls.ligature_sets.get(idx)?;
            'lig_set: for lig in set {
                for (ci, c) in lig.components.into_iter().enumerate() {
                    if buf[ci + 1] != Some(c) {
                        continue 'lig_set;
                    }
                    if ci >= gm.1 {
                        gm = (lig.glyph, ci);
                    }
                }
            }
        }

        Some(gm.0)
    }

    fn init_emoji_map(&mut self) -> Option<()> {
        for (emoji, tab) in crate::data::TABS {
            self.emo(emoji);
            for e in tab.iter() {
                self.emo(e);
            }
        }

        Some(())
    }

    pub fn emo(&mut self, code: &'static str) -> Option<Emo> {
        if let Some((gid, name)) = self.emoji_map.get(code) {
            return Some((*gid, name));
        }

        let gid = self.emoji_to_glyph_id(code)?;
        let name = emoji::lookup_by_glyph::lookup(code)
            .map(|v| v.name)
            .unwrap_or_default();
        self.emoji_map.insert(code, (gid, name));
        Some((gid, name))
    }

    pub fn emoji_image(&mut self, gid: GlyphId) -> Option<&egui::Image<'_>> {
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
        self.cache[idx].as_ref()
    }

    pub fn emote(
        &mut self, code: &'static str, ui: &mut egui::Ui, for_tabs: bool,
    ) -> egui::Response {
        let emo =
            self.emo(code).and_then(|(g, n)| Some((self.emoji_image(g)?, n)));
        if let Some((img, name)) = emo {
            ui.add(emote(code, img, name, !for_tabs))
        } else {
            let (rc, rs) = ui.allocate_exact_size(
                Vec2::splat(Config::EMOT_BOX),
                Sense::HOVER | Sense::CLICK,
            );

            if ui.is_rect_visible(rc) {
                let stroke = if rs.hovered() {
                    Stroke::new(2.0, Color32::WHITE)
                } else {
                    Stroke::NONE
                };

                ui.painter().rect(
                    rc,
                    0.0,
                    Color32::from_rgb(4, 4, 4),
                    stroke,
                    epaint::StrokeKind::Inside,
                );
            }

            if rs.clicked() {
                crate::utils::copy_text(code);
            }
            rs
        }
    }
}

impl eframe::App for OrpheusApp<'_> {
    fn update(&mut self, ctx: &egui::Context, _: &mut eframe::Frame) {
        if (self.search.is_empty()
            && ctx.input_mut(|i| i.consume_shortcut(&sc::QUIT)))
            || ctx.input_mut(|i| i.consume_shortcut(&sc::CQUIT))
        {
            ctx.send_viewport_cmd(egui::ViewportCommand::Close);
        }

        let es = egui::ScrollArea::vertical()
            .auto_shrink([false, false])
            .max_width(Config::WINDOW_SIZE.x)
            .max_height(Config::SCROLL_HEIGHT)
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
        let c = egui::CentralPanel::default().frame(egui::Frame::default());
        c.show(ctx, |ui| {
            ui.allocate_ui(
                egui::vec2(ui.available_width(), Config::SEARCH_HEIGHT),
                |ui| {
                    ui.set_max_height(Config::SEARCH_HEIGHT);
                    ui.set_min_height(Config::SEARCH_HEIGHT);
                    let x = TextEdit::singleline(&mut self.search)
                        .frame(false)
                        .hint_text("search:")
                        .desired_width(Config::WINDOW_SIZE.x)
                        .min_size(Vec2::new(
                            Config::WINDOW_SIZE.x,
                            Config::SEARCH_HEIGHT,
                        ));
                    ui.add(x).request_focus();
                },
            );

            if !self.search.is_empty() {
                // let x = emoji::search::search_tts_all(&self.search);
                // let x = emoji::search::search_annotation_all(&self.search);
                let x = emoji::search::search_name(&self.search);
                let x = x.iter().map(|e| e.glyph).enumerate();
                es.show(ui, |ui| {
                    eg.show(ui, |ui| {
                        for (i, code) in x {
                            if i % Config::GRID == 0 && i != 0 {
                                ui.end_row();
                            }

                            self.emote(code, ui, false);
                        }
                    })
                });
            } else {
                let emotes = crate::data::TABS[self.tab].1;

                es.show(ui, |ui| {
                    eg.show(ui, |ui| {
                        for (i, code) in emotes.iter().enumerate() {
                            if i % Config::GRID == 0 && i != 0 {
                                ui.end_row();
                            }

                            self.emote(code, ui, false);
                        }
                    })
                });
            };

            tg.show(ui, |ui| {
                for (i, (code, _)) in crate::data::TABS.iter().enumerate() {
                    if self.emote(code, ui, true).clicked() {
                        self.tab = i;
                        self.search.clear();
                    }
                }
            });
        });
    }
}
