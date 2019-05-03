gcc -w ./cl_utils.c ./disassemble.c -o ./disassemble.out

TARGETS=("hello_arm" "print_loop" "print_hex" "print_nomem" "print_ng" "putchar_bad" "putchar_mem" "print_hex_mem")

for TARGET in ${TARGETS[@]}; do
	DIFF=`diff <(cat "./test/test_expect/$TARGET.txt") <(./disassemble.out "./test/test_input/$TARGET.bin")`

	if [ "$DIFF" = "" ]; then
    	echo "Regression test SUCESS: $TARGET"
	else
    	echo "Regression test FAIL: $TARGET"
    	echo $DIFF
	fi
done
