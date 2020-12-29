import string
import random
import os
import pytest
import multiprocessing
import subprocess
import queue
import timeit


uid = os.getuid()
ROOT = f'/sys/fs/cgroup/terminals/{uid}/'
CGROUP2_AVAILABLE = os.path.isfile('/sys/fs/cgroup/cgroup.procs')


def random_string(length):
    return ''.join(random.choices(string.ascii_uppercase + string.digits, k=length))


def list_groups():
    return [f for f in os.listdir(ROOT) if os.path.isdir(os.path.join(ROOT, f))]


def list_processes(group):
    with open(os.path.join(ROOT, group, 'cgroup.procs')) as f:
        return f.read()


def test_invalid_argument():
    with pytest.raises(subprocess.CalledProcessError):
        tcg
    with pytest.raises(subprocess.CalledProcessError):
        tcg aaa


def test_help():
    tcg help
    tcg h


def test_create_illegal():
    with pytest.raises(subprocess.CalledProcessError):
        tcg create aaa aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg c aaa aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg c -aaa


def test_create_builtin_name():
    if not CGROUP2_AVAILABLE:
        pytest.xfail("requires cgroup v2")

    groups1 = set(list_groups())
    tcg create
    tcg c
    groups2 = set(list_groups())
    assert len(groups2 - groups1) == 2


def test_create_and_destroy():
    if not CGROUP2_AVAILABLE:
        pytest.skip("requires cgroup v2")

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
        tcg ls aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg list aaa


def test_list_empty():
    if CGROUP2_AVAILABLE:
        pytest.skip("hard to guarantee empty in this context")
    assert $(tcg ls).strip() == ''


def test_list():
    if not CGROUP2_AVAILABLE:
        pytest.xfail("requires cgroup v2")

    name1 = random_string(10)
    name2 = random_string(10)
    tcg create @(name1)
    tcg create @(name2)
    groups1 = set(list_groups())
    groups2 = {x.split()[0] for x in $(tcg list).strip().split('\n')}
    groups3 = {x.split()[0] for x in $(tcg ls).strip().split('\n')}
    print(groups1)
    print(groups2)
    print(groups3)
    assert groups1 == groups2
    assert groups1 == groups3


def test_list_procs():
    if not CGROUP2_AVAILABLE:
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

    if not CGROUP2_AVAILABLE:
        return

    name = random_string(10)
    tcg create @(name)
    with pytest.raises(subprocess.CalledProcessError):
        tcg freeze @(name) aaa


def test_freeze_unfreeze():
    if not CGROUP2_AVAILABLE:
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
    if not CGROUP2_AVAILABLE:
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
    if not CGROUP2_AVAILABLE:
        pytest.xfail("requires cgroup v2")

    name1 = random_string(10)
    name2 = random_string(10)

    tcg create @(name1)
    assert $(tcg self).strip() == name1

    tcg c @(name2)
    assert $(tcg sf).strip() == name2
