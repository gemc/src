"""Python helpers for reading and plotting GEMC output."""

__all__ = ["GemcOutput", "plot_variable", "read_output"]


def __getattr__(name):
	if name == "GemcOutput":
		from .dataset import GemcOutput

		return GemcOutput
	if name == "plot_variable":
		from .plotting import plot_variable

		return plot_variable
	if name == "read_output":
		from .readers import read_output

		return read_output
	raise AttributeError(f"module 'analyzer' has no attribute {name!r}")
