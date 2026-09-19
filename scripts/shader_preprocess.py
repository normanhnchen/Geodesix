import argparse
from pathlib import Path


# This file lives at /scripts/shader_preprocess.py
PROJECT_ROOT = Path(__file__).resolve().parent.parent


def preprocess_shader(file_path: Path, visited_paths=[], is_root=True):
    """
    Preprocess shader files for custom #include support.

    Include paths are relative to the project root. They also must be defined in the shader with a
    #include followed by the include path in strings ("[path]").

    Args:
        visited_paths: The current chain of includes, used to prevent circular dependencies.
        is_root: Used to make sure that only the root file has the #version declaration.
    
    Returns:
        The final concatenated preprocessed shader.

    Raises:
        RuntimeError: If there is a circular dependency with #include files.
        FileNotFoundError: If the #include file path does not exist.
    """

    file_path = Path(file_path).resolve()

    if str(file_path) in visited_paths:
        chain = " -> ".join([*visited_paths, str(file_path)])
        raise RuntimeError(f"Circular shader include: {chain}")

    visited_paths.append(str(file_path))

    output = ""
    version = ""
    with open(file_path) as f:
        for line_num, line in enumerate(f, 1):
            if line.strip().startswith("#include"):
                include_name = line.strip().replace('"', "").split()[1]
                include_abs_path = (PROJECT_ROOT / include_name).resolve()

                if not include_abs_path.is_file():
                    raise FileNotFoundError(
                        f"{file_path}:{line_num}: cannot find include '{include_name}' "
                        f"(looked in {include_abs_path})"
                    )

                included = preprocess_shader(include_abs_path, visited_paths, is_root=False)
                output += included
                if not included.endswith("\n"):
                    output += "\n"
            # The very first line of the shader file must be the version declaration
            elif line.strip().startswith("#version"):
                if is_root and not version:
                    version = line
            else:
                output += line

    # Remove the current file from the visited paths as it is done reading
    visited_paths.pop()

    # Include the #version declaration which must be the first line of the file
    return f"{version}\n{output}"


def main():
    parser = argparse.ArgumentParser()
    # NOTE: must match the exact CMake script flags in the command for preprocessesing shaders
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--dest", required=True, type=Path)

    # Run the script
    args = parser.parse_args()

    # Make sure the destination path exists
    args.dest.parent.mkdir(parents=True, exist_ok=True)

    output = preprocess_shader(args.source)

    args.dest.write_text(output)


if __name__ == "__main__":
    main()