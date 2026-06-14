from pathlib import Path
import subprocess

BASE_DIR = Path(__file__).parent.parent


def run_ps1(script_name: str, **kwargs) -> dict:
    try:
        script_path = BASE_DIR / script_name

        cmd = [
            "powershell",
            "-ExecutionPolicy",
            "Bypass",
            "-File",
            str(script_path),
        ]

        for key, value in kwargs.items():
            if value is None:
                continue

            cmd.extend([f"-{key}", str(value)])

        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=True,
        )

        return {"success": True, "output": result.stdout}

    except subprocess.CalledProcessError as e:
        return {"success": False, "output": e.stderr or e.stdout}

    except Exception as e:
        return {"success": False, "output": str(e)}


def configure_project(build_type="Debug") -> dict:
    try:
        build_dir = BASE_DIR / "build"
        build_dir.mkdir(exist_ok=True)

        result = subprocess.run(
            [
                "cmake",
                "-S",
                str(BASE_DIR),
                "-B",
                str(build_dir),
                f"-DCMAKE_BUILD_TYPE={build_type}",
            ],
            capture_output=True,
            text=True,
            check=True,
        )

        return {
            "success": True,
            "output": result.stdout,
        }

    except subprocess.CalledProcessError as e:
        return {
            "success": False,
            "output": e.stderr or e.stdout,
        }

    except Exception as e:
        return {
            "success": False,
            "output": str(e),
        }


def build_project(config="Debug") -> dict:
    try:
        build_dir = BASE_DIR / "build"

        result = subprocess.run(
            [
                "cmake",
                "--build",
                str(build_dir),
                "--config",
                config,
            ],
            capture_output=True,
            text=True,
            check=True,
        )

        return {
            "success": True,
            "output": result.stdout,
        }

    except subprocess.CalledProcessError as e:
        return {
            "success": False,
            "output": e.stderr or e.stdout,
        }

    except Exception as e:
        return {
            "success": False,
            "output": str(e),
        }
