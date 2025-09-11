#!/usr/bin/env python3
import argparse

valid_images = ["11.3.2-fedora-40", "11.3.2-ubuntu-24.04", "11.3.2-archlinux-latest", "11.3.2-almalinux-9.4",
				"11.3.2-debian-12"]


def remote_novnc_startup_script() -> str:
	return '/usr/local/bin/start-novnc'


def remote_setup_filename():
	return '/etc/profile.d/local_g4setup.sh'


def is_valid_image(image: str) -> bool:
	if image in valid_images:
		return True
	else:
		print(f"Error: invalid image '{image}'")
		print(f"Valid images: {available_images()}")
		exit(1)


def available_images() -> str:
	return ', '.join(sorted(valid_images))


def docker_header(image: str) -> str:
	commands = f"FROM ghcr.io/gemc/g4install:{image}\n"
	commands += f"LABEL maintainer=\"Maurizio Ungaro <ungaro@jlab.org>\"\n\n"
	commands += f"# run bash instead of sh\n"
	commands += f"SHELL [\"/bin/bash\", \"-c\"]\n\n"
	commands += f"# Make browser UI the default; users can override with \"docker run ... bash -l\"\n"
	commands += f"CMD [\"{remote_novnc_startup_script()}\"]\n\n"
	commands += f"ENV AUTOBUILD=1\n"
	return commands


def add_load_gemc(tag: str) -> str:
	commands = f'\nRUN echo "module load gemc/{tag}" >> {remote_setup_filename()}\n'
	return commands

def install_gemc(tag: str) -> str:
	commands = f'\nRUN source {remote_setup_filename()} \\\n'
	commands += f'     && RUN git clone -c advice.detachedHead=false --recurse-submodules --single-branch -b {tag}  http://github.com/gemc/src  /root/src \\\n'
	commands += f'     && cd /root/src \\\n'
	commands += f'     &&  ./ci/build.sh \n'
	return commands



def create_dockerfile(image: str, tag: str) -> str:
	commands = ""
	commands += docker_header(image)
	commands += add_load_gemc(tag)
	commands += install_gemc(tag)

	return commands


# ------------------------------------------------------------------------------
def main():
	parser = argparse.ArgumentParser(
		description="Print a dockerfile with install commands for a given base image, image tag and variouse packages versions",
		epilog="Example: ./dockerfile_creator.py -i  11.3.2-almalinux-9.4 -t dev "
	)
	parser.add_argument(
		"-i", "--image", required=True,
		help="Target base os (e.g., 11.3.2-almalinux-9.4, g4install:11.3.2-debian-12"
	)
	parser.add_argument(
		"-t", "--tag", required=True,
		help="Base image tags (e.g., dev)"
	)

	args = parser.parse_args()
	is_valid_image(args.image)

	dockerfile = create_dockerfile(args.image,
								   args.tag)
	print(dockerfile)


# ------------------------------------------------------------------------------
if __name__ == "__main__":
	main()
