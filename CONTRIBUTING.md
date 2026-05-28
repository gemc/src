# Contributing to GEMC

Contributions to GEMC are welcome. Useful contributions include source code, Python API improvements, documentation, examples, tests, bug reports, build-system fixes, and reproducible simulation problems.

Keep changes focused and reviewable. A small pull request with clear motivation is easier to review and safer to merge than a broad rewrite.

## Before You Start

Open an issue before working on large changes, new features, build-system changes, public APIs, file formats, or behavior that affects examples and documentation. Small typo fixes, narrow documentation edits, and obvious bug fixes can go directly to a pull request.

For security vulnerabilities, do not open a public issue. Follow [`SECURITY.md`](SECURITY.md).

## Development Setup

Fork `gemc/src` on GitHub, then clone your fork:

```shell
git clone https://github.com/<your-username>/src.git gemc-src
cd gemc-src
```

## Build and Test

Use the project Meson build. The normal local command for building and installing the current tree is:

```shell
meson install -C build
```

List available tests:

```shell
meson test -C build --list
```

Run the relevant test with logs:

```shell
meson test -C build -v --print-errorlogs <testname>
```

When a change touches shared code, run the smallest useful set of related tests plus any examples that exercise the behavior. If a test cannot be run locally, say why in the pull request.

## Contribution Guidelines

- Match the style of the surrounding C++ or Python code.
- Prefer clear, local fixes over broad refactors.
- Add or update tests for behavior changes.
- Update README files, examples, and website documentation when user-facing behavior changes.
- Keep generated files, build output, caches, and local IDE files out of commits.
- Do not mix unrelated cleanup with feature or bug-fix work.
- For geometry or visualization changes, include screenshots or links to generated images when that helps review.

## Commit Messages

Use short, imperative commit summaries:

```text
Fix flux digitization timestamp output
```

When the reasoning is not obvious, add a body explaining what changed and why. If the pull request closes an issue, include `Closes #123` in the pull request description.

## Pull Requests

Open a pull request from your fork to `gemc/src`.

Before requesting review, check that:

- The title and description explain the change.
- Related issues are linked.
- Relevant tests or examples were run and listed.
- Documentation and examples were updated if needed.
- The pull request is focused on one topic.

Reviews may ask for changes to improve correctness, maintainability, performance, documentation, or test coverage.

## Communication

- General questions: open an issue.
- Security vulnerabilities: email **ungaro@jlab.org** instead of opening a public issue.
- Conduct concerns: email **ungaro@jlab.org** and see [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md).

## Licensing and Citation

By contributing, you agree that your contributions are licensed under the GEMC Software License v1.0. See [`LICENSE.md`](LICENSE.md).

If you use GEMC in scientific work, cite:

> M. Ungaro, "Geant4 Monte-Carlo (GEMC) A database-driven simulation program," EPJ Web of Conferences 295, 05005 (2024). https://doi.org/10.1051/epjconf/202429505005
