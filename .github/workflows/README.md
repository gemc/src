# GitHub Actions workflows

This directory contains the CI, deployment, release, documentation, and maintenance workflows for GEMC. This
file is the authoritative overview of the cross-repository `pygemc -> src -> clas12-systems` automation chain.

## End-to-end dependency chain

A direct GEMC change follows this path:

```text
src push to main
  -> Test
  -> Deploy
     -> Binary Tarballs
     -> Triggers clas12-systems test
        -> clas12-systems Test after gemc/src deploy
        -> clas12-systems Deploy
        -> clas12-systems Binary Tarballs
```

A pygemc-only change rebuilds every derived image without requiring commits in the downstream repositories:

```text
pygemc push to main
  -> Trigger GEMC compatibility test
  -> src Test after pygemc
     -> reusable jobs from src Test
     -> src Deploy
        -> src Binary Tarballs
        -> clas12-systems Test after gemc/src deploy
           -> clas12-systems Deploy
           -> clas12-systems Binary Tarballs
```

The pygemc test workflow and its GEMC dispatcher are separate push workflows; the dispatch does not wait for the
standalone pygemc test matrix.

## Deployment authorization

`Deploy` accepts only successful, same-repository runs on `main` with one of these workflow/event pairs:

- `Test` with a `push` event.
- `Test after pygemc` with a `workflow_dispatch` event.

The compatibility workflow can also be run from the Actions page. A successful manual run on `main` is
deployment-authorized and continues through the CLAS12 deployment chain.

## Build, test, and deployment workflows

- [`test.yml`](test.yml) — **Test**
  - Trigger: pull requests, merge queue runs, pushes to `main`, and `workflow_call`.
  - Effect: builds and tests the configured operating-system and architecture matrix.
  - Downstream: `Deploy` only for a successful push to `main`.
- [`test_after_pygemc.yml`](test_after_pygemc.yml) — **Test after pygemc**
  - Trigger: API or manual `workflow_dispatch`; the upstream dispatcher selects `main`.
  - Effect: calls the reusable jobs in `test.yml` against the current pygemc source consumed by GEMC.
  - Downstream: `Deploy` after success.
- [`deploy.yml`](deploy.yml) — **Deploy**
  - Trigger: completion of either approved Test workflow on `main`.
  - Effect: builds per-architecture images, publishes GHCR manifests and Docker Hub mirrors, and exports logs
    and GEMC binary tarball artifacts.
  - Downstream: `Binary Tarballs` and the CLAS12 compatibility-test dispatcher after completion.
- [`binary_tarballs.yml`](binary_tarballs.yml) — **Binary Tarballs**
  - Trigger: completion of `Deploy`.
  - Effect: after a successful same-repository deployment, attaches its Linux tarballs to the `dev` release
    and tests the published archives.
- [`sanitize.yml`](sanitize.yml) — **Sanitize**
  - Trigger: pull requests, merge queue runs, pushes to `main`, or manual dispatch.
  - Effect: runs the sanitizer matrix and uploads sanitizer logs.
- [`pr-docker-image.yml`](pr-docker-image.yml) — **PR Docker Image**
  - Trigger: pull request open, update, reopen, and close events.
  - Effect: publishes an AlmaLinux preview image for an open PR and deletes it when the PR closes.

## Release workflows

- [`release_tarballs.yml`](release_tarballs.yml) — **Release Tarballs**
  - Trigger: a published release or a manual dispatch naming a stable release tag.
  - Effect: builds Linux and macOS release archives and attaches them to the non-`dev` release.
- [`macos_tarball.yml`](macos_tarball.yml) — **macOS GEMC Binary Tarball**
  - Trigger: daily at 05:00 UTC or manual dispatch.
  - Effect: builds and tests GEMC natively on Apple Silicon and updates the selected release.
- [`dev_release.yml`](dev_release.yml) — **Nightly Dev Release**
  - Trigger: daily at 01:44 UTC or manual dispatch.
  - Effect: moves the `dev` tag and updates the development prerelease and generated notes.

## Documentation, security, and maintenance workflows

