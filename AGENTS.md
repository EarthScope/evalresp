Project instructions for this repo. These override default behavior.

## Git: never write to the repository unless told to

- Never run `git commit` unless I explicitly ask you to commit.
- Never run `git push`, to any remote, for any reason. There is no exception.
- Don't `git add` as a side effect of doing other work.

Reading git state (`git status`, `git diff`, `git log`) is always fine.

## After applying a fix, do these things unprompted

1. Report a single-line commit message: imperative mood. Say what the change does,
   not how it was arrived at.

2. Add that same line to `ChangeLog` as a `* ` bullet in the top dated section:

   If the top section already carries a released version,
   start a new `Unreleased` section above it,
   for today.

3. If this fix can be tested, add a regression test to the test suite.

## Code comments

Write for a future reader who sees only the final code — never the previous state,
and never the conversation that produced it.

- Match the style, tone, and density of the comments adjacent to the code you are
  editing. Terse.
- State what the code does or what invariant holds. Do not state why it changed,
  what it used to do, or that it is new, fixed, or optimized.
- Never reference a conversation, a code review, a report, a to-do or error list,
  a benchmark run, or an AI model. None of that means anything in a release.
- Don't comment the obvious and don't restate the function name.

## Never break the public API

Do not change the public API with explicit instruction.

Adding new functions is fine. If a change you believe is correct requires breaking
API, stop and tell me rather than doing it.
