# Repository Guidelines

## Project Structure & Module Organization

This repository is the ICS2025 Programming Assignment workspace. The top level
contains the course launcher and shared settings:

- `docs/` is the local, static ICS2025 guide. Start at `docs/index.html`; the
  PA milestones are `PA0.html` through `PA4.html`.
- `init.sh` clones course components on demand.
- `Makefile` defines shared student metadata and the submission entry point.

After initialization, primary implementation work lives in the matching
component directory: `nemu/`, `abstract-machine/`, `nanos-lite/`, or
`navy-apps/`. Keep changes inside the component and stage required by the
guide; do not invent extra features or rearrange the course layout.

## Build, Test, and Development Commands

Initialize only the component required by the current PA, for example:

```bash
bash init.sh nemu
cd nemu && make menuconfig && make
cd nemu && make run
```

Other supported initializers are `abstract-machine`, `am-kernels`,
`nanos-lite`, and `navy-apps`. Each component supplies its own Makefile, run
targets, and tests; follow the exact command and configuration specified by
the relevant page in `docs/`. `make submit` at the repository root invokes the
course submission script—do not run it casually.

## Coding Style & Naming Conventions

The implementation is predominantly C and Make. Match the style of the file
being edited: use existing indentation, brace placement, comments, and naming.
Prefer small focused functions and course-provided APIs. Do not reformat
unrelated files. New test helpers should use the names and locations requested
by the current PA document.

## Testing Guidelines

There is no repository-wide test runner before components are initialized.
Build and run the designated PA tests from the relevant component, then record
the exact command and observable result in the commit message or report when
appropriate. Treat successful compilation alone as insufficient when the guide
names functional tests.

## Commits and Changes

Use concise, imperative, task-focused messages, e.g. `implement expression
evaluation` or `fix watchpoint deletion`. Keep each commit scoped to one
logical course task. The historical course tracing macro is disabled for this
non-course workspace; do not re-enable automated commits or add automatic
pushes. Before committing, review `git diff --check` and `git status`.

## Source of Truth

`docs/` is authoritative. Read the current PA section before coding and follow
its required interfaces, tests, and constraints exactly. If a required linked
reference cannot be accessed, report the missing URL or document instead of
guessing its contents.
