from pkg_resources import get_distribution, DistributionNotFound

try:
    __version__ = get_distribution(__name__).version
except DistributionNotFound:
    # package is not installed
    pass

if "AUTO_INIT_TCG" not in ${...}:
    $AUTO_INIT_TCG = True


def initialize_tcg():
    cg = $(tcg create).strip()
    if cg:
        $TITLE = $TITLE + f'@{cg}'
        $PROMPT = f'{{BOLD_RED}}[{cg}]{{RESET}} ' + $PROMPT
        print(f"Your cgroup is {cg}.")
    else:
        print("Unable to create cgroup for shell.")


if $AUTO_INIT_TCG:
    initialize_tcg()
