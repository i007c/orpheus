
from json import load

from fontTools.ttLib import TTFont


font_path = '/usr/share/fonts/noto/NotoColorEmoji.ttf'

known_chars = []
good_emojis = []

def main():
    with open('./data/converted.json') as f:
        data = load(f)
    
    with TTFont(font_path) as ttf:
        for table in ttf['cmap'].tables:
            for c, *_ in table.cmap.items():
                known_chars.append(c)
    
    for emoji in data:
        # ingnoring multi unicode emojis
        if len(emoji[2]) > 1: continue

        code = emoji[2][0]

        # ignore if emoji is not supported by the font
        if not code in known_chars: continue

        good_emojis.append(code)

    
    with open('./emojis.txt', 'w', encoding='utf-8') as f:
        for idx, emoji in enumerate(good_emojis):
            if idx % 13 == 0:
                f.write('\n')
            f.write(f'"{chr(emoji)}", ')

if __name__ == '__main__':
    main()
