"""Data containers used by the GEMC analyzer."""

from __future__ import annotations

from dataclasses import dataclass, field

import pandas as pd


FrameMap = dict[str, pd.DataFrame]


@dataclass
class GemcOutput:
	"""In-memory representation of GEMC analyzer input.

	CSV output usually has one table for true information and one table for digitized
	data. ROOT output can have one table per detector tree.
	"""

	true_info: FrameMap = field(default_factory=dict)
	digitized: FrameMap = field(default_factory=dict)
	headers: FrameMap = field(default_factory=dict)
	source: str | None = None

	def get_frame(self, data: str = "digitized", detector: str | None = None) -> pd.DataFrame:
		"""Return a DataFrame from a named data stream.

		If ``detector`` is omitted and the stream has exactly one table, that table is
		returned. If the stream has several tables, they are concatenated and tagged
		with a ``detector`` column when needed.
		"""

		frames = self._frames_for(data)
		if detector is not None:
			try:
				return frames[detector]
			except KeyError as exc:
				available = ", ".join(sorted(frames)) or "<none>"
				raise KeyError(f"Detector '{detector}' not found. Available: {available}") from exc

		if not frames:
			raise ValueError(f"No '{data}' data are loaded.")
		if len(frames) == 1:
			return next(iter(frames.values()))

		tagged = []
		for name, frame in frames.items():
			if "detector" in frame.columns:
				tagged.append(frame)
			else:
				tagged.append(frame.assign(detector=name))
		return pd.concat(tagged, ignore_index=True)

	def available(self, data: str = "digitized") -> list[str]:
		"""Return available table names for a stream."""

		return sorted(self._frames_for(data))

	def summary(self) -> str:
		"""Return a compact text summary."""

		lines = []
		if self.source:
			lines.append(f"source: {self.source}")
		for label, frames in (
			("true_info", self.true_info),
			("digitized", self.digitized),
			("headers", self.headers),
		):
			if frames:
				parts = [f"{name}({len(frame)} rows)" for name, frame in sorted(frames.items())]
				lines.append(f"{label}: " + ", ".join(parts))
			else:
				lines.append(f"{label}: <none>")
		return "\n".join(lines)

	def _frames_for(self, data: str) -> FrameMap:
		if data == "true_info":
			return self.true_info
		if data == "digitized":
			return self.digitized
		if data == "headers":
			return self.headers
		raise ValueError("data must be one of: 'digitized', 'true_info', 'headers'")
