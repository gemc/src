# g4_units.py
import ast
import math
from typing import List, Sequence, Dict

# --- Canonical factors (to SI base units) ---
# Length to meters
LEN_TO_M = {
	'nm': 1e-9, 'um': 1e-6, 'µm': 1e-6, 'mm': 1e-3, 'cm': 1e-2, 'dm': 1e-1,
	'm': 1.0, 'km': 1e3, 'inch': 0.0254, 'in': 0.0254, 'ft': 0.3048
}
# Angle to radians
ANG_TO_RAD = {
	'rad': 1.0, 'deg': math.pi / 180.0, 'degree': math.pi / 180.0, 'gon': math.pi / 200.0
}

# Optional: time and others if you need them
TIME_TO_S = {
	'ps': 1e-12, 'ns': 1e-9, 'us': 1e-6, 'ms': 1e-3, 's': 1.0
}
ENERGY_TO_J = {
	'eV': 1.602176634e-19, 'keV': 1.602176634e-16, 'MeV': 1.602176634e-13,
	'GeV': 1.602176634e-10, 'TeV': 1.602176634e-7, 'J': 1.0
}

# --- Safe evaluator for simple arithmetic with unit names ---
_ALLOWED_NODES = (
	ast.Expression, ast.BinOp, ast.UnaryOp, ast.Num, ast.Constant,
	ast.Add, ast.Sub, ast.Mult, ast.Div, ast.Pow, ast.USub, ast.UAdd,
	ast.Name, ast.Load, ast.Call
)
_ALLOWED_FUNCS = {'sqrt': math.sqrt, 'abs': abs}
_ALLOWED_CONSTS = {'pi': math.pi, 'e': math.e}



# add at top with the other imports
import re

def _normalize_expr(expr: str) -> str:
	s = str(expr)
	for ch in ["\u00a0","\u2000","\u2001","\u2002","\u2003","\u2004","\u2005",
			   "\u2006","\u2007","\u2008","\u2009","\u200a","\u202f","\u205f",
			   "\u3000","\ufeff"]:
		s = s.replace(ch, " ")
	s = (s.replace("µ","u")
		 .replace("×","*")
		 .replace("·","*")
		 .replace("^","**"))
	return re.sub(r"\s+"," ", s).strip()


def _safe_eval(expr: str, names: Dict[str, float]) -> float:
	"""
	Evaluate '2*cm + 3*mm', raising clear errors on unknown symbols or operators.
	"""
	s = _normalize_expr(expr)

	try:
		tree = ast.parse(s, mode='eval')
	except (IndentationError, SyntaxError) as e:
		raise ValueError(f"Could not parse unit expression: {s!r}") from e

	def _eval(node):
		if not isinstance(node, _ALLOWED_NODES):
			raise ValueError(f"Disallowed expression node: {type(node).__name__} in {s!r}")

		if isinstance(node, (ast.Num, ast.Constant)):
			return float(node.n if hasattr(node, 'n') else node.value)

		if isinstance(node, ast.BinOp):
			l = _eval(node.left); r = _eval(node.right)
			if   isinstance(node.op, ast.Add):  return l + r
			elif isinstance(node.op, ast.Sub):  return l - r
			elif isinstance(node.op, ast.Mult): return l * r
			elif isinstance(node.op, ast.Div):  return l / r
			elif isinstance(node.op, ast.Pow):  return l ** r
			# If we get here, it’s an operator we didn’t intend to support
			raise ValueError(f"Unsupported operator {type(node.op).__name__} in {s!r}")

		if isinstance(node, ast.UnaryOp):
			v = _eval(node.operand)
			if   isinstance(node.op, ast.USub): return -v
			elif isinstance(node.op, ast.UAdd): return +v
			else:
				raise ValueError(f"Unsupported unary op {type(node.op).__name__} in {s!r}")

		if isinstance(node, ast.Name):
			key = node.id
			if key in names:            return float(names[key])
			if key in _ALLOWED_CONSTS:  return float(_ALLOWED_CONSTS[key])
			raise ValueError(f"Unknown symbol: {key!r} in {s!r}")

		if isinstance(node, ast.Call):
			if not isinstance(node.func, ast.Name):
				raise ValueError(f"Only simple calls allowed in {s!r}")
			fname = node.func.id
			if fname not in _ALLOWED_FUNCS:
				raise ValueError(f"Function not allowed: {fname!r} in {s!r}")
			args = [_eval(a) for a in node.args]
			return float(_ALLOWED_FUNCS[fname](*args))

		raise ValueError(f"Unsupported node {type(node).__name__} in {s!r}")

	return _eval(tree.body)

