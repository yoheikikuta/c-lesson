gcc -w ./cl_util.c ./cl_getline.c ./parser.c ./binary_tree.c ./dict.c ./unsolved_label_address_list.c ./assembler.c -o ./assembler.out
gcc -w ./cl_util.c ./regression_test_single_file.c -o ./regression_test_single_file.out

TARGETS=("test_assembler" "test_hello_asm" "test_print_loop" "test_print_hex")
EXPECT_BASE="/test/test_expect/"
ACTUAL_BASE="/test/test_input/"

# Assemble target files.
for TARGET in ${TARGETS[@]}; do
    ./assembler.out ${ACTUAL_BASE}${TARGET}.ks ${ACTUAL_BASE}${TARGET}.bin
done

# Regressin test.
echo ""
for TARGET in ${TARGETS[@]}; do
    DIFF=`./regression_test_single_file.out ${EXPECT_BASE}${TARGET}.bin ${ACTUAL_BASE}${TARGET}.bin`

	if [ "$DIFF" = "" ]; then
    	echo "Regression test SUCESS: $TARGET"
	else
    	echo "Regression test FAIL: $TARGET"
    	echo "$DIFF"
	fi
done
echo ""