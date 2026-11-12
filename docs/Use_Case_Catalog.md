# Use Case Catalog — Simple Shell

This document identifies and names the actors, and groups the use cases by actor. It is the
authoritative definition of the use case identifiers used by
`4-Architectural-Design/Use_Case_Diagram.puml` and `RTM.md`.

## 1. Actors

**Number of actors: 2**

| Actor ID | Actor | Classification | Description |
|---|---|---|---|
| A1 | Interactive User | Primary actor | The end user of the shell. Enters command lines at the prompt, receives output and diagnostics, and terminates the session. This is the customer for whom the use cases in Section 2 are defined. |
| A2 | Linux Kernel / POSIX Operating System | Supporting actor (external system) | Supplies the process, file-descriptor, pipe and directory services the shell depends on. The shell does not implement these; it consumes them through the POSIX API. |

Notes on the actor count:

- A1 is the only human actor. No administrator, operator or GUI actor exists in the initial scope
  because the shell has no configuration surface, no service mode, and no remote access.
- A2 is modelled as an actor rather than an internal component because it is outside the system
  boundary and the system has an explicit, testable dependency on it.
- Actors are **not** created for external programs run by the user. Those are child processes created
  by the shell, not entities that negotiate goals with the shell.

## 2. Use cases grouped by actor

### 2.1 Group A — Interactive User (A1)

All ten use cases below are ultimately triggered by input from A1. They are grouped by the stage of
the command lifecycle that A1's input drives, which is how the system responds to that actor.

| UC ID | Use case | Trigger | Included / extends | Related FR |
|---|---|---|---|---|
| UC-01 | Enter Command Line | User types a line at the prompt and presses Enter | includes UC-02 | FR-01 |
| UC-02 | Parse Input & Tokens | Always performed for any input line | included by UC-01 | FR-04 |
| UC-03 | Execute Command | A parsed pipeline is dispatched for execution | extended by UC-06, UC-07, UC-08, UC-09 | FR-02, FR-09 |
| UC-04 | Execute External Program | Parsed command names a program that is not a built-in | specialises UC-03 | FR-02 |
| UC-05 | Execute Built-in Command | Parsed command names `cd`, `pwd`, `echo`, `help`, `exit` | specialises UC-03 | FR-03 |
| UC-06 | Redirect I/O (`<`, `>`, `>>`) | Command line contains an input or output redirection operator | extends UC-03 | FR-05 |
| UC-07 | Execute Pipeline (`|`) | Command line contains two or more commands joined by `\|` | extends UC-03 | FR-06 |
| UC-08 | Run in Background (`&`) | Command line ends with `&` | extends UC-03 | FR-07 |
| UC-09 | Report Diagnostics & Syntax Errors | Malformed syntax, failed lookup, or failed system call | extends UC-03, extends UC-02 | FR-08 |
| UC-10 | Exit Shell Session | User types `exit`, or sends EOF (Ctrl-D) | — | FR-10 |

### 2.2 Group B — Linux Kernel / POSIX Operating System (A2)

A2 does not have user-visible use cases of its own. It is associated with the use cases in Group A
because it supplies the services those use cases depend on. Each association below is an
implementation-backed dependency with a corresponding function in the source.

| Associated use case | Service required from A2 | Source |
|---|---|---|
| UC-04 Execute External Program | `fork`, `execvp` | `src/executor.c` |
| UC-04, UC-05 Execute Command | `waitpid` (blocking, foreground) | `src/executor.c` |
| UC-08 Run in Background | `waitpid` with `WNOHANG` (periodic reap) | `src/executor.c` |
| UC-06 Redirect I/O | `open`, `dup2` | `src/executor.c` |
| UC-07 Execute Pipeline | `pipe`, `dup2` | `src/executor.c` |
| UC-05 Execute Built-in Command | `chdir`, `getcwd` | `src/builtins.c` |

## 3. Relationship rules used in the diagram

| Relationship | Meaning | Applied to |
|---|---|---|
| Association (solid arrow) | A1 initiates the use case. | UC-01, UC-10 |
| «include» (dotted arrow) | The target use case is **always** executed as part of the source. | UC-01 → UC-02 |
| Generalisation (solid arrow, hollow triangle) | A specialised use case is a form of its parent. | UC-04, UC-05 → UC-03 |
| «extend» (dotted arrow) | The source is an **optional** behaviour added to a use case that is already complete without it. | UC-06, UC-07, UC-08, UC-09 → UC-03 |

Two modelling rules are worth stating because they are easy to get wrong:

1. Redirection, pipelines and background execution are **modifiers**, so they «extend» the
   execution use case rather than being separate top-level use cases. They apply to both external
   programs and built-ins, which is why they attach to the shared parent UC-03 rather than to
   UC-04 alone.
2. A use case may extend only one base use case in UML. Attaching the modifiers to both UC-04 and
   UC-05 directly would be invalid, so UC-03 is introduced as their common parent.

## 4. Traceability

- Diagram: `4-Architectural-Design/Use_Case_Diagram.puml` (UML; rendered to
  `4-Architectural-Design/Use_Case_Diagram.svg`/`.png` and `docs/Use_Case_Diagram.png`)
- Requirements: `docs/SRS.md`
- Requirement-to-use-case-to-test mapping: `docs/RTM.md`
