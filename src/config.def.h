
static const short gap = 2;
static const short grid = 10;
static const short tabs_row = grid - 1;
static const short box = 45;
static const short gap_box = box + gap;
static const short width = (grid * box) + ((grid - 1) * gap);
static const short height = width;
static const char *colors[] = {"#f2f2f2", "#040404"};
static const int tab_active = 0xFFD600;
static const short focus_line[2] = {2, 1};
static const short tab_line = 3;
// default tab 0 to (grid - 1)
short tab = 0;

static const char *fonts[] = { "monospace:size=20", "emoji:size=18" };

static const char *tabs[10] = {"😀", "🤱" ,"🐧", "🍌", "🗽", "🎮", "💡", "☯", "🚩", "⭐"};

static const char *e_emotions[] = {
    "😀", "😃", "😄", "😁", "😆", "😅", "🤣", "😂", "🙂", "🙃", "🫠", "😉", "😊",
    "😇", "🥰", "😍", "🤩", "😘", "😗", "☺", "😚", "😙", "🥲", "😋", "😛", "😜",
    "🤪", "😝", "🤑", "🤗", "🤭", "🫢", "🫣", "🤫", "🤔", "🫡", "🤐", "🤨", "😐",
    "😑", "😶", "🫥", "😏", "😒", "🙄", "😬", "🤥", "🫨", "😌", "😔", "😪", "🤤",
    "😴", "😷", "🤒", "🤕", "🤢", "🤮", "🤧", "🥵", "🥶", "🥴", "😵", "🤯", "🤠",
    "🥳", "🥸", "😎", "🤓", "🧐", "😕", "🫤", "😟", "🙁", "☹", "😮", "😯", "😲",
    "😳", "🥺", "🥹", "😦", "😧", "😨", "😰", "😥", "😢", "😭", "😱", "😖", "😣",
    "😞", "😓", "😩", "😫", "🥱", "😤", "😡", "😠", "🤬", "😈", "👿", "💀", "☠",
    "💩", "🤡", "👹", "👺", "👻", "👽", "👾", "🤖", "😺", "😸", "😹", "😻", "😼",
    "😽", "🙀", "😿", "😾", "🙈", "🙉", "🙊","💌", "💘", "💝", "💖", "💗", "💓",
    "💞", "💕", "💟", "❣", "💔", "❤", "🩷", "🧡", "💛", "💚", "💙", "🩵", "💜",
    "🤎", "🖤", "🩶", "🤍", "💋", "💯", "💢", "💥", "💫", "💦", "💨", "🕳", "💬",
    "🗨", "🗯", "💭", "💤"
};
static const char *e_people[] = {
    "👋", "🤚", "🖐", "✋", "🖖", "🫱", "🫲", "🫳", "🫴", "🫷", "🫸", "👌", "🤌",
    "🤏", "✌", "🤞", "🫰", "🤟", "🤘", "🤙", "👈", "👉", "👆", "🖕", "👇", "☝",
    "🫵", "👍", "👎", "✊", "👊", "🤛", "🤜", "👏", "🙌", "🫶", "👐", "🤲", "🤝",
    "🙏", "✍", "💅", "🤳", "💪", "🦾", "🦿", "🦵", "🦶", "👂", "🦻", "👃", "🧠",
    "🫀", "🫁", "🦷", "🦴", "👀", "👁", "👅", "👄", "🫦", "👶", "🧒", "👦", "👧",
    "🧑", "👱", "👨", "🧔", "👩", "🧓", "👴", "👵", "🙍", "🙎", "🙅", "🙆", "💁",
    "🙋", "🧏", "🙇", "🤦", "🤷", "👮", "🕵", "💂", "🥷", "👷", "🫅", "🤴", "👸",
    "👳", "👲", "🧕", "🤵", "👰", "🤰", "🫃", "🫄", "🤱", "👼", "🎅", "🤶", "🦸",
    "🦹", "🧙", "🧚", "🧛", "🧜", "🧝", "🧞", "🧟", "🧌", "💆", "💇", "🚶", "🧍",
    "🧎", "🏃", "💃", "🕺", "🕴", "👯", "🧖", "🧗", "🤺", "🏇", "⛷", "🏂", "🏌",
    "🏄", "🚣", "🏊", "⛹", "🏋", "🚴", "🚵", "🤸", "🤼", "🤽", "🤾", "🤹", "🧘",
    "🛀", "🛌", "👭", "👫", "👬", "💏", "💑", "👪", "🗣", "👤", "👥", "🫂", "👣"
};
static const char *e_nature[] = {
    "🐵", "🐒", "🦍", "🦧", "🐶", "🐕", "🦮", "🐩", "🐺", "🦊", "🦝", "🐱", "🐈",
    "🦁", "🐯", "🐅", "🐆", "🐴", "🫎", "🫏", "🐎", "🦄", "🦓", "🦌", "🦬", "🐮",
    "🐂", "🐃", "🐄", "🐷", "🐖", "🐗", "🐽", "🐏", "🐑", "🐐", "🐪", "🐫", "🦙",
    "🦒", "🐘", "🦣", "🦏", "🦛", "🐭", "🐁", "🐀", "🐹", "🐰", "🐇", "🐿", "🦫",
    "🦔", "🦇", "🐻", "🐨", "🐼", "🦥", "🦦", "🦨", "🦘", "🦡", "🐾", "🦃", "🐔",
    "🐓", "🐣", "🐤", "🐥", "🐦", "🐧", "🕊", "🦅", "🦆", "🦢", "🦉", "🦤", "🪶",
    "🦩", "🦚", "🦜", "🪽", "🪿", "🐸", "🐊", "🐢", "🦎", "🐍", "🐲", "🐉", "🦕",
    "🦖", "🐳", "🐋", "🐬", "🦭", "🐟", "🐠", "🐡", "🦈", "🐙", "🐚", "🪸", "🪼",
    "🐌", "🦋", "🐛", "🐜", "🐝", "🪲", "🐞", "🦗", "🪳", "🕷", "🕸", "🦂", "🦟",
    "🪰", "🪱", "🦠", "💐", "🌸", "💮", "🪷", "🏵", "🌹", "🥀", "🌺", "🌻", "🌼",
    "🌷", "🪻", "🌱", "🪴", "🌲", "🌳", "🌴", "🌵", "🌾", "🌿", "☘", "🍀", "🍁",
    "🍂", "🍃", "🪹", "🪺",
};
static const char *e_foods[] = {
    "🍇", "🍈", "🍉", "🍊", "🍋", "🍌", "🍍", "🥭", "🍎", "🍏", "🍐", "🍑", "🍒",
    "🍓", "🫐", "🥝", "🍅", "🫒", "🥥", "🥑", "🍆", "🥔", "🥕", "🌽", "🌶", "🫑",
    "🥒", "🥬", "🥦", "🧄", "🧅", "🍄", "🥜", "🫘", "🌰", "🫚", "🫛", "🍞", "🥐",
    "🥖", "🫓", "🥨", "🥯", "🥞", "🧇", "🧀", "🍖", "🍗", "🥩", "🥓", "🍔", "🍟",
    "🍕", "🌭", "🥪", "🌮", "🌯", "🫔", "🥙", "🧆", "🥚", "🍳", "🥘", "🍲", "🫕",
    "🥣", "🥗", "🍿", "🧈", "🧂", "🥫", "🍱", "🍘", "🍙", "🍚", "🍛", "🍜", "🍝",
    "🍠", "🍢", "🍣", "🍤", "🍥", "🥮", "🍡", "🥟", "🥠", "🥡", "🦀", "🦞", "🦐",
    "🦑", "🦪", "🍦", "🍧", "🍨", "🍩", "🍪", "🎂", "🍰", "🧁", "🥧", "🍫", "🍬",
    "🍭", "🍮", "🍯", "🍼", "🥛", "☕", "🫖", "🍵", "🍶", "🍾", "🍷", "🍸", "🍹",
    "🍺", "🍻", "🥂", "🥃", "🫗", "🥤", "🧋", "🧃", "🧉", "🧊", "🥢", "🍽", "🍴",
    "🥄", "🔪", "🫙", "🏺"
};
static const char *e_places[] = {
    "🌍", "🌎", "🌏", "🌐", "🗺", "🗾", "🧭", "🏔", "⛰", "🌋", "🗻", "🏕", "🏖",
    "🏜", "🏝", "🏞", "🏟", "🏛", "🏗", "🧱", "🪨", "🪵", "🛖", "🏘", "🏚", "🏠",
    "🏡", "🏢", "🏣", "🏤", "🏥", "🏦", "🏨", "🏩", "🏪", "🏫", "🏬", "🏭", "🏯",
    "🏰", "💒", "🗼", "🗽", "⛪", "🕌", "🛕", "🕍", "⛩", "🕋", "⛲", "⛺", "🌁",
    "🌃", "🏙", "🌄", "🌅", "🌆", "🌇", "🌉", "♨", "🎠", "🛝", "🎡", "🎢", "💈",
    "🎪", "🚂", "🚃", "🚄", "🚅", "🚆", "🚇", "🚈", "🚉", "🚊", "🚝", "🚞", "🚋",
    "🚌", "🚍", "🚎", "🚐", "🚑", "🚒", "🚓", "🚔", "🚕", "🚖", "🚗", "🚘", "🚙",
    "🛻", "🚚", "🚛", "🚜", "🏎", "🏍", "🛵", "🦽", "🦼", "🛺", "🚲", "🛴", "🛹",
    "🛼", "🚏", "🛣", "🛤", "🛢", "⛽", "🛞", "🚨", "🚥", "🚦", "🛑", "🚧", "⚓",
    "🛟", "⛵", "🛶", "🚤", "🛳", "⛴", "🛥", "🚢", "✈", "🛩", "🛫", "🛬", "🪂",
    "💺", "🚁", "🚟", "🚠", "🚡", "🛰", "🚀", "🛸", "🛎", "🧳", "⌛", "⏳", "⌚",
    "⏰", "⏱", "⏲", "🕰", "🕛", "🕧", "🕐", "🕜", "🕑", "🕝", "🕒", "🕞", "🕓",
    "🕟", "🕔", "🕠", "🕕", "🕡", "🕖", "🕢", "🕗", "🕣", "🕘", "🕤", "🕙", "🕥",
    "🕚", "🕦", "🌑", "🌒", "🌓", "🌔", "🌕", "🌖", "🌗", "🌘", "🌙", "🌚", "🌛",
    "🌜", "🌡", "☀", "🌝", "🌞", "🪐", "⭐", "🌟", "🌠", "🌌", "☁", "⛅", "⛈",
    "🌤", "🌥", "🌦", "🌧", "🌨", "🌩", "🌪", "🌫", "🌬", "🌀", "🌈", "🌂", "☂", "☔",
    "⛱", "⚡", "❄", "☃", "⛄", "☄", "🔥", "💧", "🌊"
};
static const char *e_activity[] = {
    "🎃", "🎄", "🎆", "🎇", "🧨", "✨", "🎈", "🎉", "🎊", "🎋", "🎍", "🎎", "🎏",
    "🎐", "🎑", "🧧", "🎀", "🎁", "🎗", "🎟", "🎫", "🎖", "🏆", "🏅", "🥇", "🥈",
    "🥉", "⚽", "⚾", "🥎", "🏀", "🏐", "🏈", "🏉", "🎾", "🥏", "🎳", "🏏", "🏑",
    "🏒", "🥍", "🏓", "🏸", "🥊", "🥋", "🥅", "⛳", "⛸", "🎣", "🤿", "🎽", "🎿",
    "🛷", "🥌", "🎯", "🪀", "🪁", "🔫", "🎱", "🔮", "🪄", "🎮", "🕹", "🎰", "🎲",
    "🧩", "🧸", "🪅", "🪩", "🪆", "♠", "♥", "♦", "♣", "♟", "🃏", "🀄", "🎴", "🎭",
    "🖼", "🎨", "🧵", "🪡", "🧶", "🪢"
};
static const char *e_object[] = {
    "👓", "🕶", "🥽", "🥼", "🦺", "👔", "👕", "👖", "🧣", "🧤", "🧥", "🧦", "👗",
    "👘", "🥻", "🩱", "🩲", "🩳", "👙", "👚", "🪭", "👛", "👜", "👝", "🛍", "🎒",
    "🩴", "👞", "👟", "🥾", "🥿", "👠", "👡", "🩰", "👢", "🪮", "👑", "👒", "🎩",
    "🎓", "🧢", "🪖", "⛑", "📿", "💄", "💍", "💎", "🔇", "🔈", "🔉", "🔊", "📢",
    "📣", "📯", "🔔", "🔕", "🎼", "🎵", "🎶", "🎙", "🎚", "🎛", "🎤", "🎧", "📻",
    "🎷", "🪗", "🎸", "🎹", "🎺", "🎻", "🪕", "🥁", "🪘", "🪇", "🪈", "📱", "📲",
    "☎", "📞", "📟", "📠", "🔋", "🪫", "🔌", "💻", "🖥", "🖨", "⌨", "🖱", "🖲",
    "💽", "💾", "💿", "📀", "🧮", "🎥", "🎞", "📽", "🎬", "📺", "📷", "📸", "📹",
    "📼", "🔍", "🔎", "🕯", "💡", "🔦", "🏮", "🪔", "📔", "📕", "📖", "📗", "📘",
    "📙", "📚", "📓", "📒", "📃", "📜", "📄", "📰", "🗞", "📑", "🔖", "🏷", "💰",
    "🪙", "💴", "💵", "💶", "💷", "💸", "💳", "🧾", "💹", "✉", "📧", "📨", "📩",
    "📤", "📥", "📦", "📫", "📪", "📬", "📭", "📮", "🗳", "✏", "✒", "🖋", "🖊",
    "🖌", "🖍", "📝", "💼", "📁", "📂", "🗂", "📅", "📆", "🗒", "🗓", "📇", "📈",
    "📉", "📊", "📋", "📌", "📍", "📎", "🖇", "📏", "📐", "✂", "🗃", "🗄", "🗑",
    "🔒", "🔓", "🔏", "🔐", "🔑", "🗝", "🔨", "🪓", "⛏", "⚒", "🛠", "🗡", "⚔",
    "💣", "🪃", "🏹", "🛡", "🪚", "🔧", "🪛", "🔩", "⚙", "🗜", "⚖", "🦯", "🔗",
    "⛓", "🪝", "🧰", "🧲", "🪜", "⚗", "🧪", "🧫", "🧬", "🔬", "🔭", "📡", "💉",
    "🩸", "💊", "🩹", "🩼", "🩺", "🩻", "🚪", "🛗", "🪞", "🪟", "🛏", "🛋", "🪑",
    "🚽", "🪠", "🚿", "🛁", "🪤", "🪒", "🧴", "🧷", "🧹", "🧺", "🧻", "🪣", "🧼",
    "🫧", "🪥", "🧽", "🧯", "🛒", "🚬", "⚰", "🪦", "⚱", "🧿", "🪬", "🗿", "🪧", "🪪"
};
static const char *e_symbol[] = {
    "🏧", "🚮", "🚰", "♿", "🚹", "🚺", "🚻", "🚼", "🚾", "🛂", "🛃", "🛄", "🛅",
    "⚠", "🚸", "⛔", "🚫", "🚳", "🚭", "🚯", "🚱", "🚷", "📵", "🔞", "☢", "☣",
    "⬆", "↗", "➡", "↘", "⬇", "↙", "⬅", "↖", "↕", "↔", "↩", "↪", "⤴",
    "⤵", "🔃","🔄", "🔙", "🔚", "🔛", "🔜", "🔝", "🛐", "⚛", "🕉", "✡", "☸",
    "☯", "✝", "☦", "☪", "☮", "🕎", "🔯", "🪯", "♈", "♉", "♊", "♋", "♌",
    "♍", "♎", "♏", "♐", "♑", "♒", "♓", "⛎", "🔀", "🔁", "🔂", "▶", "⏩",
    "⏭", "⏯", "◀", "⏪", "⏮", "🔼", "⏫", "🔽", "⏬", "⏸", "⏹", "⏺", "⏏",
    "🎦", "🔅", "🔆", "📶", "🛜", "📳", "📴", "♀", "♂", "⚧", "✖", "➕", "➖",
    "➗", "🟰", "♾", "‼", "⁉", "❓", "❔", "❕", "❗", "〰", "💱", "💲", "⚕",
    "♻", "⚜", "🔱", "📛", "🔰", "⭕", "✅", "☑", "✔", "❌", "❎", "➰", "➿",
    "〽", "✳", "✴", "❇", "©", "®", "™", "🔟", "🔠", "🔡", "🔢", "🔣", "🔤",
    "🅰", "🆎", "🅱", "🆑", "🆒", "🆓", "ℹ", "🆔", "Ⓜ", "🆕", "🆖", "🅾", "🆗",
    "🅿", "🆘", "🆙", "🆚", "🈁", "🈂", "🈷", "🈶", "🈯", "🉐", "🈹", "🈚", "🈲",
    "🉑", "🈸", "🈴", "🈳", "㊗", "㊙", "🈺", "🈵", "🔴", "🟠", "🟡", "🟢", "🔵",
    "🟣", "🟤", "⚫", "⚪", "🟥", "🟧", "🟨", "🟩", "🟦", "🟪", "🟫", "⬛", "⬜",
    "◼", "◻", "◾", "◽", "▪", "▫", "🔶", "🔷", "🔸", "🔹", "🔺", "🔻", "💠",
    "🔘", "🔳", "🔲"
};
static const char *e_flags[] = {"🏁", "🚩", "🎌", "🏴", "🏳"};
static const char *e_favorites[] = {"🧊"};

static EmojiSet emojis[10] = {
    EMOJI_SET(e_emotions),
    EMOJI_SET(e_people),
    EMOJI_SET(e_nature),
    EMOJI_SET(e_foods),
    EMOJI_SET(e_places),
    EMOJI_SET(e_activity),
    EMOJI_SET(e_object),
    EMOJI_SET(e_symbol),
    EMOJI_SET(e_flags),
    EMOJI_SET(e_favorites),
};