import sys, os

POS_OUTPUT_DIRS = os.environ['POS_OUTPUT_DIRS']

def run_from_argv():
    run = None
    for x in sys.argv:
        try:
            run = int(x)
            break
        except ValueError:
            pass
    if run is None:
        raise ValueError('no number in argv')
    return run

def run_dir(run):
    run_thousand = run / 1000 * 1000
    return os.path.join(POS_OUTPUT_DIRS, 'Run_%i' % run_thousand, 'Run_%i' % run)

def run_dir_from_argv():
    return run_dir(run_from_argv())

def run_fn(run, fn):
    return os.path.join(run_dir(run), fn)

def run_fn_from_argv(fn):
    return os.path.join(run_dir_from_argv(), fn)
