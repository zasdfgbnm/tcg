import string
import random
import os
import pytest
import multiprocessing
import subprocess
import queue
import timeit
import re
import enum


uid = os.getuid()
ROOT = ''

class Cgroupv2Status(enum.Enum):
    UNAVAILABLE = 1
    PURE = 2
    HYBRID = 3

CGROUP2_STATUS = Cgroupv2Status.UNAVAILABLE

if os.path.isfile('/sys/fs/cgroup/cgroup.procs'):
    CGROUP2_STATUS = Cgroupv2Status.PURE
    ROOT = f'/sys/fs/cgroup/terminals/{uid}/'

if os.path.isfile('/sys/fs/cgroup/unified/cgroup.procs'):
    CGROUP2_STATUS = Cgroupv2Status.HYBRID
    ROOT = f'/sys/fs/cgroup/unified/terminals/{uid}/'

known_commands = {
    "help": ["h"],
    "list": ["ls", "l"],
    "self": ["sf"],
    "create": ["c"],
    "freeze": ["f"],
    "unfreeze": ["uf"],
    "set": [],
    "show": [],
    "tab-complete": [],
    "version": [],
}

def remove_ansi_escape(text):
    # https://stackoverflow.com/a/14693789
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    return ansi_escape.sub('', text)


def random_string(length):
    return ''.join(random.choices(string.ascii_uppercase + string.digits, k=length))


def list_groups():
    return [f for f in os.listdir(ROOT) if os.path.isdir(os.path.join(ROOT, f))]


def list_processes(group):
    with open(os.path.join(ROOT, group, 'cgroup.procs')) as f:
        return f.read()


