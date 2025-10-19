#let을 이용한 피보나치 수열
a=0
b=1

echo "피보나치 수열 :"

for i in {1..10}
do 
	let c=a+b
	echo -n "$c "
	a=$b
	b=$c
done
echo
