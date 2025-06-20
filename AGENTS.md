# AGENTS

## Scope
These instructions apply to the entire repository.

## Coding Guidelines
- Keep the coding style flexible. Follow existing patterns or whichever style feels natural.
- All new or modified functions, classes or modules must include Doxygen-style docstrings.
- Include a literal `[AI GENERATED]` tag in the docstring or as a comment near the top of newly created files to indicate AI involvement.

## Testing Guidelines
- Provide unit tests that exercise every line of newly written or modified code. Avoid mocks or simulated behavior unless specifically required.
- Use the real functionality for tests.
 - Run `./build_and_test.sh` after any code changes and ensure all tests pass before committing.
 - If a change only affects documentation, running tests is optional.

## Repository Overview
See `CODEBASE_OVERVIEW.md` for a quick description of directories and files.

## CLI Usage
The `piano_synth` executable accepts `--record` to begin recording immediately and `--config` to display the note settings manager.

