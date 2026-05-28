# GEMC Security Policy

**Contact:** ungaro@jlab.org

Do not open public issues for security vulnerabilities.

## Supported Versions

Security fixes are handled for the active development branch and the most recent public release when applicable.

| Version | Supported |
| --- | --- |
| `main` / `dev` | Yes |
| Latest release | Yes, when applicable |
| Older releases | No |

## Reporting a Vulnerability

Email **ungaro@jlab.org** with as much detail as you can safely share:

- A short summary of the issue and why it matters.
- Steps to reproduce, preferably with a minimal proof of concept.
- Affected repository, version, commit, container image, or installation method.
- Operating system, compiler, Python version, Geant4 version, and relevant build options.
- Whether the issue is already public or known elsewhere.

If GitHub private vulnerability reporting is enabled for the repository, you may also use **Security -> Report a vulnerability**.

## What to Expect

After a report is received, maintainers will:

- Acknowledge the report as soon as practical.
- Triage the issue and may ask for additional information.
- Work on a fix, mitigation, or coordinated disclosure plan.
- Credit the reporter if desired and appropriate.

Please avoid public disclosure until maintainers have had a reasonable opportunity to investigate and address the issue.

## Scope

In scope:

- GEMC source code in official GEMC repositories.
- The `pygemc` Python API and command-line tools.
- Official GEMC container images and build scripts.
- Documentation or examples that could lead users to unsafe behavior.

Out of scope:

- Vulnerabilities only in third-party dependencies. Report those upstream; you may copy GEMC maintainers if GEMC users are affected.
- Local misconfiguration unrelated to GEMC.
- Denial-of-service reports based only on intentionally extreme local resource usage.

Thank you for helping keep GEMC users safe.
