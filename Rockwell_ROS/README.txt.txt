ros_client.cpp is the ros script, check comments in code for more detail

runs a sine wave script, which outputs float values

these float values are saved into an ASCII array

the ASCII array is written to a server socket from a client socket 

coding on the controller side is needed to read from the client and to write back

this has already been done in rockwell_ros_server.acd file, but more effort needs to be put in to make it more automated

