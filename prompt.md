## Step 1: Read Context
Read these files:
- README.md
- AGENTS.md
- implementation-plan.md

## Step 2: Pick Task
- Select EXACTLY ONE task from implementation-plan.md.
- If dependencies are incomplete, implement the dependencies first.
- If no clear task is available, pick the first incomplete item.

## Step 3: Execute Task (MANDATORY FLOW)

You MUST follow this sequence strictly:

1. Write tests for the selected task
   - Prefer unit tests
   - Tests must fail initially

2. Implement the code
   - Make minimal changes required to pass tests

3. Run tests
   - If tests fail → fix code immediately
   - Repeat until all tests pass

## Step 4: Update Plan
- Mark the task as completed in implementation-plan.md

## Step 5: Commit
- Stage all files
- Commit with a clear message
- Push to remote

## Anti-Stall Rules (CRITICAL)

- NEVER stop after reading files
- NEVER ask for clarification
- NEVER wait for input
- NEVER output only explanations

If unsure:
- Make a reasonable assumption
- Proceed with implementation

If stuck:
- Pick a smaller sub-task
- OR create a minimal working version

You must ALWAYS produce code changes or tests in every run.

## Output Requirement

Every cycle MUST include at least one of:
- New or updated test
- New or updated code
- Updated implementation-plan.md
- Git commit

Doing nothing is NOT allowed.


