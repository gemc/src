"""Check dependency generation against installations with and without the optional XR driver."""

from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    generator = Path(__file__).resolve().parents[1] / "meson" / "g4_pkgconfig.py"
    with tempfile.TemporaryDirectory(prefix="gemc-geant4-pc-") as temporary:
        root = Path(temporary)
        library_dir = root / "lib64"
        library_dir.mkdir()
        config = root / "geant4-config"
        output_dir = root / "pkgconfig"

        def generate(export_xr=False):
            responses = {
                "--prefix": str(root),
                "--version": "11.4.3",
                "--cflags": "-I/include/Geant4" + (" -DG4VIS_USE_XR" if export_xr else ""),
                "--libs": f"-L{library_dir} " + ("-lG4visXr " if export_xr else "") + "-lG4run",
            }
            config.write_text(f"#!{sys.executable}\nimport sys\nprint({responses!r}[sys.argv[1]])\n")
            config.chmod(0o755)
            subprocess.run([sys.executable, str(generator), str(output_dir), str(config)],
                           check=True, stdout=subprocess.PIPE, text=True)
            full = (output_dir / "geant4.pc").read_text()
            core = (output_dir / "geant4_core.pc").read_text()
            assert "G4VIS_USE_XR" not in core and "G4visXr" not in core, core
            return full

        full = generate()
        assert "G4VIS_USE_XR" not in full and "G4visXr" not in full, full

        for extension in ("a", "so", "dylib"):
            library = library_dir / f"libG4visXr.{extension}"
            library.touch()
            full = generate()
            assert full.count("-DG4VIS_USE_XR") == 1, full
            assert full.count("-lG4visXr") == 1, full
            assert full.index("-lG4visXr") < full.index("-lG4run"), full
            assert generate(export_xr=True) == full
            before = (output_dir / "geant4.pc").stat().st_mtime_ns
            assert generate(export_xr=True) == full
            assert (output_dir / "geant4.pc").stat().st_mtime_ns == before
            library.unlink()

        full = generate()
        assert "G4VIS_USE_XR" not in full and "G4visXr" not in full, full
    print("Geant4 pkg-config checks passed.")


if __name__ == "__main__":
    main()
