import argparse
import tempfile
import shutil
from pathlib import Path


def main(build_dir: Path, output_zip: Path):
    if not build_dir.is_dir():
        print(f"Error: '{build_dir}' is not a valid directory.")
        return

    with tempfile.TemporaryDirectory() as temp_plugin_dir:
        print(f"Created temporary directory: {temp_plugin_dir}")

        for subdir in build_dir.iterdir():
            if not subdir.is_dir():
                continue

            if subdir.name in ["JuceLibraryCode"]:
                print(f"Skipping '{subdir}'")
                continue

            for plugin in subdir.iterdir():
                target = Path(temp_plugin_dir) / plugin.name

                print(f"Copying '{plugin}' to '{target}'")

                if plugin.is_dir():
                    shutil.copytree(plugin, target)
                else:
                    shutil.copy2(plugin, target)

        with tempfile.TemporaryDirectory() as temp_archive_dir:
            temp_zip = Path(temp_archive_dir) / "archive"

            print(f"Creating archive: {temp_zip}.zip")
            shutil.make_archive(temp_zip, 'zip', temp_plugin_dir)

            print(f"Moving archive to: {output_zip}")
            shutil.move(temp_zip.with_suffix('.zip'), output_zip)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("build_dir", help="Path to the build directory", type=Path)
    parser.add_argument("output_zip", help="Path for the output zip file", type=Path)
    args = parser.parse_args()
    main(args.build_dir.resolve(), args.output_zip.resolve())
