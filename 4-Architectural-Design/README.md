# 4 — Architectural Design

Contents:
- [Architecture](Architecture.md) — existing module design.
- [Architecture Selection and Justification](Architecture_Selection.md).
- [API and Component Interfaces](API_and_Component_Interfaces.md).
- **Architecture diagram (PlantUML):** [source](Architecture.puml) · [PNG](Architecture.png) · [SVG](Architecture.svg) — component and layered view with the POSIX services the shell depends on.
- [System Architecture](System_Architecture.svg).
- [Component Diagram](Component_Diagram.svg).
- [Sequence Diagram](Sequence_Diagram.svg).
- [Use Case Diagram](Use_Case_Diagram.png) — existing diagram reused from the requirements docs.

The SVGs are editable vector diagrams. The system has C module APIs; it does not expose a web API.

## Regenerating the PlantUML diagram

```bash
java -jar plantuml.jar -tpng 4-Architectural-Design/Architecture.puml
java -jar plantuml.jar -tsvg 4-Architectural-Design/Architecture.puml
```

The source uses `!pragma layout smetana`, so Graphviz is not required.
