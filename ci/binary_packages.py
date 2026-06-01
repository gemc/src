#!/usr/bin/env python3
import argparse

valid_images = ["fedora", "ubuntu", "archlinux", "almalinux", "debian"]


def map_family(image: str) -> str:
	if image in ("almalinux", "centos"):
		return "fedora"
	if image == "ubuntu":
		return "debian"
	return image


def unique_preserve_order(items):
	seen = set()
	out = []
	for item in items:
		if item not in seen:
			seen.add(item)
			out.append(item)
	return out


pkg_sections = {
	"download_unpack": {
		"fedora": ["ca-certificates", "curl", "gzip", "tar"],
		"debian": ["ca-certificates", "curl", "gzip", "tar"],
		"archlinux": ["ca-certificates", "curl", "gzip", "tar"],
	},
	"core_runtime": {
		"fedora": ["expat", "sqlite-libs", "zlib"],
		"debian": ["libexpat1", "libsqlite3-0", "zlib1g"],
		"archlinux": ["expat", "sqlite", "zlib"],
	},
	"x11_gl": {
		"fedora": ["libX11", "libXext", "libXmu", "libXt", "mesa-libEGL", "mesa-libGL"],
		"debian": ["libegl1", "libgl1", "libx11-6", "libxext6", "libxmu6", "libxt6"],
		"archlinux": ["libx11", "libxext", "libxmu", "libxt", "mesa"],
	},
	"qt6": {
		"fedora": ["qt6-qtbase", "qt6-qtsvg"],
		"debian": ["libqt6core6t64", "libqt6gui6", "libqt6widgets6", "libqt6opengl6", "libqt6openglwidgets6", "libqt6svg6"],
		"archlinux": ["qt6-base", "qt6-svg"],
	},
	"other_linked_runtime": {
		"fedora": ["tbb"],
		"debian": ["libtbb12"],
		"archlinux": ["tbb"],
	},
}


def packages_to_be_installed(image: str, tag: str = "") -> str:
	if image not in valid_images:
		raise SystemExit(f"invalid image '{image}'; valid images: {', '.join(sorted(valid_images))}")

	family = map_family(image)
	packages = []
	for section in pkg_sections.values():
		packages.extend(section.get(family, []))

	return " ".join(unique_preserve_order(packages))


def packages_install_command(image: str, tag: str = "") -> str:
	family = map_family(image)
	packages = packages_to_be_installed(image, tag)
	log = "/tmp/gemc-binary-packages-install.log"

	if family == "fedora":
		return (
			"RUN /bin/bash -lc 'set -euo pipefail; "
			f"dnf install -y --allowerasing {packages} >{log} 2>&1 "
			f"|| {{ rc=$?; cat {log}; exit $rc; }}'"
		)

	if family == "debian":
		return (
			"ENV DEBIAN_FRONTEND=noninteractive\n"
			"ENV DEBCONF_NONINTERACTIVE_SEEN=true\n"
			"ENV TZ=UTC\n"
			"RUN /bin/bash -lc 'set -euo pipefail; "
			"ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime && "
			"apt-get update && "
			f"apt-get install -y --no-install-recommends tzdata {packages} >{log} 2>&1 "
			f"|| {{ rc=$?; cat {log}; exit $rc; }}'"
		)

	if family == "archlinux":
		return (
			"RUN /bin/bash -lc 'set -euo pipefail; "
			f"pacman -Syu --noconfirm --needed {packages} >{log} 2>&1 "
			f"|| {{ rc=$?; cat {log}; exit $rc; }}'"
		)

	return ""


def main():
	parser = argparse.ArgumentParser(description="Return runtime packages for GEMC binary tarball tests")
	parser.add_argument("-i", "--image", required=True, help="Target base OS")
	parser.add_argument("-t", "--tag", default="", help="Target base OS tag")
	parser.add_argument("--command", action="store_true", help="Print a Dockerfile RUN command instead of the package list")
	args = parser.parse_args()

	if args.command:
		print(packages_install_command(args.image, args.tag))
	else:
		print(packages_to_be_installed(args.image, args.tag))


if __name__ == "__main__":
	main()
