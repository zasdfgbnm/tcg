import string
import random
import os
import pytest
import multiprocessing
import subprocess
import queue


uid = os.getuid()
ROOT = f'/sys/fs/cgroup/user.slice/user-{uid}.slice/user@{uid}.service/terminals.slice/'


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
    groups1 = set(list_groups())
    tcg create
    tcg c
    groups2 = set(list_groups())
    assert len(groups2 - groups1) == 2


def test_create_and_destroy():
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


def test_list():
    with pytest.raises(subprocess.CalledProcessError):
        tcg ls aaa
    with pytest.raises(subprocess.CalledProcessError):
        tcg list aaa

    name1 = random_string(10)
    name2 = random_string(10)
    tcg create @(name1)
    tcg create @(name2)
    groups1 = set(list_groups())
    groups2 = set($(tcg list).strip().split(', '))
    groups3 = set($(tcg ls).strip().split(', '))
    print(groups1)
    print(groups2)
    print(groups3)
    assert groups1 == groups2
    assert groups1 == groups3

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

    name = random_string(10)
    tcg create @(name)
    with pytest.raises(subprocess.CalledProcessError):
        tcg freeze @(name) aaa

def test_freeze_unfreeze():
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