class _NameCollector(ast.NodeVisitor):
	def __init__(self):
		self.names = set()
		self.funcs = set()
	def visit_Name(self, node):
		self.names.add(node.id)
	def visit_Call(self, node):
		if isinstance(node.func, ast.Name):
			self.funcs.add(node.func.id)
		self.generic_visit(node)

def _names_in_expr(expr: str):
	s = _normalize_expr(expr)
	tree = ast.parse(s, mode="eval")
	nc = _NameCollector(); nc.visit(tree)
	# symbols used as variables = names - function names
	return s, (nc.names - nc.funcs)

def _infer_quantity(expr: str) -> str | None:
	"""
	Returns 'length', 'angle', 'mixed', or None (unknown/unitless).
	"""
	s, names = _names_in_expr(expr)
	if not names:
		return None  # unitless or only numbers/constants
	len_keys = set(LEN_TO_M.keys())
	ang_keys = set(ANG_TO_RAD.keys())
	in_len  = names <= len_keys
	in_ang  = names <= ang_keys
	if in_len and not in_ang:
		return 'length'
	if in_ang and not in_len:
		return 'angle'
	if in_len and in_ang:
		# empty set case is handled above; overlapping only if names == set()
		return None
	return 'mixed'  # contains unknown or both kinds of units

def convert_list(tokens,
				 kinds: list[str] | None = None,
				 out_len: str = 'mm',
				 out_ang: str = 'deg',
				 default_if_unitless: str | None = None) -> tuple[float, ...]:
	"""
	Convert a list of tokens where each item may be a length or an angle.
	- kinds: optional per-position list like ['length','angle','length',...]
	- default_if_unitless: if a token has no units, coerce to this kind ('length' or 'angle');
	  if None (default), unitless tokens raise an error.
	"""
	dims = []
	n = len(tokens)
	for i, tok in enumerate(tokens):
		norm = _normalize_expr(tok)
		# Determine the kind for this position
		kind = None
		if kinds is not None:
			if i >= len(kinds):
				raise ValueError(f"kinds length ({len(kinds)}) < tokens length ({n})")
			kind = kinds[i]
		else:
			k = _infer_quantity(norm)
			if k == 'mixed':
				raise ValueError(f"Token[{i}] mixes unit systems: {tok!r} (normalized={norm!r})")
			if k is None:
				if default_if_unitless is None:
					raise ValueError(f"Token[{i}] has no recognizable units: {tok!r} (normalized={norm!r})")
				kind = default_if_unitless
			else:
				kind = k

		try:
			if kind == 'length':
				val = convert_length(norm, out_len)
			elif kind == 'angle':
				val = convert_angle(norm, out_ang)
			else:
				raise ValueError(f"Unsupported kind {kind!r} for token[{i}]={tok!r}")
		except Exception as e:
			raise ValueError(
				f"Conversion failed for token[{i}]={tok!r} (normalized={norm!r}, kind={kind})"
			) from e

		dims.append(val)

	return tuple(dims)

# --- Public API ---

def convert_length(expr: str, out: str = 'mm') -> float:
	"""
	Convert '2*cm', '5.5*mm', 'sqrt(2)*m/10' to a numeric length in the requested unit.
	out: one of keys in LEN_TO_M (e.g. 'mm', 'm').
	"""
	if out not in LEN_TO_M:
		raise ValueError(f"Unknown output length unit: {out}")
	names = {**LEN_TO_M}  # names valued in meters
	val_m = _safe_eval(expr.replace('µ', 'u'), names)  # meters
	return val_m / LEN_TO_M[out]


def convert_angle(expr: str, out: str = 'deg') -> float:
	"""
	Convert '10*deg', 'pi/2*rad' to a numeric angle in requested unit ('rad' or 'deg').
	"""
	if out not in ANG_TO_RAD and out not in ('deg', 'rad'):
		raise ValueError(f"Unknown output angle unit: {out}")
	names = {**ANG_TO_RAD}
	val_rad = _safe_eval(expr, names)  # radians
	if out in ('deg', 'degree'):
		return val_rad * (180.0 / math.pi)
	return val_rad  # rad


def convert_time(expr: str, out: str = 'ns') -> float:
	if out not in TIME_TO_S:
		raise ValueError(f"Unknown output time unit: {out}")
	val_s = _safe_eval(expr, TIME_TO_S)  # seconds
	return val_s / TIME_TO_S[out]


def convert_energy(expr: str, out: str = 'MeV') -> float:
	if out not in ENERGY_TO_J:
		raise ValueError(f"Unknown output energy unit: {out}")
	val_J = _safe_eval(expr, ENERGY_TO_J)  # joules
	return val_J / ENERGY_TO_J[out]
