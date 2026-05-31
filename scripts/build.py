import sys
from run.cmd import run_ps1, configure_project, build_project


def parse_build_type() -> str:
    build_type = "Debug"

    if len(sys.argv) > 1:
        arg = sys.argv[1].lower()

        if arg == "debug":
            build_type = "Debug"
        elif arg == "release":
            build_type = "Release"
        else:
            print(f"Unknown build type: {sys.argv[1]}")
            sys.exit(1)

    return build_type


def run_step(description: str, script: str):
    print(f"{description}...")

    result = run_ps1(script)

    print(f"End process of {description.lower()}")

    if not result["success"]:
        print(f"Failed to execute '{script}': {result['output']}")
        sys.exit(1)

    if result["output"]:
        print(result["output"])


def main():
    build_type = parse_build_type()

    # SDL2
    run_step("Getting SDL2", "get_sdl2.ps1")
    run_step("Building SDL2", "build_sdl2.ps1")

    # bgfx
    run_step("Getting bgfx", "get_bgfx.ps1")
    run_step("Building bgfx", "build_bgfx.ps1")

    # entt (header only)
    run_step("Getting entt", "get_entt.ps1")

    # tinygltf
    run_step("Getting tinygltf", "get_tinygltf.ps1")
    run_step("Validating tinygltf", "validate_tinygltf.ps1")

    # configure project
    print(f"Configuring project ({build_type})...")
    result = configure_project(build_type=build_type)

    print("End process of configuring project")

    if not result["success"]:
        print(f"Failed to configure project: {result['output']}")
        sys.exit(1)

    if result["output"]:
        print(result["output"])

    # build project
    print(f"Building project ({build_type})...")
    result = build_project(build_type=build_type)

    print("End process of building project")

    if not result["success"]:
        print(f"Failed to build project: {result['output']}")
        sys.exit(1)

    if result["output"]:
        print(result["output"])

    print("All tasks completed successfully!")


if __name__ == "__main__":
    main()
