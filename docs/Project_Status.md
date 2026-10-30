# Project Status

**Project:** Simple Shell
**Team:** H1
**Primary language:** C
**Target:** Linux

## Phase 1 (Requirements stage) status

| Deliverable | File | Status |
|---|---|---|
| Problem statement / feasibility | `Problem_Statement.md`, `Feasibility_Study.md` | Prepared |
| SRS and requirement categorization | `SRS.md` | Prepared |
| RTM | `RTM.md` | Prepared |
| Actors identified and named | `Use_Case_Catalog.md` §1 | Prepared — 2 actors |
| Use cases grouped by actor | `Use_Case_Catalog.md` §2 | Prepared |
| Use case diagram | `Use_Case_Diagram.mmd` / `.png` | Prepared |
| Validation specification | `Validation_Specification.md` | Prepared, including penetration test plan (planning only) |

Deliverable-to-file mapping is in `docs/Phase_1_Checklist.md` Section 1.

## Implementation baseline

- Modular C implementation in `src/` with headers in `include/`.
- Automated parser, direct executor, and shell integration tests in `tests/`.
- GitHub Actions CI workflow running `make`, `make test`, and `make asan-test` with timeouts on every push and pull request (Refs Issue #8).
- Jira backlog seed in `docs/Jira_Backlog.md`.

## Validation state

- Standard and sanitizer validation evidence, suite totals, and limitations are recorded in [Test_Report.md](Test_Report.md). The operator and two-stage pipeline evidence applies to the branch under review in [PR #27](https://github.com/SE-H1-SimpleShell/MiniProject_H1_SimpleShell/pull/27).
- Automated coverage includes malformed syntax with no execution/file side effects, pipeline boundaries, built-in stages, both-child waiting, rightmost status, and repeated pipeline descriptor checks.

## Phase 2 (Agile Development stage) status

| Deliverable | Description / Location | Status | ETA |
|---|---|---|---|
| Backlog creation in GitHub | Issues #2 to #19 mapped to FR/NFRs | Published | 12th Oct 2026 |
| Story Points assignment | Fibonacci points assigned across all stories (66 pts total) | Completed | 12th Oct 2026 |
| Team story assignments | Distributed equally across 4 team members | Completed | 12th Oct 2026 |
| Two Sprint plans | Sprint 1 (Oct 19–23) and Sprint 2 (Oct 26–30) documented in `docs/Sprint_Backlog.md` | Completed | 12th Oct 2026 |
| Sprint dry run | Practice PRs and CI validation | Historical plan; completion not separately recorded | 12th–16th Oct 2026 |
| Sprint 1 execution | Working foreground shell & [demo evidence](demos/README.md) | Done; all Sprint 1 issues closed | 19th–23rd Oct 2026 |
| Sprint 2 execution | Advanced features, 2-min demo video & freeze | In progress; #10 done, #11/#12 in review, #13-#18 open | 26th–30th Oct 2026 |

## Open items

1. Complete teammate review and merge of [PR #27](https://github.com/SE-H1-SimpleShell/MiniProject_H1_SimpleShell/pull/27) for #11/#12.
2. Complete the remaining Sprint 2 stories (#13-#18), including resource audit, full regression/traceability review, final documentation, and the final demo/release.
3. Capture the genuine screenshots still listed in the [GitHub evidence index](../5-GitHub-Jira-Evidence/Evidence_Index.md) and [demo evidence index](../9-Demo/Demo_Evidence_Index.md).

The [Sprint backlog](Sprint_Backlog.md) records the status snapshot; subsequent changes are tracked on the live board.
