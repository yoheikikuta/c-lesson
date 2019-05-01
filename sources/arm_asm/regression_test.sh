gcc -w ./04_disasm/cl_utils.c ./04_disasm/disassemble.c -o ./04_disasm/disassemble.out

TARGETS=("hello_arm" "print_loop" "print_hex" "print_nomem")

for TARGET in ${TARGETS[@]}; do
	DIFF=`diff <(cat "./04_disasm/test/test_expect/$TARGET.txt") <(./04_disasm/disassemble.out "./04_disasm/test/test_input/$TARGET.bin")`

	if [ "$DIFF" = "" ]; then
    	echo "Regression test SUCESS: $TARGET"
	else
    	echo "Regression test FAIL: $TARGET"
    	echo $DIFF
	fi
done
