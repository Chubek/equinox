# The Job of Agent

Your job is to fix how Equinox uses third_party libraries, stored in `third_party/` directory.

Special file of intrest is `include/equinox/libutils.h`. But other files in `include/equinox` and `src` also apply.

## Steps
1. Absorb libraries in `third_party`
2. Search `include/` and `src/` for incorrect usage
3. Fix.

If you face another unrelated bug, report the bug to `DISCOVERED_BUGS.md`.

---

# Token Cost Control Rules

This repository is large and agentic coding can become expensive quickly.

Use the cheapest model that can correctly solve the task.

Do not default to the strongest model.

---

## Model Selection Policy

Preferred order:

1. gpt-5.3-codex-spark
2. gpt-5.3-codex
3. claude-sonnet-4-6
4. claude-opus-4-6

Escalate only when necessary.

If a cheaper model can solve it safely, do not use a more expensive one.

---

## Use Cheap Models For

Always prefer low-cost models for:

- YAML editing
- JSON Schema work
- AGENTS.md updates
- fixes-report.yaml updates
- documentation generation
- HTML report generation
- simple test scaffolding
- grep guidance
- locating symbols
- formatting fixes
- deterministic refactors
- file organization
- non-semantic code cleanup
- obvious one-line bug fixes

Do not spend premium tokens on clerical work.

---

## Use Stronger Models For

Use stronger models only for:

- parser correctness
- compiler invariants
- runtime corruption
- type unification failures
- symbol poisoning
- recursion cycles
- graph corruption
- nondeterministic behavior
- root-cause debugging
- cross-subsystem invariant repair
- “fix the cause, not the symptom” work

Hard correctness problems justify stronger models.

Formatting does not.

---

## Context Window Discipline

Never paste large files unless required.

Bad:

- entire parser.py
- entire compiler.py
- full repository trees repeatedly

Good:

- minimal failing function
- exact traceback
- exact failing test
- exact relevant class
- exact invariant violation

Smaller context = lower cost + better fixes.

---

## One Error Per Session

Work from one `error-fixes.yaml` entry at a time.

Do not ask the model to solve 12 unrelated bugs in one request.

Use:

- one slug
- one root cause
- one fix
- one regression test

This improves quality and reduces wasted tokens.

---

## Do Not Re-Explain Stable Context

Avoid repeatedly sending:

- repository structure
- schema definitions
- AGENTS.md contents
- already-known subsystem descriptions

Agents should read existing files instead.

Do not pay twice for the same context.

---

## Prefer References Over Pasting

Instead of:

“Here is the full file again…”

Use:

“See `liblint/parser.py`, function parse_declaration()`”

Reference first.

Paste only the minimal required code.

---

## Stop Recursive Re-Debugging

If the same issue has already been analyzed:

- reuse the prior diagnosis
- continue from the prior fix attempt

Do not restart analysis from zero every session.

Repeated diagnosis is a major token sink.

---

## Mandatory Local Validation First

Before asking the model:

Run:

- pytest
- rg
- stack trace capture
- exact failure reproduction

Never ask the model vague questions like:

“something is broken”

Precise failures are cheaper than exploratory guessing.

---

## Reports Must Be Short

`fixes-report.yaml` entries should be concise.

Do not write essays.

Use:

- exact files changed
- exact tests added
- exact validation command
- exact summary

Short reports save tokens and improve future retrieval.

---

## Final Principle

Expensive models should solve difficult invariants.

Cheap models should handle everything else.

Most token waste comes from using a senior surgeon to staple paper.
