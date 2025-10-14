import re

# Common PyVista/HTML color names → 'RRGGBB' (uppercase, no '#')
_NAME_TO_HEX = {
	# basics
	"black":                "000000", "white": "FFFFFF", "red": "FF0000", "green": "008000",
	"blue":                 "0000FF",
	"yellow":               "FFFF00", "cyan": "00FFFF", "aqua": "00FFFF", "magenta": "FF00FF",
	"fuchsia":              "FF00FF",
	"gray":                 "808080", "grey": "808080", "silver": "C0C0C0", "maroon": "800000",
	"olive":                "808000",
	"navy":                 "000080", "teal": "008080", "purple": "800080",

	# grays
	"gainsboro":            "DCDCDC", "lightgray": "D3D3D3", "lightgrey": "D3D3D3",
	"darkgray":             "A9A9A9", "darkgrey": "A9A9A9",
	"dimgray":              "696969", "dimgrey": "696969", "slategray": "708090",
	"slategrey":            "708090",
	"lightslategray":       "778899", "lightslategrey": "778899", "darkslategray": "2F4F4F",
	"darkslategrey":        "2F4F4F",
	"whitesmoke":           "F5F5F5",

	# blues
	"aliceblue":            "F0F8FF", "ghostwhite": "F8F8FF", "lavender": "E6E6FA",
	"powderblue":           "B0E0E6",
	"lightblue":            "ADD8E6", "lightskyblue": "87CEFA", "skyblue": "87CEEB",
	"deepskyblue":          "00BFFF",
	"dodgerblue":           "1E90FF", "cornflowerblue": "6495ED", "steelblue": "4682B4",
	"lightsteelblue":       "B0C4DE",
	"royalblue":            "4169E1", "mediumblue": "0000CD", "darkblue": "00008B",
	"midnightblue":         "191970",
	"slateblue":            "6A5ACD", "mediumslateblue": "7B68EE", "darkslateblue": "483D8B",
	"blueviolet":           "8A2BE2",
	"indigo":               "4B0082",

	# cyans / teals
	"cadetblue":            "5F9EA0", "lightseagreen": "20B2AA", "mediumturquoise": "48D1CC",
	"paleturquoise":        "AFEEEE",
	"turquoise":            "40E0D0", "darkturquoise": "00CED1", "aquamarine": "7FFFD4",
	"mediumaquamarine":     "66CDAA",
	"darkcyan":             "008B8B",

	# greens
	"honeydew":             "F0FFF0", "mintcream": "F5FFFA", "palegreen": "98FB98",
	"lightgreen":           "90EE90",
	"springgreen":          "00FF7F", "mediumspringgreen": "00FA9A", "lime": "00FF00",
	"limegreen":            "32CD32",
	"lawngreen":            "7CFC00", "chartreuse": "7FFF00", "greenyellow": "ADFF2F",
	"yellowgreen":          "9ACD32",
	"darkseagreen":         "8FBC8F", "seagreen": "2E8B57", "forestgreen": "228B22",
	"mediumseagreen":       "3CB371",
	"darkgreen":            "006400", "olivedrab": "6B8E23", "darkolivegreen": "556B2F",

	# yellows / oranges
	"lightgoldenrodyellow": "FAFAD2", "lemonchiffon": "FFFACD", "lightyellow": "FFFFE0",
	"ivory":                "FFFFF0",
	"beige":                "F5F5DC", "cornsilk": "FFF8DC", "oldlace": "FDF5E6", "linen": "FAF0E6",
	"antiquewhite":         "FAEBD7",
	"papayawhip":           "FFEFD5", "blanchedalmond": "FFEBCD", "bisque": "FFE4C4",
	"moccasin":             "FFE4B5",
	"navajowhite":          "FFDEAD", "wheat": "F5DEB3", "burlywood": "DEB887", "tan": "D2B48C",
	"khaki":                "F0E68C", "darkkhaki": "BDB76B", "gold": "FFD700",
	"palegoldenrod":        "EEE8AA",
	"goldenrod":            "DAA520", "darkgoldenrod": "B8860B", "orange": "FFA500",
	"darkorange":           "FF8C00",

	# browns
	"peru":                 "CD853F", "chocolate": "D2691E", "saddlebrown": "8B4513",
	"sienna":               "A0522D", "brown": "A52A2A",
	"rosybrown":            "BC8F8F",

	# reds / corals
	"mistyrose":            "FFE4E1", "lavenderblush": "FFF0F5", "seashell": "FFF5EE",
	"snow":                 "FFFAFA", "floralwhite": "FFFAF0",
	"peachpuff":            "FFDAB9", "palevioletred": "DB7093", "lightcoral": "F08080",
	"salmon":               "FA8072",
	"darksalmon":           "E9967A", "lightsalmon": "FFA07A", "coral": "FF7F50",
	"tomato":               "FF6347", "orangered": "FF4500",
	"firebrick":            "B22222", "darkred": "8B0000", "crimson": "DC143C",

	# pinks / purples
	"pink":                 "FFC0CB", "lightpink": "FFB6C1", "hotpink": "FF69B4",
	"deeppink":             "FF1493", "mediumvioletred": "C71585",
	"orchid":               "DA70D6", "mediumorchid": "BA55D3", "darkorchid": "9932CC",
	"thistle":              "D8BFD8", "plum": "DDA0DD",
	"violet":               "EE82EE", "magenta": "FF00FF", "fuchsia": "FF00FF", "purple": "800080",
	"rebeccapurple":        "663399",
	"darkviolet":           "9400D3", "mediumpurple": "9370DB",

	# neutrals / misc
	"slategray":            "708090", "lightslategray": "778899", "darkslategray": "2F4F4F",
	"azure":                "F0FFFF", "lightcyan": "E0FFFF",
}

