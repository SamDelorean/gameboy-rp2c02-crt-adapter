# Licensing plan

This repository intentionally separates **hardware**, **firmware/software**, and **documentation** because they are different kinds of works.

## Proposed licenses

These are the current project recommendations and are **not yet declared final**:

- `hardware/` — **CERN-OHL-W-2.0**
- `firmware/` and software utilities — **MIT License**
- `docs/`, diagrams, original explanatory material — **CC BY-SA 4.0**

## Why a mixed model?

The intended behavior is:

- hardware may be fabricated, modified, sold, and incorporated into larger systems;
- direct modifications to the open hardware design should remain available under a reciprocal open-hardware license;
- firmware should be easy to reuse in unrelated technical projects;
- documentation should remain shareable and adaptable with attribution and share-alike treatment.

## Finalization rule

Before the first reproducible hardware release, the repository should:

1. add the complete license texts,
2. place clear per-directory notices,
3. add SPDX identifiers where practical,
4. define how mixed files (for example, generated diagrams embedded in documentation) are treated,
5. update the root README with the final license statement.

Until then, do not assume that absence of a final license grants unrestricted reuse.
