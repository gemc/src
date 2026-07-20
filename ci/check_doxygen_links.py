#!/usr/bin/env python3

"""Check generated Doxygen HTML for references to missing local files."""

from __future__ import annotations

import argparse
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit


class ReferenceParser(HTMLParser):
    """Collect local-file candidates from HTML link and media attributes."""

    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.references: list[str] = []

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        del tag
        for name, value in attrs:
            if name in {"href", "src"} and value:
                self.references.append(value)


def missing_references(site_root: Path) -> list[tuple[Path, str]]:
    """Return the generated pages and references whose local targets do not exist."""

    missing: set[tuple[Path, str]] = set()
    for page in site_root.rglob("*.html"):
        parser = ReferenceParser()
        parser.feed(page.read_text(encoding="utf-8", errors="replace"))

        for reference in parser.references:
            parsed = urlsplit(reference)
            if parsed.scheme or parsed.netloc or not parsed.path:
                continue

            path = unquote(parsed.path)
            target = site_root / path.lstrip("/") if path.startswith("/") else page.parent / path
            if not target.resolve().exists():
                missing.add((page.relative_to(site_root), reference))

    return sorted(missing, key=lambda item: (str(item[0]), item[1]))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("site_root", nargs="?", default="pages", type=Path)
    args = parser.parse_args()

    missing = missing_references(args.site_root)
    if not missing:
        print(f"Doxygen link check passed: {args.site_root}")
        return 0

    print(f"Doxygen link check failed: {len(missing)} missing local targets")
    for page, reference in missing:
        print(f"  {page}: {reference}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
