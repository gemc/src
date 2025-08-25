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
