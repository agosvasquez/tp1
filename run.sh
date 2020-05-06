#!/bin/bash

if [ "$1" == "--help" ] || [ $# -lt 2 ]; then
	echo -e "Uso: run.sh port [valgrind|no-valgrind]\n"
	exit -1
fi

server_host=127.0.0.1
port=$1
valgrind_mode=$2
max_clientes=100
server_cmd="./server"
client_cmd="./client"
valgrind_error_code=42

exit_code=0

function check_status() {
    module_name=$1
    status=$2
    echo -n "$module_name exit code: $status. "
    if [[ "$valgrind_mode" == "valgrind"  &&  $status -eq $valgrind_error_code ]]; then
        echo "ERROR: Valgrind error detected in $module_name execution."
        exit_code=6
    elif [[ $status -ne 0 ]]; then
        echo "ERROR: Unexpected result in $module_name execution."
        exit_code=7
    else
        echo "OK."
    fi
}

if [ "$valgrind_mode" == "valgrind" ]; then
	valgrind_prefix="valgrind --tool=memcheck --trace-children=yes --track-fds=yes --time-stamp=yes --num-callers=20 --error-exitcode=$valgrind_error_code --leak-check=full --leak-resolution=med"
	server_cmd="$valgrind_prefix --log-file=valgrind_server.out $server_cmd"
	client_cmd="$valgrind_prefix "'--log-file=valgrind_client$n.out'" $client_cmd"
fi


echo "Running with host:$server_host, port:$port, valgrind-mode:$valgrind_mode"

if [ -f "server.args" ]; then
	server_args=$(cat server.args)
fi
echo -n "Running Server: "
echo "$server_cmd $port $server_args <>server-stdin 1>__server_stdout__ 2>__server_stderr__  &"
$server_cmd $port $server_args 1>__server_stdout__ 2>__server_stderr__  & server_pid=$!
sleep 1
ps $server_pid > /dev/null
server_status=$? 
if [ $server_pid -gt 0 ] && [ $server_status -eq 0 ]; then
	echo "Server OK. PID: $server_pid"
else
	echo "ERROR: The server was not initialized. Server result code: $server_status"
	exit 1
fi

declare -a remote_status
for (( n=0; n < $max_clientes; n++ )); do
    if [ -f "netcat$n.in" ]; then	
        echo -n "Running Netcat $n: "
        echo "nc $server_host $port 0<netcat$n.in >netcat$n.out"
        eval nc $server_host $port 0<netcat$n.in >netcat$n.out
        remote_status[$n]=$?
    elif [ -f "client$n.in" ]; then
        echo -n "Running Client $n: "
        if [ -f "client$n.args" ]; then
            client_args=$(cat client$n.args)
        fi
        echo "$client_cmd $server_host $port $client_args 0<client$n.in >client$n.out"
        eval $client_cmd $server_host $port $client_args 0<client$n.in >client$n.out
        remote_status[$n]=$?
	fi
done

wait $server_pid
server_status=$?
check_status "Server" $server_status

for (( n=0; n < $max_clientes; n++ ));
do
    if [ -f "netcat$n.in" ] || [ -f "client$n.in" ]; then
        status=${remote_status[$n]}
        check_status "Client/Netcat $n" $status
    fi
done

exit $exit_code

