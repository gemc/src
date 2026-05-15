"""Small command line interface for GEMC output analysis."""

from __future__ import annotations

import argparse
from pathlib import Path


def build_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read and plot GEMC CSV or ROOT output.")
	parser.add_argument("path", help="Output file, ROOT file, CSV file, or CSV rootname.")
	parser.add_argument("variable", nargs="?", help="Variable to plot, for example 'totEdep'.")
	parser.add_argument(
		"--kind",
		choices=("auto", "csv", "root"),
		default="auto",
		help="Input format. Default: auto.",
	)
	parser.add_argument(
		"--data",
		choices=("digitized", "true_info"),
		default="digitized",
		help="Data stream to plot. Default: digitized.",
	)
	parser.add_argument("--detector", help="Detector/tree name to select.")
	parser.add_argument("--bins", type=int, default=30, help="Histogram bin count. Default: 30.")
	parser.add_argument("--xlim", nargs=2, type=float, metavar=("LOW", "HIGH"), help="X axis limits.")
	parser.add_argument("--linear-y", action="store_true", help="Use a linear y axis instead of log scale.")
	parser.add_argument("--save", type=Path, help="Write the figure to this path instead of showing it.")
	return parser


def main(argv: list[str] | None = None) -> None:
	args = build_parser().parse_args(argv)

	if args.save:
		import matplotlib

		matplotlib.use("Agg")

	from .plotting import plot_variable
	from .readers import read_output

	output = read_output(args.path, kind=args.kind)

	if not args.variable:
		print(output.summary())
		return

	fig, _ = plot_variable(
		output,
		args.variable,
		data=args.data,
		detector=args.detector,
		bins=args.bins,
		xlim=tuple(args.xlim) if args.xlim else None,
		logy=not args.linear_y,
	)

	if args.save:
		fig.savefig(args.save, bbox_inches="tight")
	else:
		from matplotlib import pyplot as plt

		plt.show()
