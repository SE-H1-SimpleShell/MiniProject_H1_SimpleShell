# 4 — Architectural Design

Contents:
- [Architecture](Architecture.md) — IEEE 42010/1016 SAD: stakeholders, architectural views, interfaces, traceability, and deviations.
- [SAD PDF](SAD.pdf) — print-ready export of the canonical SAD (`docs/Architecture.md`).
- [Architecture Selection and Justification](Architecture_Selection.md).
- [API and Component Interfaces](API_and_Component_Interfaces.md).
- **Architecture diagram (PlantUML):** [source](Architecture.puml) · [PNG](Architecture.png) · [SVG](Architecture.svg) — component and layered view with the POSIX services the shell depends on.
- [System Architecture](System_Architecture.svg).
- [Component Diagram](Component_Diagram.svg).
- **Sequence diagrams:** [Foreground command execution](Sequence_Diagram.svg) · [Background execution and asynchronous reaping](Sequence_Background_Reaping.svg) ([PlantUML source](Sequence_Background_Reaping.puml)).
- **Use Case Diagram (UML):** [source](Use_Case_Diagram.puml) · [SVG](Use_Case_Diagram.svg) · [PNG](Use_Case_Diagram.png) — two actors, ten use cases, include/extend/generalisation relationships.

The SVGs are editable vector diagrams. The system has C module APIs; it does not expose a web API.

## Regenerating the PlantUML diagrams

```bash
java -jar plantuml.jar -tpng 4-Architectural-Design/Architecture.puml
java -jar plantuml.jar -tsvg 4-Architectural-Design/Architecture.puml
java -jar plantuml.jar -tpng 4-Architectural-Design/Use_Case_Diagram.puml
java -jar plantuml.jar -tsvg 4-Architectural-Design/Use_Case_Diagram.puml
java -jar plantuml.jar -tpng 4-Architectural-Design/Sequence_Background_Reaping.puml
java -jar plantuml.jar -tsvg 4-Architectural-Design/Sequence_Background_Reaping.puml
```

The sources use `!pragma layout smetana`, so Graphviz is not required.