def install_xontrib():
    path = os.path.dirname(__file__)
    path = os.path.join(path, '../shells/xonsh')
    pushd @(path)
    rm -rf dist
    python setup.py bdist_wheel
    xpip install --force-reinstall dist/*.whl
    popd


def test_invalid_argument():
    pytest.xfail("Failed for unknown reason...")
    with pytest.raises(subprocess.CalledProcessError):
        tcg aaa


def test_help():
    # help for the entire tool
    assert "Usage" in $(tcg help)
    assert "Usage" in $(tcg h)
    assert "Usage" in $(tcg)

    for cmd in known_commands:
        assert cmd in $(tcg h)

    # help for illegal command
    assert "Unknown command" in $(tcg h aaaa)

    # help for known command
    for cmd, alias in known_commands.items():
        desc = $(tcg h @(cmd))
        assert cmd + ": " in desc

        head = "Alias:"
        for l in desc.split('\n'):
            if l.startswith(head):
                alias_line = l[len(head):]
                break
        else:
            alias_line = ""
        for a in alias:
            assert a in alias_line

        for a in alias:
            assert cmd + ": " in $(tcg h @(a))


def test_create_illegal():
    with pytest.raises(subprocess.CalledProcessError):
        tcg create aaa aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg c aaa aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg c -aaa


def test_create_builtin_name():
    if CGROUP2_STATUS == Cgroupv2Status.UNAVAILABLE:
        pytest.xfail("requires cgroup v2")

    groups1 = set(list_groups())
    tcg create
    tcg c
    groups2 = set(list_groups())
    assert len(groups2 - groups1) == 2


def test_create_and_destroy():
    if CGROUP2_STATUS == Cgroupv2Status.UNAVAILABLE:
        pytest.xfail("requires cgroup v2")

    name1 = random_string(10)
    name2 = random_string(10)

    q1 = multiprocessing.Queue()
    q2 = multiprocessing.Queue()

    def f(q1, q2, name1, name2):
        tcg create @(name1)

        q1.put("sync point 1")
        assert q2.get() == "sync point 1"

        tcg c @(name2)

        q1.put("sync point 2")
        assert q2.get() == "sync point 2"

    p = multiprocessing.Process(target=f, args=(q1, q2, name1, name2))
    p.start()

    assert q1.get() == "sync point 1"
    groups1 = list_groups()
    assert name1 in groups1
    assert name2 not in groups1
    assert str(p.pid) in list_processes(name1)

    q2.put("sync point 1")
    assert q1.get() == "sync point 2"

    # At this point, a new cgroup name2 is created, and process `p`
    # are moved from name1 to name2. But name1 is not empty. It contains
    # one process which is the child process of second tcg call waiting
    # for cleaning up name2. This means, name1 can only be garbage collected
    # after name2 is garbage collected.
    groups2 = list_groups()
    assert name1 in groups2
    assert name2 in groups2
    assert str(p.pid) not in list_processes(name1)
    assert str(p.pid) in list_processes(name2)
    assert p.is_alive()

    q2.put("sync point 2")
    sleep 0.1
    groups3 = list_groups()
    assert name1 not in groups3
    assert name2 not in groups3


def test_list_illegal():
    with pytest.raises(subprocess.CalledProcessError):
        tcg l aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg ls aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg list aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg l cgs aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg l cgroups aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg ls cgs aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg ls cgroups aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg list cgs aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg list cgroups aaa


def test_list_empty():
    if CGROUP2_STATUS != Cgroupv2Status.UNAVAILABLE:
        pytest.skip("hard to guarantee empty in this context")
    assert $(tcg ls).strip() == ''


def test_list():
    if CGROUP2_STATUS == Cgroupv2Status.UNAVAILABLE:
        pytest.xfail("requires cgroup v2")

    name1 = random_string(10)
    name2 = random_string(10)
    tcg create @(name1)
    tcg create @(name2)
    groups = [
        set(list_groups()),
        {x.split()[0] for x in $(tcg list).strip().split('\n')},
        {x.split()[0] for x in $(tcg ls).strip().split('\n')},
        {x.split()[0] for x in $(tcg l).strip().split('\n')},
        {x for x in $(tcg list cgroups).strip().split('\n')},
        {x for x in $(tcg list cgs).strip().split('\n')},
        {x for x in $(tcg ls cgroups).strip().split('\n')},
        {x for x in $(tcg ls cgs).strip().split('\n')},
        {x for x in $(tcg l cgroups).strip().split('\n')},
        {x for x in $(tcg l cgs).strip().split('\n')},
    ]

    for g in groups:
        assert g == groups[0]
    assert name1 in groups[0]
    assert name2 in groups[0]


def test_list_procs():
    if CGROUP2_STATUS == Cgroupv2Status.UNAVAILABLE:
        pytest.skip("requires cgroup v2")

    name = random_string(10)

    q = multiprocessing.Queue()

    def f(q, name):
        tcg c @(name)
        q.put(None)
        sleep infinity

    p = multiprocessing.Process(target=f, args=(q, name))
    p.start()
    assert q.get() == None

    tcg_list = $(tcg ls).strip().split('\n')
    p.kill()

    procs_list = ""
    for l in tcg_list:
        if name in l:
            procs_list = l
            break

    assert 'python' in procs_list
    assert 'sleep' in procs_list


def test_freeze_unfreeze_illegal():
    non_existing_name = random_string(10)
    with pytest.raises(subprocess.CalledProcessError):
        tcg freeze @(non_existing_name)
    with pytest.raises(subprocess.CalledProcessError):
        tcg f @(non_existing_name)
    with pytest.raises(subprocess.CalledProcessError):
        tcg unfreeze @(non_existing_name)
    with pytest.raises(subprocess.CalledProcessError):
        tcg uf @(non_existing_name)

    if CGROUP2_STATUS == Cgroupv2Status.UNAVAILABLE:
        return

    name = random_string(10)
    tcg create @(name)
    with pytest.raises(subprocess.CalledProcessError):
        tcg freeze @(name) aaa


def test_freeze_unfreeze():
    if CGROUP2_STATUS == Cgroupv2Status.UNAVAILABLE:
        pytest.skip("requires cgroup v2")

    name = random_string(10)

    q1 = multiprocessing.Queue()
    q2 = multiprocessing.Queue()

    def echo(q1, q2, name):
        tcg create @(name)

        q1.put("start")

        while (v := q2.get()) != "end":
            q1.put(v)

    p = multiprocessing.Process(target=echo, args=(q1, q2, name))
    p.start()

    assert q1.get() == "start"

    q2.put(1)
    q2.put(2)
    q2.put(3)
    assert q1.get() == 1
    assert q1.get() == 2
    assert q1.get() == 3

    tcg freeze @(name)

    q2.put(4)
    q2.put(5)
    q2.put(6)

    with pytest.raises(queue.Empty):
        q1.get(timeout=0.1)

    tcg unfreeze @(name)

    assert q1.get() == 4
    assert q1.get() == 5
    assert q1.get() == 6

    tcg f @(name)

    q2.put(7)
    q2.put(8)
    q2.put(9)

    with pytest.raises(queue.Empty):
        q1.get(timeout=0.1)

    tcg uf @(name)

    assert q1.get() == 7
    assert q1.get() == 8
    assert q1.get() == 9

    q2.put("end")


def test_cpu_weight():
    if CGROUP2_STATUS != Cgroupv2Status.PURE:
        pytest.skip("requires cgroup v2")

    name1 = random_string(10)
    name2 = random_string(10)

    q1 = multiprocessing.Queue()
    q2 = multiprocessing.Queue()
    q3 = multiprocessing.Queue()
    q4 = multiprocessing.Queue()

    def f(q1, q2, name):
        tcg c @(name)
        q2.put("created")

        def compute():
            for i in range(100):
                hash((0,) * 10000)

        assert q1.get() == "start"
        compute()
        start = timeit.default_timer()
        compute()
        end = timeit.default_timer()
        q2.put(end - start)
        while True:
            compute()

    p1 = multiprocessing.Process(target=f, args=(q1, q2, name1))
    p1.start()
    p2 = multiprocessing.Process(target=f, args=(q3, q4, name2))
    p2.start()

    assert q2.get() == "created"
    assert q4.get() == "created"

    os.sched_setaffinity(p1.pid, {0})
    os.sched_setaffinity(p2.pid, {0})

    tcg set @(name1) cpu.weight 1
    tcg set @(name2) cpu.weight 20

    q1.put("start")
    q3.put("start")

    time_ratio = q2.get() / q4.get()
    p1.kill()
    p2.kill()

    assert time_ratio > 10


def test_self():
    if CGROUP2_STATUS == Cgroupv2Status.UNAVAILABLE:
        pytest.xfail("requires cgroup v2")

    name1 = random_string(10)
    name2 = random_string(10)

    tcg create @(name1)
    assert $(tcg self).strip() == name1

    tcg c @(name2)
    assert $(tcg sf).strip() == name2


def test_show():
    if CGROUP2_STATUS != Cgroupv2Status.PURE:
        pytest.xfail("requires cgroup v2")

    name = random_string(10)
    tcg create @(name)
    tcg set @(name) cpu.weight 1234
    assert $(tcg show @(name) cpu.weight).strip() == "1234"


def test_xontrib():
    pytest.skip("flaky")
    if CGROUP2_STATUS == Cgroupv2Status.UNAVAILABLE:
        pytest.xfail("requires cgroup v2")

    install_xontrib()
    xontrib load tcg
    assert $(tcg self).strip() == $TERMINAL_CGROUP


def test_tab_complete_command():
    install_xontrib()
    $AUTO_INIT_TCG = False
    from xontrib.tcg import tcg_tab_complete

    a = sorted($(tcg tab-complete '').strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg ')))
    assert a == b
    assert a == sorted(list(known_commands.keys()))

    a = sorted($(tcg tab-complete fr).strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg fr')))
    assert a == b
    assert a == ['freeze']

    a = sorted($(tcg tab-complete uf).strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg uf')))
    assert a == b
    assert a == ['uf']

    a = $(tcg tab-complete 'aaa').strip()
    b = tcg_tab_complete('tcg aaa')
    assert b == set()
    assert a == ''

    with pytest.raises(subprocess.CalledProcessError):
        tcg tab-complete


def test_tab_complete_argument():
    install_xontrib()
    $AUTO_INIT_TCG = False
    from xontrib.tcg import tcg_tab_complete

    a = $(tcg tab-complete l aaa).strip()
    b = tcg_tab_complete('tcg l aaa')
    assert b == set()
    assert a == ""

    a = sorted($(tcg tab-complete l '').strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg l ')))
    assert a == b
    assert a == ['cgroups']

    a = sorted($(tcg tab-complete ls 'c').strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg ls c')))
    assert a == b
    assert a == ['cgroups']

    a = sorted($(tcg tab-complete ls 'cg').strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg ls cg')))
    assert a == b
    assert a == ['cgroups']

    a = sorted($(tcg tab-complete l cgr).strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg l cgr')))
    assert a == b
    assert a == ['cgroups']

    a = sorted($(tcg tab-complete l cgs).strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg l cgs')))
    assert a == b
    assert a == ['cgs']

    a = sorted($(tcg tab-complete help '').strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg help ')))
    assert a == b
    assert a == sorted(list(known_commands.keys()))

    a = sorted($(tcg tab-complete help 'c').strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg c')))
    assert a == b
    assert a == ['create']

    a = sorted($(tcg tab-complete help 'uf').strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg uf')))
    assert a == b
    assert a == ['uf']

    a = sorted($(tcg tab-complete help 's').strip().split('\n'))
    b = sorted(list(tcg_tab_complete('tcg s')))
    assert a == b
    assert a == ['self', 'set', 'show']

def test_tab_complete_existing_cgroups():
    if CGROUP2_STATUS == Cgroupv2Status.UNAVAILABLE:
        pytest.xfail("requires cgroup v2")

    install_xontrib()
    $AUTO_INIT_TCG = False
    from xontrib.tcg import tcg_tab_complete

    name1 = 'n1_' + random_string(10)
    name2 = 'n2_' + random_string(10)

    tcg create @(name1)
    tcg c @(name2)

    a = set($(tcg tab-complete f '').strip().split('\n'))
    b = tcg_tab_complete('tcg f ')
    assert a == b
    assert name1 in a
    assert name2 in a

    a = set($(tcg tab-complete freeze 'n1_').strip().split('\n'))
    b = tcg_tab_complete('tcg freeze n1_')
    assert a == b
    assert name1 in a
    assert name2 not in a

    a = set($(tcg tab-complete uf '').strip().split('\n'))
    b = tcg_tab_complete('tcg uf ')
    assert a == b
    assert name1 in a
    assert name2 in a

    a = set($(tcg tab-complete set '').strip().split('\n'))
    b = tcg_tab_complete('tcg set ')
    assert a == b
    assert name1 in a
    assert name2 in a

    a = set($(tcg tab-complete show '').strip().split('\n'))
    b = tcg_tab_complete('tcg show ')
    assert a == b
    assert name1 in a
    assert name2 in a

def test_tcg_version():
    assert $(tcg version) == $(tcg v)
    v = $(tcg v)
    assert "Version:" in v
    assert "Git commit:" in v
    assert "Build date:" in v
    assert "C++ compiler:" in v
    assert "CMake version:" in v
