#!/bin/bash


#checking directory
if [ -e $1 ]
then
	if [ ! -d $1 ]
	then 
		echo "File" $1 "is not a directory!"
		exit 1
	fi
else
	echo "File" $1 "doesn't exists!"
	exit 2
fi

#checking if directory is not empty
if [ "$(ls -A $1)" ]
then
	echo "# Warning: directory is full, purging ..."
	rm -rf root_directory/*
else
	echo "# Directory is empty ..."
fi

#checking w
A=$3
if [[ $A =~ ^[0-9]+$ ]] && (( A > 0)); then
	#null command
	:
else
	echo "Third argument must be a positive integer!"
	exit 3
fi

#checking p
B=$4
if [[ $B =~ ^[0-9]+$ ]] && (( B > 0)); then
	#null command
	:
else
	echo "Fourth argument must be a positive integer!"
	exit 4
fi

#checking number of lines of file
LINES_OF_FILE=`cat $2 | wc -l`
if [[ "$LINES_OF_FILE" -lt 10000 ]]
then
	echo "File $2 must have more than 10000 lines!"
	echo "It has $LINES_OF_FILE lines"
	exit 5
fi


n=0;
max=$3;
while [ "$n" -lt "$max" ]; do
  mkdir "./$1/site$n"
  n=`expr "$n" + 1`;
done


ALL_FILES=()


m=0;
n=0;
max=$3;
max1=$4;
RANGE=20000;
FLOOR=0;

#for each site (w)
while [ "$m" -lt "$max" ]
do 
#	for each page (p)
	while [ "$n" -lt "$max1" ]
	do
#		get the random number
		number=0
		while [ "$number" -le $FLOOR ]
		do
			number=$RANDOM
			let "number %= $RANGE"
		done

		while [[ -e "./$1/site$m/page${m}_${number}.html" ]]
		do
			number=0
			while [ "$number" -le $FLOOR ]
			do
				number=$RANDOM
				let "number %= $RANGE"
			done
		done

		touch "./$1/site$m/page${m}_${number}.html"

		ALL_FILES+=("./$1/site$m/page${m}_${number}.html")

		n=`expr "$n" + 1`;
#		echo "-->n == $n"		
	done
	n=0
#	echo "m == $m"
#	echo "max == $max"
	m=`expr "$m" + 1`;	
done



RANGE=$4;
max=$3;
number_of_site=0

FLOOR_k=1
RANGE_k=$(($LINES_OF_FILE-2000))

FLOOR_m=1000
RANGE_m=2000

TOTAL_LINKS=()

while [ "$number_of_site" -lt "$max" ]
do
#	echo "$3"
	echo "# Creating web site ${number_of_site} ..."

	EXTERNAL_FILES=()
	ALL_EXTERNAL_FILES=()
	for ((i = 0; i < "$3"; i++))
	do
		if [ "$i" != "$number_of_site" ]
		then
			EXTERNAL_FILES=`ls ./$1/site${i}/page${i}_*.html `
			ALL_EXTERNAL_FILES+=($EXTERNAL_FILES)
		fi
	done

	CURRENT_FILES_OF_DIR=()
	CURRENT_FILES_OF_DIR=`ls ./$1/site${number_of_site}/page${number_of_site}_*.html`
	CURRENT_FILES_OF_DIR=($CURRENT_FILES_OF_DIR)
#	echo "there"

	for page in ${CURRENT_FILES_OF_DIR[@]}
	do
		k=0
		while [ "$k" -le $FLOOR_k ]
		do
			k=$RANDOM
			let " k %= $RANGE_k"
		done
#		echo "k == $k"

		m=0
		while [ "$m" -le $FLOOR_m ]
		do
			m=$RANDOM
			let " m %= $RANGE_m"
		done
#		echo "m == $m"

		f=`expr "$B" / 2 + 1`;
#		echo "f == $f"

#		find internal links
		Internal_links=()
		i=0
		flag=1
		while [[ "$i" -lt "$f" ]]
		do
			number=-1
			while [ "$number" -lt $FLOOR ]
			do
				number=$RANDOM
				let "number %= $RANGE"
			done

			if [ "${page}" != "${CURRENT_FILES_OF_DIR[$number]}" ]
			then

				for j in ${Internal_links[@]}
				do
					if [ "${CURRENT_FILES_OF_DIR[$number]}" == "$j" ]
					then
						flag=0
					fi	
				done
				if [ "$flag" == 1 ]
				then
					Internal_links+=("${CURRENT_FILES_OF_DIR[$number]}")
					i=`expr "$i" + 1`;					
				fi
				flag=1
			fi

		done

		q=`expr "$A" / 2 + 1`;


		External_links=()
		i=0
		flag=1
		while [[ "$i" -lt "$q" ]]
		do
			number=-1
			CURRENT_RANGE=`expr $3 \* $4 - $4`
			while [ "$number" -le "0" ]
			do
				number=$RANDOM
				let "number %= $CURRENT_RANGE"
			done

			for j in ${External_links[@]}
			do
				if [ "${ALL_EXTERNAL_FILES[$number]}" == "$j" ]
				then
					flag=0
				fi
			done
			if [ "$flag" == 1 ]
			then
				External_links+=("${ALL_EXTERNAL_FILES[$number]}")
				i=`expr "$i" + 1`;
			fi
			flag=1
		done

		All_links=( "${Internal_links[@]}" "${External_links[@]}" )

		echo "#	Creating page ${page} with ${m} lines starting at line ${k} ..."

		echo "<!DOCTYPE html>" >> "${page}"
		echo "<html>" >> "${page}"
		echo "	<body>" >> "${page}"
		b=`expr "$f" + "$q"`;
		a=`expr "$m" / "$b"`; 
		total=`expr "$a" + "$k"`;
		for (( l = 0; l < "$b"; l++))
		do
#			echo "k == $k"
#			echo "total == $total"
			for (( i = "$k"; i <= "${total}"; i++ ))
			do
				echo -n "	" >> "${page}"
				sed -n "${i}p" "$2" >> "${page}"
				if [ "$i" -lt "$total" ]
				then
					echo "	<br>" >> "${page}"
				else
					echo -n "	<br>" >> "${page}"
				fi
			done
			select_link=$RANDOM
			let "select_link %= $b"
			while [ "${All_links[select_link]}" == "-" ]
			do
				select_link=$RANDOM
				let "select_link %= $b"
			done
			file=$(basename "${All_links[select_link]}")
			file2=$(dirname "${All_links[select_link]}")
			file3=$(basename "${file2}")
			#echo $file
			#echo $file2
			#echo $file3
			echo " <a href=\"../${file3}/${file}\">${file}</a>" >> "${page}"
			TOTAL_LINKS+=(${All_links[select_link]})
			echo "#	Adding link to ${All_links[select_link]}"
			All_links[select_link]="-"
			k=`expr "$total"`;
			total=`expr "$total" + "$a"`;
		done
		echo "	</body>" >> "${page}"
		echo "</html>" >> "${page}"	

	done

	number_of_site=`expr "$number_of_site" + 1`;

done


#FILES=()
#ALL_FILES=()
#for ((i = 0; i < "$3"; i++))
#do
#	FILES=`ls ./$1/site${i}/page${i}_*.html `
#	ALL_FILES+=($FILES)
#done



count=0
for i in "${ALL_FILES[@]}"
do
	for j in "${TOTAL_LINKS[@]}"
	do
		if [ "$i" == "$j" ]
		then
			count=`expr "$count" + 1`;
			break
		fi
	done
done

if [ "$count" == "${#ALL_FILES[@]}" ]
then
	echo "# All pages have at least one incoming link"
else
	count=`expr "${#ALL_FILES[@]}" - "$count"`;
	echo "# $count pages have not incoming link"
fi

echo "# Done."

#echo "number_of_site == $number_of_site"
#echo "\$3 == $3"
#echo "\$4 == $4"
exit 0
