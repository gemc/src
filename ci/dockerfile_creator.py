#!/usr/bin/env python3

valid_images = ["fedora", "ubuntu", "archlinux", "almalinux", "debian"]
g4_registry='ghcr.io/gemc/g4install'

def is_valid_image(image: str) -> bool:
    if image in valid_images:
        return True
    else:
        print(f"Error: invalid image '{image}'")
        print(f"Valid images: {available_images()}")
        exit(1)


def available_images() -> str:
    return ', '.join(sorted(valid_images))


# def remote_novnc_startup_script() -> str:
# 	return '/usr/local/bin/start-novnc'


def docker_header(image: str, image_tag: str, geant4_tag: str) -> str:
    commands = f"FROM {g4_registry}:{geant4_tag}-{image}-{image_tag}\n"
    commands += f"LABEL maintainer=\"Maurizio Ungaro <ungaro@jlab.org>\"\n\n"
    commands += f"# run bash instead of sh\n"
    commands += f"SHELL [\"/bin/bash\", \"-c\"]\n\n"
    # commands += f"# Make browser UI the default; users can override with \"docker run ... bash -l\"\n"
    # commands += f"CMD [\"{remote_novnc_startup_script()}\"]\n\n"
    commands += f"ENV AUTOBUILD=1\n"
    return commands


def install_gemc(geant4_version: str, gemc_version: str) -> str:
    clone_arguments = f'-c advice.detachedHead=false --recurse-submodules --single-branch'
    if gemc_version == "dev":
        clone_arguments += ' --depth 1'
    else:
        clone_arguments += f'--branch "{gemc_version}'
    commands = f'\nRUN  git clone {clone_arguments} http://github.com/gemc/src /root/src \\\n'
    commands += f'     && cd /root/src \\\n'
    commands += f'     && module load geant4/{geant4_version} \\\n'
    commands += f'     &&  ./ci/build.sh \n'
    return commands


def log_exporters() -> str:
    commands = '\n# logs exporter \n'
    commands += 'FROM scratch AS logs-export \n'
    commands += 'COPY --from=final /root/src/logs /logs \n'
    return commands


def create_dockerfile(image: str, image_tag: str, geant4_version: str, gemc_version: str) -> str:
    commands = ""
    commands += docker_header(image, image_tag, geant4_version)
    commands += install_gemc(geant4_version, gemc_version)
    commands += log_exporters()

    return commands


import argparse
import sys


def main():
    parser = argparse.ArgumentParser(
        description="Print a dockerfile with install commands for a given base image and tag, gemc and geant4 versions",
        epilog="Example: python3 ./ci/dockerfile_creator.py -i fedora -t 40 --geant4-version 11.4.0 --gemc-version dev"
    )
    # Required *conceptually*, but we want: if missing, show usage (not a long error)
    parser.add_argument(
        "-i", "--image",
        help="Target base os (e.g., fedora, almalinux, ubuntu, debian, archlinux)"
    )
    parser.add_argument(
        "-t", "--image_tag",
        help="Base image tag (e.g., 40 for fedora, 24.04 for ubuntu, etc.)"
    )
    parser.add_argument(
        "--geant4-version", default="11.4.0",
        help="Version of Geant4 to install (default: %(default)s)"
    )
    parser.add_argument(
        "--gemc-version", default="dev",
        help="Version of GEMC to install (default: %(default)s)"
    )

    args = parser.parse_args()

    # 1) If -i/--image or -t/--tag are not given, print usage and exit
    if not args.image or not args.image_tag:
        parser.print_usage(sys.stderr)
        sys.exit(2)

    is_valid_image(args.image)

    dockerfile = create_dockerfile(
        args.image,
        args.image_tag,
        args.geant4_version,
        args.gemc_version,
    )
    print(dockerfile)


# ------------------------------------------------------------------------------
if __name__ == "__main__":
    main()
