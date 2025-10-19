#for문을 이용한 구구단
for i in {1..9}
do
	echo "$i 단:"
	for j in {1..9}
	do
		echo "$i x $j = $((i*j))"
	done
	echo
done



