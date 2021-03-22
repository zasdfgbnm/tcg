import subprocess
import os

try:
    from importlib.metadata import version, PackageNotFoundError
    try:
        __version__ = version("package-name")
    except PackageNotFoundError:
        # package is not installed
        pass
except (ModuleNotFoundError, ImportError):
    # importlib.metadata not available
    pass


if "AUTO_INIT_TCG" not in ${...}:
    $AUTO_INIT_TCG = True


tcg_executable = $(which tcg)


def initialize_tcg():
    $TERMINAL_CGROUP = $(tcg create).strip()
    if $TERMINAL_CGROUP:
        $TITLE = $TITLE + f'@{$TERMINAL_CGROUP}'
        $PROMPT = f'{{BOLD_RED}}[{$TERMINAL_CGROUP}]{{RESET}} ' + $PROMPT
        print(f"Your cgroup is {$TERMINAL_CGROUP}.")
    else:
        print("Unable to create cgroup for shell.")


def tcg_tab_complete(line):
    line = line.split(' ')
    command = line[0]

    def is_tcg(command):
        if command == 'tcg':
            return True
        command = os.path.expanduser(command)
        return os.path.isfile(command) and os.path.samefile(command, tcg_executable)

    if is_tcg(command):
        try:
            items = subprocess.check_output([tcg_executable, "tab-complete", *line[1:]], stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            return set()
        except subprocess.CalledProcessError:
            return set()
        items = items.decode("utf-8").splitlines()
        return set(items)
    return


def tcg_completer(prefix, line, begidx, endidx, ctx):
    """Completer for `tcg`"""
    return tcg_tab_complete(line)


completer add tcg tcg_completer

if $AUTO_INIT_TCG:
    initialize_tcg()
