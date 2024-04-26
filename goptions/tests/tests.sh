#!/bin/zsh

# simpleExample

simpleExampleTests=( simpleExampleBothDefined simpleExampleDefaultMissing simpleExampleEmptyJcard simpleExampleOneSameAsDefault simpleExampleEmptyJcard )
structuredExampleTests=( structureExample )
cumulativeStructureExampleTests=( cumulativeStructureExample )

echo

if [[ $1 = 'reset' ]]; then

	echo simpleExample reset
	echo -------------------
	echo

	echo " Resetting no jcard"
	check1=tests/simpleExampleNoJcard.txt
	check2=tests/simpleExampleNoJcardWithGui.txt
	$(./simpleExample      > $check1)
	$(./simpleExample -gui > $check2)

	for t in $simpleExampleTests
	do
		echo " Resetting " $t
		jcard=tests/$t".jcard"
		check1=tests/$t".txt"
		check2=tests/$t"WithGui.txt"
		rm -f $check1 $check2
		$(./simpleExample $jcard      > $check1)
		$(./simpleExample $jcard -gui > $check2)
	done

	echo
	echo
	echo structureExample reset
	echo ----------------------
	echo
	for t in $structuredExampleTests
	do
		echo " Resetting " $t
		jcard=tests/$t".jcard"
		check=tests/$t".txt"
		rm -f $check
		$(./structureExample $jcard > $check)
	done

	echo
	echo
	echo cumulativeExample reset
	echo -----------------------
	echo
	for t in $cumulativeStructureExampleTests
	do
		echo " Resetting " $t
		jcard=tests/$t".jcard"
		check=tests/$t".txt"
		rm -f $check
		$(./cumulativeStructureExample $jcard > $check)
	done



else

	echo simpleExample tests
	echo -------------------
	echo

	# first running w/o jcard
	check1=tests/simpleExampleNoJcard.txt
	check2=tests/simpleExampleNoJcardWithGui.txt
	a1=$(./simpleExample)
	a2=$(./simpleExample -gui)
	b1=$(cat $check1)
	b2=$(cat $check2)
	if [[ "$a1" = "$b1" ]]; then
		echo " - no jcard:          success"
	else
		echo " - no jcard: fail"
		echo ---
		echo $a1
		echo ---
		echo $b1
		echo ---
	fi
	if [[ "$a2" = "$b2" ]]; then
		echo " - no jcard with gui: success"
	else
		echo " - no jcard with gui: fail"
		echo ---
		echo $a2
		echo ---
		echo $b2
		echo ---
	fi

	for t in $simpleExampleTests
	do
		jcard=tests/$t".jcard"
		check1=tests/$t".txt"
		check2=tests/$t"WithGui.txt"
		a1=$(./simpleExample $jcard)
		a2=$(./simpleExample $jcard  -gui)
		b1=$(cat $check1)
		b2=$(cat $check2)
		if [[ "$a1" = "$b1" ]]; then
			echo " - "$t":          success"
		else
			echo " - "$t": fail"
			echo ---
			echo $a1
			echo ---
			echo $b1
			echo ---
		fi
		if [[ "$a2" = "$b2" ]]; then
			echo " - "$t" with gui: success"
		else
			echo " - "$t" with gui: fail"
			echo ---
			echo $a2
			echo ---
			echo $b2
			echo ---
		fi
	done


	echo
	echo
	echo structureExample tests
	echo ----------------------
	echo

	for t in $structuredExampleTests
	do
		jcard=tests/$t".jcard"
		check=tests/$t".txt"
		a3=$(./structureExample $jcard)
		b3=$(cat $check)
		if [[ "$a3" = "$b3" ]]; then
			echo " - "$t":          success"
		else
			echo " - "$t": fail"
			echo ---
			echo $a3
			echo ---
			echo $b3
			echo ---
		fi
	done

	echo
	echo
	echo cumulativeStructureExample tests
	echo --------------------------------
	echo

	for t in $cumulativeStructureExampleTests
	do
		jcard=tests/$t".jcard"
		check=tests/$t".txt"
		a3=$(./cumulativeStructureExample $jcard)
		b3=$(cat $check)
		if [[ "$a3" = "$b3" ]]; then
			echo " - "$t":          success"
		else
			echo " - "$t": fail"
			echo ---
			echo $a3
			echo ---
			echo $b3
			echo ---
		fi
	done

fi
echo
