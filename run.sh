if [ "$#" -lt 4 ]; then
	echo "Usage: sh run_referee.sh [CMD PLAYER1] [CMD PLAYER 2] [CMD PLAYER 3] [LEAGUE LEVEL] {GAMES, default 1} {threads, default 1}"
	echo "CMD needs to be runnable here, likes \"./a.out\" or \"python3 script.py\""
	echo "League level is 1 for Wood2, 2 for Wood1, 3 for Bronze"
	exit $1
fi

games=1
if [ "$#" -gt 4 ]; then
	games=$5
fi

threads=1
if [ "$#" -gt 5 ]; then
	threads=$6
fi


java -jar brutaltester.jar -r "java -Dleague.level=$4 --add-opens java.base/java.lang=ALL-UNNAMED -jar referee.jar" -p1 "$1" -p2 "$2" -p3 "$3" -n $games -t $threads