"""Plotting helpers for GEMC output tables."""

from __future__ import annotations

from collections.abc import Mapping
from typing import Any

import matplotlib.colors as mcolors
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from .dataset import GemcOutput


DEFAULT_LABELS: Mapping[str, str] = {
	"totEdep": "Total Energy Deposit (MeV)",
	"totalEDeposited": "Total Energy Deposited (MeV)",
	"etot": "Total Energy (MeV)",
	"dose": "Dose",
	"time": "Time",
	"avgTime": "Average Time",
	"E": "Energy (MeV)",
	"totalE": "Total Energy (MeV)",
	"p": "Momentum (MeV)",
	"theta": "Theta (rad)",
	"phi": "Phi (rad)",
	"vx": "Track Vertex X (mm)",
	"vy": "Track Vertex Y (mm)",
	"vz": "Track Vertex Z (mm)",
	"mvx": "Mother Track Vertex X (mm)",
	"mvy": "Mother Track Vertex Y (mm)",
	"mvz": "Mother Track Vertex Z (mm)",
	"mtid": "Mother Track ID",
}

VARIABLE_ALIASES: Mapping[str, tuple[str, ...]] = {
	"E": ("totalE", "etot"),
	"totalEDeposited": ("totEdep",),
	"etot": ("totalE",),
	"track_vx": ("vx",),
	"track_vy": ("vy",),
	"track_vz": ("vz",),
	"mother_vx": ("mvx",),
	"mother_vy": ("mvy",),
	"mother_vz": ("mvz",),
}


def plot_variable(
	output: GemcOutput | pd.DataFrame,
	variable: str,
	*,
	data: str = "digitized",
	detector: str | None = None,
	bins: int = 30,
	xlim: tuple[float, float] | None = None,
	group_by: str | None = "pid",
	logy: bool = True,
	ax: plt.Axes | None = None,
	show: bool = False,
	**hist_kwargs: Any,
) -> tuple[plt.Figure, plt.Axes]:
	"""Plot a variable selected by string.

	``data`` selects the GEMC stream and should be either ``digitized`` or
	``true_info`` when ``output`` is a :class:`GemcOutput`.
	"""

	frame = output.get_frame(data=data, detector=detector) if isinstance(output, GemcOutput) else output
	return plot_histogram(
		frame,
		variable,
		bins=bins,
		xlim=xlim,
		group_by=group_by,
		logy=logy,
		ax=ax,
		show=show,
		**hist_kwargs,
	)


def plot_histogram(
	frame: pd.DataFrame,
	variable: str,
	*,
	bins: int = 30,
	xlim: tuple[float, float] | None = None,
	group_by: str | None = "pid",
	logy: bool = True,
	ax: plt.Axes | None = None,
	show: bool = False,
	**hist_kwargs: Any,
) -> tuple[plt.Figure, plt.Axes]:
	"""Plot one numeric variable as a histogram, optionally grouped by a column."""

	variable = _resolve_variable(frame, variable)

	if frame.empty:
		raise ValueError("Selected data table is empty.")

	values = pd.to_numeric(frame[variable], errors="coerce").dropna()
	if values.empty:
		raise ValueError(f"Column '{variable}' has no numeric values to plot.")

	if ax is None:
		fig, ax = plt.subplots(figsize=(10, 6))
	else:
		fig = ax.figure

	data_range = values.clip(*xlim) if xlim is not None else values
	if data_range.min() == data_range.max():
		bin_edges = bins
	else:
		bin_edges = np.linspace(data_range.min(), data_range.max(), bins + 1)

	hist_defaults = {"alpha": 0.7, "edgecolor": "white", "linewidth": 0.5}
	hist_defaults.update(hist_kwargs)

	if group_by and group_by in frame.columns:
		colors = list(mcolors.TABLEAU_COLORS.values())
		for index, group_value in enumerate(sorted(frame[group_by].dropna().unique())):
			subset = pd.to_numeric(frame.loc[frame[group_by] == group_value, variable], errors="coerce").dropna()
			ax.hist(
				subset,
				bins=bin_edges,
				label=f"{group_by} {group_value}",
				color=colors[index % len(colors)],
				**hist_defaults,
			)
		ax.legend(title=group_by)
	else:
		ax.hist(values, bins=bin_edges, **hist_defaults)

	if xlim is not None:
		ax.set_xlim(xlim)
	if logy:
		ax.set_yscale("log")

	ax.set_xlabel(DEFAULT_LABELS.get(variable, variable), fontsize=12)
	ax.set_ylabel("Counts", fontsize=12)
	ax.set_title(f"{DEFAULT_LABELS.get(variable, variable)} Histogram", fontsize=14)
	fig.tight_layout()

	if show:
		plt.show()

	return fig, ax


def _resolve_variable(frame: pd.DataFrame, variable: str) -> str:
	if variable not in frame.columns:
		for alias in VARIABLE_ALIASES.get(variable, ()):
			if alias in frame.columns:
				return alias
		available = ", ".join(frame.columns)
		raise KeyError(f"Column '{variable}' not found. Available columns: {available}")
	return variable