- [`doxygen.yml`](doxygen.yml) — **Doxygen**
  - Trigger: non-Markdown pushes to any branch or manual dispatch.
  - Effect: builds documentation and deploys GitHub Pages from `main`.
- [`codeql.yml`](codeql.yml) — **CodeQL Advanced**
  - Trigger: non-Markdown pushes and pull requests targeting `main`.
  - Effect: performs Actions, C/C++, and Python security analysis.
- [`cleanup.yml`](cleanup.yml) — **Cleanup old artifacts and GHCR images**
  - Trigger: daily at 03:00 UTC or manual dispatch, with a dry-run option.
  - Effect: removes expired artifacts, stale managed image tags, and unreferenced child manifests.
- [`rerun_failed_matrix.yml`](rerun_failed_matrix.yml) — **Retry Failed Matrix Jobs**
  - Trigger: completion of Test, Sanitize, Deploy, Binary Tarballs, or Release Tarballs.
  - Effect: on attempt 1, retries failed jobs only when the run failed or timed out and at least half of the
    counted matrix jobs succeeded.
- [`trigger_c12s_tests.yml`](trigger_c12s_tests.yml) — **Triggers clas12-systems test**
  - Trigger: completion of `Deploy`.
  - Effect: after a successful same-repository deployment, dispatches the dedicated CLAS12 compatibility
    workflow using `CLAS12_SYSTEMS_PAT`.

## Cross-repository contracts

Workflow filenames and displayed `name` values are interfaces, not cosmetic labels:

- `gemc/pygemc` dispatches `test_after_pygemc.yml`; renaming it requires changing
  `pygemc/.github/workflows/trigger_src_tests.yml`.
- `Deploy` matches the exact workflow names `Test` and `Test after pygemc`.
- `test_after_pygemc.yml` calls `test.yml` through `workflow_call`; keep the shared matrix in `test.yml`.
- `trigger_c12s_tests.yml` dispatches `gemc/clas12-systems` file `test_after_src.yml`.
- The CLAS12 Deploy workflow matches the exact name `Test after gemc/src deploy`.

The local checkout paths used by developers are sibling repositories, but Actions dispatches use repository and
workflow identifiers hosted by GitHub.

## Secrets, variables, and privileges

- `CLAS12_SYSTEMS_PAT` authorizes the cross-repository CLAS12 workflow dispatch.
- `DOCKERHUB_USERNAME` and `DOCKERHUB_TOKEN` enable Docker Hub login and mirroring.
- `DOCKERHUB_REPO` optionally overrides the default `gemc/gemc` Docker Hub target.
- `GITHUB_TOKEN` handles GHCR, artifacts, releases, tags, Pages, and same-repository Actions API operations.

Privileged `workflow_run` jobs must retain conclusion, same-repository, branch, workflow-name, and event checks.
Never allow an untrusted pull-request workflow to reach image, release, or cross-repository publication steps.

## Concurrency, retries, and skipped runs

Long-running workflows use concurrency groups with `cancel-in-progress: true`, so a newer run for the same ref or
source run can cancel older work.

GitHub creates a `workflow_run` workflow before evaluating its job-level `if`. Consequently,
`Retry Failed Matrix Jobs` appears as skipped after successful watched workflows. This is expected. Failed Deploy
runs can similarly create gated Binary Tarballs and CLAS12-dispatch runs. The normal and pygemc-triggered
successful deployment paths do not create skipped Deploy or Binary Tarballs runs.

## Safe workflow changes

When changing a cross-repository workflow filename or displayed `name`:

1. Add the new target workflow first.
2. Update every caller and `workflow_run.workflows` list.
3. Remove the old entry point only after the callers are deployed.
4. Keep privileged workflow guards and least-privilege permissions intact.
5. Validate YAML, line wrapping, and `git diff --check` before pushing.

For coordinated changes to this chain, publish `clas12-systems` before `src`, and `src` before `pygemc`.

Local companion guides:

- [CLAS12 workflow guide][clas12-workflows]
- [pygemc workflow guide][pygemc-workflows]

[clas12-workflows]: https://github.com/gemc/clas12-systems/blob/main/.github/workflows/README.md
[pygemc-workflows]: https://github.com/gemc/pygemc/blob/main/.github/workflows/README.md
