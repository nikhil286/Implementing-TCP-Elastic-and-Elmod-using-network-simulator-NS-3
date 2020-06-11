# Implementing-TCP-Elastic-and-Elmod-using-network-simulator-NS-3
This project includes the implementation on TCP congestion control algorithm Elastic, and a new CCA Elmod on NS3.

NOTE:- Steps to be followed-
1) To implement TCP Elastic, copy the eval2.cc file in scratch folder.

2) Place elastic.cc and elastic.h in "ns-3.30 -> src -> internet -> model". Then go to internet folder -> Wscript. Do the following          changes-Line 164-> Type ‘model/Tcp-elastic.cc’ Line 402-> Type ‘model/tcp-elastic.h’ and save the file. 

3) Go to the folder ns-3.30 and open terminal there. Type the following command to run:

                                    ./waf --run scratch/eval2.cc
                                
After it runs successfully, pcap files for ELASTIC are generated along with plt and cwnd files. To plot the graph, open terminal and type gnuplot tcpelastic.plt.

To Implement Elmod, follow the steps 2,3.

4) Use "my-tcp-variants-comparison.cc" file for comparative analysis of elastic and elmod with older TCP variants.
