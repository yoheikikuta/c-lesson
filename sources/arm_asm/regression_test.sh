gcc -w ./04_disasm/cl_utils.c ./04_disasm/disassemble.c -o ./04_disasm/disassemble.out

EXPECT_FILE="./04_disasm/test/test_expect/hello_arm.txt"
ACTUAL_FILE="./04_disasm/test/test_input/hello_arm.bin"

DIFF=`diff <(cat $EXPECT_FILE) <(./04_disasm/disassemble.out $ACTUAL_FILE)`

if [ "$DIFF" = "" ]; then
    echo "Regression test SUCESS: $ACTUAL_FILE"
else
    echo "Regression test FAIL: $ACTUAL_FILE"
    echo $DIFF
fi
