The current implementation is as follows,

- Routing Daemon: Supports TCP connections with multiple clients. It takes into account fragmented requests by checking that the length provided in the request matches the actual string length. Once it has determined method it then either finds the object locally or adds it to our file list.

- Flask Application: Communicates the ADDFILE and GETRD with the routing daemon. It then calls urlopen on the URI returned by the daemon.



