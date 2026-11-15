import platform
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT = ROOT / "out"
IS_WIN = platform.system() == "Windows"

def main():
    filename = sys.argv[1]
    if (IS_WIN):
        filename += ".exe"

    src = Path(OUT / filename).resolve()

    print(filename)
    print(src)
    
    if not src.exists():
        print(f"Invalid path, {sys.argv[1]} couldn't be found")
        return

    sys.exit(subprocess.run([str(src), *sys.argv[2:]], cwd=OUT, check=False).returncode)


if __name__ == "__main__":
    main()
