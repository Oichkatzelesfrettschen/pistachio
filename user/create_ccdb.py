import json, os, glob
cc = []
base_dir = os.getcwd()
flags = [
    "clang++",
    "-std=c++23",
    "-Iinclude",
    "-Iutil/kickstart",
    "-Ilib/io",
    "-I.",
    "-DREVISION=0",
    "-include",
    "config.h",
    "-c",
]
for f in glob.glob('util/kickstart/*.cc'):
    cc.append({
        "directory": base_dir,
        "command": " ".join(flags + [f]),
        "file": os.path.join(base_dir, f),
    })
json.dump(cc, open('compile_commands.json', 'w'), indent=2)
