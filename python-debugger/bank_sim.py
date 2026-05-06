"""
bank_sim.py  --  Bank Transaction Processor
A realistic simulation that crashes on corrupt input data.

Designed to showcase every debugger feature:
  Watchpoints  : total_volume (write), alice_balance (read/write), txn (both)
  Breakpoints  : lines 57, 67, 75
  Eval exprs   : len(processed), txn['to'], alice_balance - 300, [t['id'] for t in processed]
  Commands     : n, back, c, rc, summary, show, find, jump
"""

# ── Static data ────────────────────────────────────────────────────────────────

ACCOUNTS = {
    "ACC001": {"owner": "Alice", "balance": 1000.0},
    "ACC002": {"owner": "Bob",   "balance":  500.0},
    "ACC003": {"owner": "Carol", "balance": 2500.0},
}

# TXN004 intentionally references a nonexistent account – this is the bug.
TRANSACTIONS = [
    {"id": "TXN001", "from": "ACC001", "to": "ACC002", "amount": 200.0},
    {"id": "TXN002", "from": "ACC003", "to": "ACC001", "amount": 150.0},
    {"id": "TXN003", "from": "ACC002", "to": "ACC003", "amount":  75.0},
    {"id": "TXN004", "from": "ACC001", "to": "ACC999", "amount": 300.0},  # <-- corrupt: ACC999 missing
    {"id": "TXN005", "from": "ACC003", "to": "ACC001", "amount":  50.0},  # never reached
]

# ── Core logic ─────────────────────────────────────────────────────────────────

def validate_transaction(txn, accounts):
    """Return an error string, or None if the transaction looks valid."""
    if txn["from"] not in accounts:
        return f"Unknown source account: {txn['from']}"
    if accounts[txn["from"]]["balance"] < txn["amount"]:
        return f"Insufficient funds in {txn['from']}"
    # BUG: destination is never validated here
    return None

def apply_transfer(accounts, src_id, dst_id, amount):
    """Debit src and credit dst.  Crashes if dst_id is not in accounts."""
    accounts[src_id]["balance"] -= amount
    accounts[dst_id]["balance"] += amount   # KeyError when dst_id == "ACC999"
    return amount

def process_batch(transactions, accounts):
    processed   = []
    total_volume = 0.0
    error_count  = 0

    for i, txn in enumerate(transactions):
        print(f"[{i+1}/{len(transactions)}] {txn['id']}  "
              f"{txn['from']} -> {txn['to']}  ${txn['amount']:.2f}")

        err = validate_transaction(txn, accounts)
        if err:
            print(f"  SKIP: {err}")
            error_count += 1
            continue

        transferred   = apply_transfer(accounts, txn["from"], txn["to"], txn["amount"])
        total_volume += transferred
        processed.append(txn)

        # Snapshot individual balances so the debugger can watch them change
        alice_balance = accounts["ACC001"]["balance"]
        bob_balance   = accounts["ACC002"]["balance"]
        carol_balance = accounts["ACC003"]["balance"]

        print(f"  OK  transferred=${transferred:.2f}  "
              f"volume=${total_volume:.2f}  "
              f"alice=${alice_balance:.2f}  bob=${bob_balance:.2f}  carol=${carol_balance:.2f}")

    return processed, total_volume, error_count

# ── Entry point ────────────────────────────────────────────────────────────────

def main():
    print("=== Bank Transaction Processor ===")
    print(f"Accounts    : {len(ACCOUNTS)}")
    print(f"Transactions: {len(TRANSACTIONS)}\n")

    accounts = {k: dict(v) for k, v in ACCOUNTS.items()}   # shallow-copy each account

    processed, total_volume, error_count = process_batch(TRANSACTIONS, accounts)

    # Only reached on a clean run
    print(f"\n=== Summary ===")
    print(f"Processed : {len(processed)} transactions")
    print(f"Errors    : {error_count}")
    print(f"Volume    : ${total_volume:.2f}")
    print(f"Alice end : ${accounts['ACC001']['balance']:.2f}")
    print(f"Bob end   : ${accounts['ACC002']['balance']:.2f}")
    print(f"Carol end : ${accounts['ACC003']['balance']:.2f}")

if __name__ == "__main__":
    main()
