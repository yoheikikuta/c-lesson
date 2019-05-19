gcc -w ./cl_util.c ./cl_getline.c ./parser.c ./assembler.c -o ./assembler.out

TARGETS=("test_assembler")

# Assemble target files.
for TARGET in ${TARGETS[@]}; do
    ./assembler.out /test/test_input/$TARGET.ks /test/test_input/$TARGET
done

# Regressin test.
for TARGET in ${TARGETS[@]}; do
	DIFF=`diff <(cat "./test/test_expect/$TARGET.txt") <(cat "./test/test_input/$TARGET")`

	if [ "$DIFF" = "" ]; then
    	echo "Regression test SUCESS: $TARGET"
	else
    	echo "Regression test FAIL: $TARGET"
    	echo $DIFF
	fi
done