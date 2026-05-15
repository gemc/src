"""Readers for GEMC analyzer input formats."""

from __future__ import annotations

from pathlib import Path
from typing import Any

import pandas as pd

from .dataset import GemcOutput

CSV_STREAM_SUFFIXES = {
	"true_info": "_true_info.csv",
	"digitized": "_digitized.csv",
}


def read_output(path: str | Path, *, kind: str = "auto") -> GemcOutput:
	"""Read GEMC output into a :class:`GemcOutput`.

	``kind`` can be ``auto``, ``csv``, or ``root``. For CSV, ``path`` can be a
	single CSV file or the common rootname before ``_true_info.csv`` and
	``_digitized.csv``.
	"""

	path = Path(path)
	if kind == "auto":
		kind = _infer_kind(path)
	if kind == "csv":
		return read_csv_output(path)
	if kind == "root":
		return read_root_output(path)
	raise ValueError("kind must be one of: 'auto', 'csv', 'root'")


def read_csv_output(path: str | Path) -> GemcOutput:
	"""Read GEMC CSV output.

	CSV files are read with ``sep=","`` and ``skipinitialspace=True`` to match
	the GEMC CSV streamer output style.
	"""

	path = Path(path)
	output = GemcOutput(source=str(path))

	if path.suffix == ".csv":
		frame = _read_csv(path)
		stream = _classify_csv_stream(path)
		if stream == "true_info":
			output.true_info[path.stem] = frame
		else:
			output.digitized[path.stem] = frame
		return output

	for stream, suffix in CSV_STREAM_SUFFIXES.items():
		candidate = Path(str(path) + suffix)
		if candidate.exists():
			frame = _read_csv(candidate)
			if stream == "true_info":
				output.true_info["csv"] = frame
			else:
				output.digitized["csv"] = frame

	if not output.true_info and not output.digitized:
		raise FileNotFoundError(f"No GEMC CSV files found for '{path}'.")

	return output


def read_root_output(path: str | Path) -> GemcOutput:
	"""Read GEMC ROOT output through uproot.

	Detector trees named ``true_info_<detector>`` and ``digitized_<detector>``
	are flattened so each vector element becomes one DataFrame row.
	"""

	path = Path(path)
	try:
		import uproot
	except ImportError as exc:
		raise ImportError("Reading ROOT output requires the optional 'uproot' package.") from exc

	output = GemcOutput(source=str(path))
	with uproot.open(path) as root_file:
		for key in root_file.keys():
			name = key.split(";")[0]
			tree = root_file[key]
			if not hasattr(tree, "arrays"):
				continue

			if name.startswith("true_info_"):
				detector = name.removeprefix("true_info_")
				output.true_info[detector] = _tree_to_frame(tree)
			elif name.startswith("digitized_"):
				detector = name.removeprefix("digitized_")
				output.digitized[detector] = _tree_to_frame(tree)
			elif name in ("event_header", "run_header"):
				output.headers[name] = _tree_to_frame(tree)

	return output


def _read_csv(path: Path) -> pd.DataFrame:
	return pd.read_csv(path, sep=",", skipinitialspace=True)


def _classify_csv_stream(path: Path) -> str:
	name = path.name
	if name.endswith(CSV_STREAM_SUFFIXES["true_info"]):
		return "true_info"
	return "digitized"


def _infer_kind(path: Path) -> str:
	if path.suffix == ".root":
		return "root"
	return "csv"


def _tree_to_frame(tree: Any) -> pd.DataFrame:
	arrays = tree.arrays(library="np")
	rows: list[dict[str, Any]] = []

	for entry_index in range(tree.num_entries):
		entry_columns = {name: _entry_values(values[entry_index]) for name, values in arrays.items()}
		row_count = max((len(values) for values in entry_columns.values()), default=0)
		if row_count == 0:
			rows.append({name: None for name in entry_columns})
			continue

		for row_index in range(row_count):
			row = {}
			for name, values in entry_columns.items():
				row[name] = values[row_index] if row_index < len(values) else None
			rows.append(row)

	return pd.DataFrame(rows)


def _entry_values(value: Any) -> list[Any]:
	if isinstance(value, (str, bytes)):
		return [value.decode() if isinstance(value, bytes) else value]
	try:
		if value.ndim == 0:
			return [value.item()]
	except AttributeError:
		pass
	try:
		return [_python_scalar(item) for item in value]
	except TypeError:
		return [_python_scalar(value)]


def _python_scalar(value: Any) -> Any:
	if isinstance(value, bytes):
		return value.decode()
	try:
		return value.item()
	except AttributeError:
		return value