# also allow grey spellings for any 'gray' keys:
for k in list(_NAME_TO_HEX.keys()):
	if "gray" in k:
		_NAME_TO_HEX.setdefault(k.replace("gray", "grey"), _NAME_TO_HEX[k])

# assumes _NAME_TO_HEX exists above

_HEX6_RE = re.compile(r'^#?([0-9a-f]{6})$', re.IGNORECASE)
_HEX7_RE = re.compile(r'^#?([0-9a-f]{6})([0-5])$', re.IGNORECASE)  # RGB + transparency digit 0..5
_HEX3_RE = re.compile(r'^#?([0-9a-f]{3})$', re.IGNORECASE)  # allow #RGB/RGB -> expand
_IS_NUM = re.compile(r'^[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?$')


def _strip_weird_spaces(s: str) -> str:
	for ch in ("\u00a0", "\u2000", "\u2001", "\u2002", "\u2003", "\u2004", "\u2005",
	           "\u2006", "\u2007", "\u2008", "\u2009", "\u200a", "\u202f", "\u205f",
	           "\u3000", "\ufeff"):
		s = s.replace(ch, " ")
	return re.sub(r"\s+", " ", s).strip()


def pyvista_color_to_hex(name: str) -> str:
	"""
	Normalize a color to 'RRGGBB' or 'RRGGBB[T]' (T in 0..5). Leading '#' optional.
	Also supports names and comma-qualified names like 'metallic, turquoise' (rightmost known color).
	"""
	if not isinstance(name, str):
		raise TypeError("pyvista_color_to_hex expects a string")

	s = _strip_weird_spaces(name).strip().lower()

	# --- hex forms (with or without '#')
	if (m := _HEX7_RE.fullmatch(s)):
		# 7-digit: keep full string (RGB + transparency digit)
		return (m.group(1) + m.group(2)).upper()
	if (m := _HEX6_RE.fullmatch(s)):
		return m.group(1).upper()
	if (m := _HEX3_RE.fullmatch(s)):
		h = m.group(1).lower()
		return (h[0] * 2 + h[1] * 2 + h[2] * 2).upper()

	# --- comma-qualified names: rightmost known non-numeric token
	parts = [p.strip().replace(" ", "") for p in s.split(",")]
	if len(parts) > 1:
		if any(_IS_NUM.fullmatch(p) for p in parts if p):
			# No support for 'turquoise, 0.5'
			raise ValueError(f"Numeric alpha not supported in color: {name!r}")
		for token in reversed(parts):
			if token and token in _NAME_TO_HEX:
				return _NAME_TO_HEX[token]

	# --- simple named color
	simple = s.replace(" ", "")
	if simple in _NAME_TO_HEX:
		return _NAME_TO_HEX[simple]

	raise ValueError(f"Unknown color name or hex (expect 6 hex digits or 6+transparency): {name!r}")


def pyvista_gcolor_to_pcolor_and_opacity(color_str: str, default_opacity: float = 1.0):
	"""
	Convert 'RRGGBB' or 'RRGGBB[T]' (T in 0..5; 5 = fully transparent) to:
	  ( (r,g,b) in 0..1, opacity in 0..1 )

	Accepts optional prefixes ('#', '0x') and ignores surrounding whitespace.
	"""
	s = color_str.strip().lower()
	if s.startswith('#'):
		s = s[1:]
	if s.startswith('0x'):
		s = s[2:]

	# Optional trailing transparency digit 0..5
	t_level = None
	if len(s) == 7 and s[-1].isdigit():
		t_level = int(s[-1])
		s = s[:-1]

	if len(s) != 6 or not re.fullmatch(r'[0-9a-f]{6}', s):
		raise ValueError(f"Invalid color string: {color_str!r} (expected RRGGBB or RRGGBB[T])")

	r = int(s[0:2], 16) / 255.0
	g = int(s[2:4], 16) / 255.0
	b = int(s[4:6], 16) / 255.0

	if t_level is None:
		opacity = float(default_opacity)
	else:
		# transparency level: 0 (opaque) … 5 (fully transparent)
		t_level = max(0, min(5, t_level))
		opacity = 1.0 - (t_level / 5.0)

	return (r, g, b), opacity
