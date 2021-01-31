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


def initialize_tcg():
    $TERMINAL_CGROUP = $(tcg create).strip()
    if $TERMINAL_CGROUP:
        $TITLE = $TITLE + f'@{$TERMINAL_CGROUP}'
        $PROMPT = f'{{BOLD_RED}}[{$TERMINAL_CGROUP}]{{RESET}} ' + $PROMPT
        print(f"Your cgroup is {$TERMINAL_CGROUP}.")
    else:
        print("Unable to create cgroup for shell.")


if $AUTO_INIT_TCG:
    initialize_tcg()
