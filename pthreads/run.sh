# ./run.sh -al | tee output.csv





for threads in {2,4} 
do
	for size in 1  2 5 10 50 100 200 500 1000 
	do
		for i in {1..5} 
		do
			let arraysize=$((size*1000000))
			./par_pthread_sort $threads $arraysize
		done
	done	
done




