from pathlib import Path
import subprocess
from sys import argv as args

def run_command(command):
    print(command)
    subprocess.run(command, shell=True)


CSOURCES = [f for f in Path("source").rglob("*.c")]
CPPSOURCES = [f for f in Path("source").rglob("*.cpp")]
SOURCES = CSOURCES + CPPSOURCES
OBJECTS = []
BIN     = Path("bin")
TARGET  = BIN / "libjuve.a"
CC = "clang"
CXX = "clang++"
    
def build():
    for src in SOURCES:
        objfile = BIN / (src.stem + ".o")
        OBJECTS.append(str(objfile))
        if (src.stat().st_mtime > objfile.stat().st_mtime):
            if src.suffix == ".c":
                run_command(f"{CC} -o {str(objfile)} -c {str(src)} -Isource -I.")
            elif src.suffix == ".cpp":
                run_command(f"{CXX} -o {str(objfile)} -c {str(src)} -Isource -I.")
                
    run_command(f"ar -rcs {TARGET} {" ".join(OBJECTS)}")
        
if __name__ == "__main__":
    build()

    run_command(f"{CC} test.c -o {str(BIN / "test")} -Isource")
