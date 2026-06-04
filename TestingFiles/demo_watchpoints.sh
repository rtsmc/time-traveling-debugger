#!/bin/bash
# Quick demo of watchpoint functionality

echo "Watchpoint Feature Demo"
echo "======================="
echo ""
echo "This demo shows how to use watchpoints in the trace viewer."
echo ""
echo "Step 1: Run the test program"
echo "  python3 idebug.py test_watchpoint.py"
echo "  > run"
echo ""
echo "Step 2: The trace viewer will open automatically"
echo ""
echo "Step 3: Try these commands:"
echo "  > w counter              # Set watchpoint on 'counter' (read/write)"
echo "  > ww result              # Set write watchpoint on 'result'"
echo "  > rw x                   # Set read watchpoint on 'x'"
echo "  > listw                  # List all watchpoints"
echo "  > c                      # Continue to next watchpoint/breakpoint"
echo "  > c                      # Continue to next"
echo "  > rc                     # Reverse continue to previous"
echo "  > clearw 1               # Clear watchpoint #1"
echo "  > clearw                 # Clear all watchpoints"
echo ""
echo "For complete documentation, see WATCHPOINT_GUIDE.md"
echo ""
echo "Press Enter to start the demo..."
read

cd /home/claude/python-debugger
python3 idebug.py test_watchpoint.py
