from pkg_resources import get_distribution, DistributionNotFound

try:
    __version__ = get_distribution(__name__).version
except DistributionNotFound:
    # package is not installed
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
