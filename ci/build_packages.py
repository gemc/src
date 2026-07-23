#!/usr/bin/env python3

import argparse
import os
import subprocess


VALID_IMAGES = ["fedora", "ubuntu", "archlinux", "almalinux", "debian"]

BUILD_PACKAGES = {
	"fedora": ["qt6-qtcharts-devel"],
	"debian": ["qt6-charts-dev"],
	"archlinux": ["qt6-charts"],
}


def map_family(image: str) -> str:
	if image == "almalinux":
		return "fedora"
	if image == "ubuntu":
		return "debian"
	return image


def packages_to_be_installed(image: str) -> list[str]:
	if image not in VALID_IMAGES:
		valid = ", ".join(sorted(VALID_IMAGES))
		raise SystemExit(f"invalid image '{image}'; valid images: {valid}")

	return BUILD_PACKAGES[map_family(image)]


def dockerfile_install_command(image: str) -> str:
	family = map_family(image)
	packages = " ".join(packages_to_be_installed(image))
	commands = "\n# install GEMC build-only packages\n"

	if family == "fedora":
		commands += f"RUN  dnf install -y --allowerasing {packages} \\\n"
		commands += "     && dnf clean all \\\n"
		commands += "     && rm -rf /var/cache/dnf\n"
	elif family == "debian":
		commands += "RUN  apt-get update \\\n"
		commands += (
			f"     && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends {packages} \\\n"
		)
		commands += "     && rm -rf /var/lib/apt/lists/*\n"
	elif family == "archlinux":
		commands += f"RUN  pacman -Syu --noconfirm --needed {packages} \\\n"
		commands += "     && pacman -Scc --noconfirm\n"

	return commands


def install_packages(image: str) -> None:
	family = map_family(image)
	packages = packages_to_be_installed(image)

	if family == "fedora":
		subprocess.run(
			["dnf", "install", "-y", "--allowerasing", *packages],
			check=True,
		)
	elif family == "debian":
		subprocess.run(["apt-get", "update"], check=True)
		environment = os.environ.copy()
		environment["DEBIAN_FRONTEND"] = "noninteractive"
		subprocess.run(
			["apt-get", "install", "-y", "--no-install-recommends", *packages],
			check=True,
			env=environment,
		)
	elif family == "archlinux":
		subprocess.run(
			["pacman", "-Syu", "--noconfirm", "--needed", *packages],
			check=True,
		)


def main() -> None:
	parser = argparse.ArgumentParser(description="Install GEMC-only build packages in a CI image")
	parser.add_argument("-i", "--image", required=True, choices=VALID_IMAGES, help="Target base OS")
	parser.add_argument(
		"--command",
		action="store_true",
		help="Print the Dockerfile installation command instead of installing packages",
	)
	args = parser.parse_args()

	if args.command:
		print(dockerfile_install_command(args.image), end="")
	else:
		install_packages(args.image)


if __name__ == "__main__":
	main()
