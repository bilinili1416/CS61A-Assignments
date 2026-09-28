# Copilot Instructions

## Repository shape

This is a self-contained archive of UC Berkeley CS61A coursework, not one
installable application. Work is organized into:

- `HW/`: Python and SQL homework, with Scheme homework in later units.
- `Lab/`: Python, Scheme, and SQL labs.
- `Project/`: the larger Python projects `ants`, `cats`, `hog`, and `scheme`.

Each assignment/project directory is intentionally isolated. It normally has
its own `ok` runner, tests/configuration, and copies of course support modules
such as `ucb.py`; do not assume imports or test data resolve from the
repository root. Run commands from the assignment directory being changed.

The projects combine student-facing logic with local drivers and UIs:
`ants/gui.py`, `cats/cats_gui.py`, and `hog/hog_gui.py` expose web interfaces,
while `hog/hog_ui.py` and `scheme/scheme.py` provide command-line programs.
The web frontend files under `gui_files/` and project `editor/` directories
are checked-in static artifacts; there is no root frontend build pipeline.

## Test and development commands

There is no repository-wide build, package manager, or configured lint command.
Python 3 is the expected runtime. From the directory containing the target
assignment's `ok` script:

```bash
# Run the complete assignment test suite without network access
python ok --local

# Run one question/function (question names are defined by that assignment)
python ok -q <question_name> --local

# Run one question verbosely
python ok -q <question_name> -v --local

# Open an interactive debugger after a failed question
python ok -q <question_name> -i --local
```

The README uses the equivalent `python3 ok ...` spelling. On Windows, use
`python` if that is the command registered for Python 3. The same commands
apply to Python, Scheme, and SQL assignments; for SQL, the test data and
`sqlite_shell.py` live beside the assignment's `.sql` file. Use `--local` for
offline validation and to avoid backup/submission network activity.

Useful project entry points, also run from their project directories, are:

```bash
python hog_ui.py
python scheme.py
python cats_gui.py
python gui.py
```

GUI servers may open a browser and require an available local port. Prefer the
`ok` tests for ordinary changes.

## Architecture and conventions

- Assignment files are teaching skeletons. Implement only the regions marked
  `BEGIN PROBLEM`/`END PROBLEM` (or `REPLACE THIS LINE` in SQL) unless the
  task explicitly requires changing scaffolding or support code.
- Tests are driven by `.ok` configuration files and numbered question files
  under `tests/` or `tests.scm`/`mytests.rst`. Question names, not pytest
  function discovery, are the stable test interface.
- Course helpers are part of the local runtime: `ucb.main` launches decorated
  scripts, `interact`/`trace` support debugging, and project-specific helper
  modules are imported by filename. Preserve these conventions instead of
  introducing a new framework.
- Python assignments commonly combine doctests in the solution file with
  hidden/visible `ok` tests. Keep documented function signatures, assertions,
  return types, and side effects intact because the grader calls functions
  directly.
- Scheme work is split across the evaluator/form machinery
  (`scheme_eval_apply.py`, `scheme_forms.py`, `scheme_classes.py`), reader
  modules under `scheme_reader/`, and builtins. Changes to evaluation or
  environment behavior can affect multiple problems.
- SQL exercises build tables in the supplied `.sql` script and validate named
  result tables with the bundled SQLite shell/`ok` tests. Preserve the schema
  and required column names; do not replace the exercise with a different
  database setup.
- Project simulations are callback/class based: Hog strategies and update
  functions are passed into `play`, Ants behavior is distributed across
  `Place`, `Insect`, `Ant`, `Bee`, and `GameState`, and Cats separates core
  text algorithms from `cats_gui.py`. Keep interfaces compatible with the
  surrounding driver and GUI code.
- Several directories contain generated runtime files (`__pycache__`,
  `.ok_*`, local editor state) and bundled third-party/static assets. Do not
  modify or add those as part of a normal solution; make source changes in the
  relevant assignment directory.
