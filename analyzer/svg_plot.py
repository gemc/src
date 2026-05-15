"""Dependency-free SVG plots for GEMC CSV output."""

from __future__ import annotations

import argparse
import csv
import html
from collections import defaultdict
from pathlib import Path


def read_csv_column(path: Path, variable: str, group_by: str | None = "pid") -> dict[str, list[float]]:
	"""Read one numeric CSV column, grouped by another column when present."""

	groups: dict[str, list[float]] = defaultdict(list)
	with path.open(newline="") as csv_file:
		reader = csv.DictReader(csv_file, skipinitialspace=True)
		if reader.fieldnames is None or variable not in reader.fieldnames:
			available = ", ".join(reader.fieldnames or [])
			raise KeyError(f"Column '{variable}' not found. Available columns: {available}")
		has_group = group_by is not None and group_by in reader.fieldnames

		for row in reader:
			try:
				value = float(row[variable])
			except (TypeError, ValueError):
				continue
			group = row[group_by] if has_group else "all"
			groups[group].append(value)

	return dict(groups)


def write_histogram_svg(
	groups: dict[str, list[float]],
	output: Path,
	variable: str,
	*,
	bins: int = 30,
	xlim: tuple[float, float] | None = None,
) -> None:
	"""Write a grouped histogram as a simple SVG file."""

	values = [value for group_values in groups.values() for value in group_values]
	if not values:
		raise ValueError(f"No numeric values found for '{variable}'.")

	xmin, xmax = xlim if xlim is not None else (min(values), max(values))
	if xmin == xmax:
		xmin -= 0.5
		xmax += 0.5

	width = 960
	height = 600
	margin_left = 76
	margin_right = 26
	margin_top = 48
	margin_bottom = 76
	plot_width = width - margin_left - margin_right
	plot_height = height - margin_top - margin_bottom
	bin_width = (xmax - xmin) / bins
	colors = ["#4e79a7", "#f28e2b", "#e15759", "#76b7b2", "#59a14f", "#edc948", "#b07aa1", "#ff9da7"]

	counts_by_group = {}
	max_count = 1
	for group, group_values in sorted(groups.items()):
		counts = [0] * bins
		for value in group_values:
			if value < xmin or value > xmax:
				continue
			index = min(int((value - xmin) / bin_width), bins - 1)
			counts[index] += 1
		counts_by_group[group] = counts
		max_count = max(max_count, max(counts, default=0))

	def x_for(index: int) -> float:
		return margin_left + index * plot_width / bins

	def y_for(count: int) -> float:
		return margin_top + plot_height - count * plot_height / max_count

	bar_group_width = plot_width / bins
	bar_width = max(1.0, bar_group_width / max(1, len(counts_by_group)) - 1)

	parts = [
		f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
		'<rect width="100%" height="100%" fill="white"/>',
		f'<text x="{width / 2}" y="28" text-anchor="middle" font-family="Arial" font-size="20">{html.escape(variable)} histogram</text>',
		f'<line x1="{margin_left}" y1="{margin_top + plot_height}" x2="{margin_left + plot_width}" y2="{margin_top + plot_height}" stroke="#333"/>',
		f'<line x1="{margin_left}" y1="{margin_top}" x2="{margin_left}" y2="{margin_top + plot_height}" stroke="#333"/>',
	]

	for group_index, (group, counts) in enumerate(counts_by_group.items()):
		color = colors[group_index % len(colors)]
		for bin_index, count in enumerate(counts):
			if count == 0:
				continue
			x = x_for(bin_index) + group_index * (bar_width + 1)
			y = y_for(count)
			h = margin_top + plot_height - y
			parts.append(f'<rect x="{x:.2f}" y="{y:.2f}" width="{bar_width:.2f}" height="{h:.2f}" fill="{color}" opacity="0.78"/>')

	for tick in range(6):
		fraction = tick / 5
		x_value = xmin + fraction * (xmax - xmin)
		x = margin_left + fraction * plot_width
		parts.append(f'<line x1="{x:.2f}" y1="{margin_top + plot_height}" x2="{x:.2f}" y2="{margin_top + plot_height + 5}" stroke="#333"/>')
		parts.append(f'<text x="{x:.2f}" y="{margin_top + plot_height + 24}" text-anchor="middle" font-family="Arial" font-size="12">{x_value:.4g}</text>')

	for tick in range(5):
		count = round(tick * max_count / 4)
		y = y_for(count)
		parts.append(f'<line x1="{margin_left - 5}" y1="{y:.2f}" x2="{margin_left}" y2="{y:.2f}" stroke="#333"/>')
		parts.append(f'<text x="{margin_left - 10}" y="{y + 4:.2f}" text-anchor="end" font-family="Arial" font-size="12">{count}</text>')

	parts.append(f'<text x="{width / 2}" y="{height - 22}" text-anchor="middle" font-family="Arial" font-size="14">{html.escape(variable)}</text>')
	parts.append(f'<text x="22" y="{height / 2}" transform="rotate(-90 22 {height / 2})" text-anchor="middle" font-family="Arial" font-size="14">Counts</text>')

	legend_x = width - margin_right - 150
	legend_y = margin_top + 10
	for group_index, group in enumerate(counts_by_group):
		y = legend_y + group_index * 20
		color = colors[group_index % len(colors)]
		parts.append(f'<rect x="{legend_x}" y="{y - 10}" width="12" height="12" fill="{color}" opacity="0.78"/>')
		parts.append(f'<text x="{legend_x + 18}" y="{y}" font-family="Arial" font-size="12">pid {html.escape(group)}</text>')

	parts.append("</svg>")
	output.write_text("\n".join(parts) + "\n")


def main(argv: list[str] | None = None) -> None:
	parser = argparse.ArgumentParser(description="Create a dependency-free SVG histogram from GEMC CSV output.")
	parser.add_argument("csv_file", type=Path)
	parser.add_argument("variable")
	parser.add_argument("--out", type=Path, required=True)
	parser.add_argument("--bins", type=int, default=30)
	parser.add_argument("--group-by", default="pid")
	parser.add_argument("--xlim", nargs=2, type=float, metavar=("LOW", "HIGH"))
	args = parser.parse_args(argv)

	groups = read_csv_column(args.csv_file, args.variable, args.group_by)
	write_histogram_svg(
		groups,
		args.out,
		args.variable,
		bins=args.bins,
		xlim=tuple(args.xlim) if args.xlim else None,
	)
	print(args.out)


if __name__ == "__main__":
	main()
