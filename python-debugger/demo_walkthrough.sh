# ─────────────────────────────────────────────────────────────────────────────
# DEMO WALKTHROUGH  –  bank_sim.py  with the time-traveling debugger
# Run:  python3 idebug.py bank_sim.py bank_sim.log
# ─────────────────────────────────────────────────────────────────────────────
#
# The program processes 5 bank transactions.  TXN004 references account
# "ACC999" which doesn't exist, causing a KeyError in apply_transfer().
# All state up to the crash is captured in bank_sim.log so you can
# rewind and inspect freely in the post-execution trace viewer.
# ─────────────────────────────────────────────────────────────────────────────


# ── PHASE 1: Pre-execution setup (idebug.py CLI) ──────────────────────────────

# Show the source before setting anything
show bank_sim.py

# --- Breakpoints ---
# Pause just before each transaction is applied (the validate call)
b bank_sim.py 57          # line: err = validate_transaction(txn, accounts)

# Pause right after a successful transfer, where per-account snapshots are taken
b bank_sim.py 67          # line: alice_balance = accounts["ACC001"]["balance"]

# --- Watchpoints ---
# Track the running dollar total every time it's written
ww total_volume           # write-only: fires each time total_volume changes

# Track Alice's balance on every read AND write
w alice_balance           # read/write: shows every access

# Watch the current transaction object (both modes to see when it changes
# each loop iteration and when validate_transaction reads it)
w txn                     # read/write watchpoint on txn

# Confirm everything is configured
list
listw

# Run the program – it will crash on TXN004 and drop straight into the viewer
run


# ── PHASE 2: Post-execution trace viewer (traceviewer) ───────────────────────
# The viewer opens at the crash frame.  All commands below are typed there.

# --- Orientation ---
summary                   # see total entries, crash location, variable snapshot

show bank_sim.py          # review the source with breakpoint markers

# --- Step forward/backward ---
n                         # step one entry forward  (or back toward the crash)
back                      # step one entry backward  (time travel!)

# --- Continue / reverse-continue between watchpoints ---
c                         # jump forward to the next breakpoint or watchpoint hit
rc                        # jump BACKWARD to the previous hit  ← unique feature!

# --- Watch total_volume accumulate across TXN001-TXN003 ---
# Reverse to the start, then c repeatedly to see each write to total_volume:
jump 1                    # rewind to trace entry 1
c                         # hit: total_volume write after TXN001  ($200)
c                         # hit: total_volume write after TXN002  ($350)
c                         # hit: total_volume write after TXN003  ($425)
c                         # hit: KeyError crash on TXN004

# --- Eval expressions at the crash frame ---
eval txn                  # show the bad transaction dict
eval txn['to']            # "ACC999" – the nonexistent account
eval txn['amount']        # 300.0 – the amount that never transferred

eval len(processed)       # 3 – only TXN001-TXN003 succeeded
eval total_volume         # 425.0 – volume before the crash

eval alice_balance        # Alice's last known balance ($950 after TXN001, +150 from TXN002)
eval bob_balance          # Bob's last known balance
eval carol_balance        # Carol's last known balance

# Compound expressions are fine too:
eval [t['id'] for t in processed]              # ['TXN001', 'TXN002', 'TXN003']
eval sum(t['amount'] for t in processed)       # 425.0

# Import then use a module:
import math
eval math.floor(total_volume)                  # 425

# --- Find a specific trace entry ---
find total_volume                              # locate all entries where total_volume appears
find alice_balance                             # find every frame where alice_balance is live

# --- Jump to a specific entry to compare state ---
jump 1                    # back to the very first trace entry
eval total_volume         # 0.0 – not yet incremented
eval alice_balance        # 1000.0 – starting balance

# --- Inspect the write watchpoint that will confirm the bug ---
w alice_balance           # add a fresh watchpoint (already have one from setup)
rc                        # reverse-continue: land on the last read of alice_balance
eval txn['id']            # which transaction were we in?

quit                      # exit the viewer
