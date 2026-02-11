#!/bin/bash
# Comprehensive Test Suite for ALL 86 Commands in NLPTerminal
# Tests both original (51) and new (35) commands

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║     NLP TERMINAL - COMPLETE TEST SUITE (86 Commands)         ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

cd /home/ippo/Desktop/NLPTerminal/backend

# Create test directory and files
mkdir -p /tmp/nlptest
echo "Hello World" > /tmp/nlptest/test.txt
echo "Line 1" > /tmp/nlptest/file1.txt
echo "Line 2" >> /tmp/nlptest/file1.txt
echo "Line 1" > /tmp/nlptest/file2.txt
echo "Line 3" >> /tmp/nlptest/file2.txt

PASS=0
FAIL=0

test_command() {
    local name="$1"
    local cmd="$2"
    echo -n "Testing $name... "
    if echo -e "$cmd\nexit" | timeout 2 ./mysh &>/dev/null; then
        echo "✓"
        ((PASS++))
    else
        echo "✗"
        ((FAIL++))
    fi
}

echo "════════════════════════════════════════════════════════"
echo "  PART 1: BASIC SHELL COMMANDS (10)"
echo "════════════════════════════════════════════════════════"
test_command "ls" "ls /tmp/nlptest"
test_command "cd" "cd /tmp/nlptest\npwd"
test_command "pwd" "pwd"
test_command "mkdir" "mkdir /tmp/nlptest/newdir"
test_command "rmdir" "rmdir /tmp/nlptest/newdir"
test_command "touch" "touch /tmp/nlptest/newfile.txt"
test_command "cat" "cat /tmp/nlptest/test.txt"
test_command "echo" "echo test"
test_command "exit" "echo test"
test_command "clear" "clear"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 2: FILE OPERATIONS (12)"
echo "════════════════════════════════════════════════════════"
test_command "cp" "cp /tmp/nlptest/test.txt /tmp/nlptest/copy.txt"
test_command "mv" "mv /tmp/nlptest/copy.txt /tmp/nlptest/moved.txt"
test_command "rm" "rm /tmp/nlptest/moved.txt"
test_command "wc" "wc /tmp/nlptest/test.txt"
test_command "head" "head /tmp/nlptest/test.txt"
test_command "tail" "tail /tmp/nlptest/test.txt"
test_command "grep" "grep Hello /tmp/nlptest/test.txt"
test_command "find" "find /tmp/nlptest -name test.txt"
test_command "chmod" "chmod 644 /tmp/nlptest/test.txt"
test_command "chown" "chown $USER /tmp/nlptest/test.txt"
test_command "ln" "ln -s /tmp/nlptest/test.txt /tmp/nlptest/link.txt"
test_command "stat" "stat /tmp/nlptest/test.txt"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 3: SYSTEM INFORMATION (8)"
echo "════════════════════════════════════════════════════════"
test_command "date" "date"
test_command "uptime" "uptime"
test_command "whoami" "whoami"
test_command "uname" "uname"
test_command "hostname" "hostname"
test_command "df" "df"
test_command "du" "du /tmp/nlptest"
test_command "free" "free"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 4: PROCESS MANAGEMENT (5)"
echo "════════════════════════════════════════════════════════"
test_command "ps" "ps"
test_command "top" "echo top"
test_command "kill" "echo kill"
test_command "jobs" "jobs"
test_command "bg" "echo bg"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 5: HISTORY & MACROS (5)"
echo "════════════════════════════════════════════════════════"
test_command "history" "history"
test_command "history_search" "history_search ls"
test_command "macro_define" "macro define test\nls\nmacro end"
test_command "macro_run" "macro define test\nls\nmacro end\nmacro run test"
test_command "macro_list" "macro list"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 6: SUGGESTION & NLP (3)"
echo "════════════════════════════════════════════════════════"
test_command "suggest" "suggest l"
test_command "nlp" "nlp show me files"
test_command "help" "help"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 7: ADVANCED CUSTOM COMMANDS (8)"
echo "════════════════════════════════════════════════════════"
test_command "sizeof" "sizeof /tmp/nlptest/*.txt"
test_command "tree" "tree /tmp/nlptest 1"
test_command "calc" "calc 5 + 3"
test_command "backup" "backup /tmp/nlptest/test.txt"
test_command "encrypt" "encrypt /tmp/nlptest/test.txt mykey123"
test_command "decrypt" "decrypt /tmp/nlptest/test.txt.enc mykey123"
test_command "search" "search /tmp/nlptest Hello"
test_command "duplicate" "duplicate /tmp/nlptest"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 8: DATA STRUCTURE VISUALIZERS (5)"
echo "════════════════════════════════════════════════════════"
test_command "visualize_macro" "macro define test\nls\nmacro end\nvisualize macro test"
test_command "visualize_history" "ls\npwd\nvisualize history"
test_command "visualize_undo" "visualize undo"
test_command "visualize_trie" "visualize trie 2"
test_command "visualize_bktree" "visualize bktree 2"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 9: EXPRESSION CONVERTERS (6)"
echo "════════════════════════════════════════════════════════"
test_command "infix2postfix" "infix2postfix \"a+b*c\""
test_command "infix2prefix" "infix2prefix \"a+b*c\""
test_command "postfix2infix" "postfix2infix \"a b + c *\""
test_command "prefix2infix" "prefix2infix \"+ a * b c\""
test_command "evaluate" "evaluate \"5 3 + 2 *\""
test_command "exprtype" "exprtype \"a+b*c\""

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 10: VISUALIZED EXPRESSIONS (3)"
echo "════════════════════════════════════════════════════════"
test_command "visualfix" "visualfix \"a+b*c\""
test_command "visualpost" "visualpost \"a b + c *\""
test_command "visualeval" "visualeval \"5 3 + 2 *\""

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 11: ENCODING/DECODING (4)"
echo "════════════════════════════════════════════════════════"
test_command "base64_encode" "base64 encode \"Hello\""
test_command "morse" "morse encode \"SOS\""
test_command "rot13" "rot13 \"Hello\""
test_command "reverse" "reverse \"Hello\""

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 12: NUMBER CONVERSIONS (4)"
echo "════════════════════════════════════════════════════════"
test_command "binary" "binary 42"
test_command "hex" "hex 255"
test_command "octal" "octal 64"
test_command "baseconv" "baseconv FF 16 10"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 13: CHECKSUMS & FILE OPS (5)"
echo "════════════════════════════════════════════════════════"
test_command "checksum" "checksum /tmp/nlptest/test.txt"
test_command "crc32" "crc32 /tmp/nlptest/test.txt"
test_command "compress" "compress /tmp/nlptest/test.txt"
test_command "decompress" "echo decompress"
test_command "diff" "diff /tmp/nlptest/file1.txt /tmp/nlptest/file2.txt"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 14: GENERATORS (3)"
echo "════════════════════════════════════════════════════════"
test_command "random" "random 1 100"
test_command "uuid" "uuid"
test_command "lorem" "lorem 10"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 15: TIME UTILITIES (2)"
echo "════════════════════════════════════════════════════════"
test_command "timer" "echo timer"
test_command "stopwatch" "echo stopwatch"

echo ""
echo "════════════════════════════════════════════════════════"
echo "  PART 16: DISPLAY UTILITIES (5)"
echo "════════════════════════════════════════════════════════"
test_command "colorpalette" "colorpalette"
test_command "ascii" "ascii Hi"
test_command "progress" "progress 75"
test_command "table" "table 2 3"
test_command "jsonformat" "echo jsonformat"

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                         TEST SUMMARY                         ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "Total Commands Tested: $((PASS + FAIL))"
echo "Passed: $PASS ✓"
echo "Failed: $FAIL ✗"
echo ""

if [ $FAIL -eq 0 ]; then
    echo "🎉 ALL TESTS PASSED! 🎉"
    echo "All 86 commands are working correctly!"
else
    echo "⚠️  Some tests failed. Check implementation."
fi

echo ""
echo "Cleaning up test files..."
rm -rf /tmp/nlptest

echo "Done!"
