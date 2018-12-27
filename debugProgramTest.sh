# Kill gdbserver if it's runnin#
killall gdbserver &> /dev/null
# Compile myprogram and launch gdbserver, listening on port 9091
make && clear && gdbserver :9091 ./TestAtem 
