set terminal png
set output "Throughput.png" 
set title "Throughput-Flows plot "
set xlabel "No. of Flows" 
set ylabel "Throughput (Mbps)"

plot "throughput_dataSet.txt" using 1:2 with lines title "UL w/o RTS-CTS", \
     "throughput_dataSet.txt" using 1:3 with lines title "UL RTS-CTS", \
     "throughput_dataSet.txt" using 1:4 with lines title "DL w/o RTS-CTS", \
     "throughput_dataSet.txt" using 1:5 with lines title "DL RTS-CTS"


set terminal png
set output "Collision.png" 
set title "Collision-Flows plot "
set xlabel "No. of Flows" 
set ylabel "No. of Collisions"

plot "collision_dataSet.txt" using 1:2 with lines title "UL w/o RTS-CTS", \
     "collision_dataSet.txt" using 1:3 with lines title "UL RTS-CTS", \
     "collision_dataSet.txt" using 1:4 with lines title "DL w/o RTS-CTS", \
     "collision_dataSet.txt" using 1:5 with lines title "DL RTS-CTS"


set terminal png
set output "PacketDrop.png" 
set title "Packet Drops-Flows plot "
set xlabel "No. of Flows" 
set ylabel "No. of Packet Drops"

plot "drop_dataSet.txt" using 1:2 with lines title "UL w/o RTS-CTS", \
     "drop_dataSet.txt" using 1:3 with lines title "UL RTS-CTS"

