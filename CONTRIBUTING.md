# Contributing to GEMC

We welcome contributions of code, documentation, tests, examples, and bug reports. This guide explains how to propose changes via GitHub.

> **Quickstart**
> 1. Open an issue to discuss your idea or bug.
> 2. Fork the repo and clone your fork.
> 3. Create a feature branch.
> 4. Commit changes with clear messages and add tests/docs.
> 5. Keep your fork in sync with `upstream`.
> 6. Open a pull request (PR) from your branch.

---

## 1) Before you start: open an issue
Have a feature in mind? Open an issue describing the problem, proposed solution, and any alternatives. For obvious, small bug fixes, opening a PR directly is fine—but filing an issue first is still appreciated.

If you’re not sure where to help, ask in an issue—documentation improvements are always welcome.

## 2) Fork and clone

Create your fork on GitHub, then:

```bash
git clone https://github.com/<your-username>/src.git gemc
cd gemc
git remote add upstream https://github.com/gemc/src.git
git remote -v
```

## 3) Create a branch

Use a descriptive branch name:

```bash
# Replace "main" with "master" if your fork uses that as the default
git switch -c feature/my-awesome-idea
# or: git checkout -b feature/my-awesome-idea
```

## 4) Keep your fork in sync
Update your local default branch and your feature branch regularly:

```bash
git fetch upstream
git switch main
git merge upstream/main    # or: git rebase upstream/main
git push origin main

# Update your feature branch
git switch feature/my-awesome-idea
git rebase main            # keeps history clean; use merge if you prefer
```

## 5) Code standards & tests

- **Style**: Match the existing project style. If a .clang-format/formatter is present, use it before committing.
- **C++/Python APIs**: Keep naming, headers, and error handling consistent with the code around your changes.
- **Tests**: Add or update tests for any behavior change.
- **Docs**: Update README/installation/examples and any relevant docs when behavior or options change.
- **Small, focused PR**s: Prefer multiple small PRs over one very large PR.

## 6) Commit messages

Write clear, imperative messages:

```bash

Short summary (max ~72 chars)

Longer explanation of what and why,
including any breaking changes or migration notes.
```

If your PR closes an issue, end the description with: Closes #<issue-number>.

## 7) Open a Pull Request
From your fork/branch, open a PR against gemc/source:

**Checklist**

- Clear title and concise description
- Link to related issues; include Closes #123 when appropriate
- Include screenshots for UI/visual changes
- Tests pass locally; CI is green
- Docs/examples updated as needed

Reviews in this project are thorough. Feedback is offered in good spirit to improve clarity, performance, and maintainability.

## 8) Communication & support

- Questions / help: Open an issue.
- Security vulnerabilities: Do not open a public issue. Email gemc@jlab.org.
- Conduct: All interactions are governed by our Code of Conduct.

## 9) Licensing of contributions
By contributing, you agree that your contributions are licensed under the GEMC Software License v1.0 and 
that publications using GEMC should cite:

M. Ungaro, “Geant4 Monte-Carlo (GEMC) A database-driven simulation program,” EPJ Web of Conferences 295, 05005 (2024), https://doi.org/10.1051/epjconf/202429505005