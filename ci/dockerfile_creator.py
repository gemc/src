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


def remote_startup_dir() -> str:
    return '/usr/local/bin'

def remote_entrypoint():
    return f'{remote_startup_dir()}/docker-entrypoint.sh'

def remote_entrypoint_addon():
    return f'{remote_startup_dir()}/additional-entrycommands.sh'


def docker_header(image: str, image_tag: str, geant4_tag: str) -> str:
    commands = f"FROM {g4_registry}:{geant4_tag}-{image}-{image_tag} AS final\n"
    commands += f"LABEL maintainer=\"Maurizio Ungaro <ungaro@jlab.org>\"\n\n"
    commands += f"# run bash instead of sh\n"
    commands += f"SHELL [\"/bin/bash\", \"-c\"]\n\n"
    commands += f"ENV AUTOBUILD=1\n"
    return commands


def install_gemc(geant4_version: str, gemc_version: str, source: str) -> str:
    if source == "context":
        commands = f'\nCOPY . /root/src \n'
        commands += f'RUN  cd /root/src \\\n'
        commands += f"     && DOCKER_ENTRYPOINT_SOURCE_ONLY=1 . {remote_entrypoint()} \\\n"
        commands += f'     && module load geant4/{geant4_version} \\\n'
        commands += f'     &&  ./ci/build.sh  \\\n'
        commands += f'     && echo "export PATH=\\${{SIM_HOME}}/gemc/dev/bin:\\${{SIM_HOME}}/gemc/dev/python_env/bin:\\${{PATH}}" >> {remote_entrypoint_addon()} \n'
        return commands

    clone_arguments = f'-c advice.detachedHead=false --recurse-submodules --single-branch'
    if gemc_version == "dev":
        clone_arguments += ' --depth 1'
    else:
        clone_arguments += f' --branch {gemc_version}'
    commands = f'\nRUN  git clone {clone_arguments} http://github.com/gemc/src /root/src \\\n'
    commands += f'     && cd /root/src \\\n'
    commands += f"     && DOCKER_ENTRYPOINT_SOURCE_ONLY=1 . {remote_entrypoint()} \\\n"
    commands += f'     && module load geant4/{geant4_version} \\\n'
    commands += f'     &&  ./ci/build.sh  \\\n'
    commands += f'     && echo "export PATH=\\${{SIM_HOME}}/gemc/dev/bin:\\${{SIM_HOME}}/gemc/dev/python_env/bin:\\${{PATH}}" >> {remote_entrypoint_addon()} \n'
    return commands


def package_install(geant4_version: str, gemc_version: str, image: str, image_tag: str, package_arch: str) -> str:
    package_name = f'gemc-{gemc_version}-geant4-{geant4_version}-{image}-{image_tag}-{package_arch}'
    commands = '\n# release tarball build \n'
    commands += 'FROM final AS package-build \n'
    commands += f'RUN  cd /root/src \\\n'
    commands += f"     && DOCKER_ENTRYPOINT_SOURCE_ONLY=1 . {remote_entrypoint()} \\\n"
    commands += f'     && module load geant4/{geant4_version} \\\n'
    commands += f'     && eval "$(geant4-config --sh)" \\\n'
    commands += f'     && GEANT4_VERSION={geant4_version} GEMC_PACKAGE_VERSION={gemc_version} \\\n'
    commands += f'        ./ci/package_install.sh "${{SIM_HOME}}/gemc/dev" /root/src/dist "{package_name}" \n'
    return commands


def log_exporters() -> str:
    commands = '\n# logs exporter \n'
    commands += 'FROM scratch AS logs-export \n'
    commands += 'COPY --from=final /root/src/logs /logs \n'
    return commands


def package_exporters() -> str:
    commands = '\n# release package exporter \n'
    commands += 'FROM scratch AS package-export \n'
    commands += 'COPY --from=package-build /root/src/dist / \n'
    return commands


def create_dockerfile(
    image: str,
    image_tag: str,
    geant4_version: str,
    gemc_version: str,
    with_package: bool = False,
    source: str = "clone",
    package_arch: str = "amd64",
) -> str:
    commands = ""
    commands += docker_header(image, image_tag, geant4_version)
    commands += install_gemc(geant4_version, gemc_version, source)
    commands += log_exporters()
    if with_package:
        commands += package_install(geant4_version, gemc_version, image, image_tag, package_arch)
        commands += package_exporters()

    return commands


import argparse
import sys


def main():
    parser = argparse.ArgumentParser(
        description="Print a dockerfile with install commands for a given base image and tag, gemc and geant4 versions",
        epilog="Example: python3 ./ci/dockerfile_creator.py -i fedora -t 42 --geant4-version 11.4.1 --gemc-version dev"
    )
    # Required *conceptually*, but we want: if missing, show usage (not a long error)
    parser.add_argument(
        "-i", "--image",
        help="Target base os (e.g., fedora, almalinux, ubuntu, debian, archlinux)"
    )
    parser.add_argument(
        "-t", "--image_tag",
        help="Base image tag (e.g., 42 for fedora, 24.04 for ubuntu, etc.)"
    )
    parser.add_argument(
        "--geant4-version", default="11.4.1",
        help="Version of Geant4 to install (default: %(default)s)"
    )
    parser.add_argument(
        "--gemc-version", default="dev",
        help="Version of GEMC to install (default: %(default)s)"
    )
    parser.add_argument(
        "--with-package", action="store_true",
        help="Add a package-export stage that emits a GEMC install tarball"
    )
    parser.add_argument(
        "--source", choices=["clone", "context"], default="clone",
        help="Build GEMC from a GitHub clone or from the Docker build context"
    )
    parser.add_argument(
        "--package-arch", choices=["amd64", "arm64"], default="amd64",
        help="Architecture suffix to use in the package artifact name"
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
        args.with_package,
        args.source,
        args.package_arch,
    )
    print(dockerfile)


# ------------------------------------------------------------------------------
if __name__ == "__main__":
    main()
