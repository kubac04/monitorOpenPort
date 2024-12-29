# MonitorOpenPort
Script to monitor server ports in C.
# How it works?
Daemon, which allows you to monitor changes in the state of network ports
within a specified time range. It works by regularly comparing
the current state of the ports with the state 60 seconds ago. If an opening or
port closure, Daemon automatically logs this information. This solution
allows for ongoing monitoring of network activity and analysis of changing states
which can help identify unusual behavior or potential threats.
