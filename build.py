import json, os, platform, shutil, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT = ROOT / "out"
IS_WIN = platform.system() == "Windows"

def msvc_env():
    """Run vcvars64.bat once, cache the resulting environment."""
    cache = OUT / ".msvc_env.json"
    if cache.exists():
        return json.loads(cache.read_text())
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = subprocess.check_output(
        [vswhere, "-latest", "-property", "installationPath"], text=True).strip()
    vcvars = Path(vs) / "VC/Auxiliary/Build/vcvars64.bat"
    dump = subprocess.check_output(f'"{vcvars}" >nul && set', shell=True, text=True)
    env = dict(l.split("=", 1) for l in dump.splitlines() if "=" in l)
    cache.write_text(json.dumps(env))
    return env

def directives(src):
    d = {"flags": [], "libs": []}
    for line in src.read_text(errors="ignore").splitlines()[:5]:
        line = line.strip()
        for key in d:
            if line.startswith(f"// {key}:"):
                d[key] += line.split(":", 1)[1].split()
    return d


def main():
    src = Path(sys.argv[1]).resolve()
    OUT.mkdir(exist_ok=True)
    exe = OUT / (src.stem + (".exe" if IS_WIN else ""))
    d = directives(src)

    if IS_WIN:
        env = None if shutil.which("cl") else msvc_env()
        path = next(v for k, v in (env or os.environ).items() if k.upper() == "PATH")
        cl = shutil.which("cl", path=path)
        cmd = [cl, "/nologo", "/std:c++latest", "/Zi", "/EHsc", "/W4", *d["flags"],
                str(src), f"/Fe{exe}", f"/Fo{OUT}\\", f"/Fd{OUT}\\",
                "/link", *[f"{l}.lib" for l in d["libs"]]]
    else:
        env = None
        cxx = os.environ.get("CXX", "c++")
        cmd = [cxx, "-std=c++26", "-g", "-Wall", "-Wextra", *d["flags"],
               str(src), "-o", str(exe), *[f"-l{l}" for l in d["libs"]]]

    if subprocess.run(cmd, env=env, cwd=ROOT).returncode != 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
