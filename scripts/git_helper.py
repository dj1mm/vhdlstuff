
def run_this_command(cmd):
    import subprocess
    res = subprocess.run(cmd.split(' '), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    return res.stdout.decode('ascii').strip(), res.returncode == 0

build_hash = "N/A"
build_tag = "untagged build"
build_branch = "unknown branch"

_, this_is_a_git_repo = run_this_command("git status")
if this_is_a_git_repo:
    build_hash, _ = run_this_command("git log --pretty=format:%h -n 1")
    _, every_files_clean = run_this_command("git diff --quiet --exit-code")
    if not every_files_clean:
        build_hash += "+"

    tag, ok = run_this_command("git describe --exact-match --tags")
    if ok:
        build_tag = tag

    build_branch, _ = run_this_command("git rev-parse --abbrev-ref HEAD")

try:
    with open("version.cpp", "r") as f:
        content = f.read()
except:
    content = ""

output  = [ '#include "version.h"']
output += [f'const char* things::build_hash = "{build_hash}";']
output += [f'const char* things::build_tag = "{build_tag}";']
output += [f'const char* things::build_branch = "{build_branch}";']
output = "\n".join(output)

if content != output:
    with open("version.cpp", "w") as f:
        f.seek(0)
        f.write(output)

