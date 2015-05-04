#define _USE_MATH_DEFINES

#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <std_msgs/Float32.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_FLOAT_LEN 32
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// make sine wave a method if desired
/*float motorMove(long double omega) {
    double a = 1.0;
    double t = 0.0;
    double dt = 1.0;
    double x_des = a*sin(omega * t);
    t+=dt;
    return x_des;
}*/

int main(int argc, char **argv) {

    // name of this node will be "vel_scheduler"
    ros::init(argc, argv, "vel_scheduler"); 

    // get a ros nodehandle; standard
    ros::NodeHandle nh; 

    // create a publisher object that can talk to ROS and issue twist messages on named topic;
    // note: this is customized for stdr robot; 
    ros::Publisher pub  = nh.advertise<std_msgs::Float32>("joint_angle_command", 1);
    
    // x = a*sin(omega*t)
    long double omega = 3.14;
    double a = 15.0;
    double t = 0.0;

    // incrementing t 
    double dt = 0.1; 

    //allow callbacks to populate fresh data
    ros::spinOnce(); 


    int sockfd, portno, n;

    // name of server is it's IP Address
    char *name = "192.168.20.5";
    struct sockaddr_in serv_addr;  // structure to contain an IP Address
    struct hostent *server;

    // creating string ASCII
    char floatArray[MAX_FLOAT_LEN];

    // Create socket for client
    ROS_INFO("about to create socket");

    portno = 10002;  // port number of server

    // error displayed if this fails
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // standard, define socket file descriptor, 
                                               //always use 0 for last arugment
    if (sockfd < 0)  // fails if socket call returns -1
        error("ERROR opening socket");
    server = gethostbyname(name);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    // setting fields in serv_addr (address of server)
    memset((char *) &serv_addr,0, sizeof(serv_addr)); // sets bytes in memory to zero, 
                                                      // initializes serv_addr to zeros
    serv_addr.sin_family = AF_INET; // always set to AF_INET

    // set IP address to serv_addr, use bcopy because server->h_addr is a character string
    bcopy((char *)server->h_addr,  // h_addr = the first address in the array of network addresses
         (char *)&serv_addr.sin_addr.s_addr,  
         server->h_length);
    serv_addr.sin_port = htons(portno); // set port number of server, converted to network byte order

    // client establish connection to server (socket descriptor, address, size of the address)
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    // do work here in infinite loop (desired for this example), 
    // but terminate if detect ROS has faulted (or ctl-C)
    while (ros::ok()) 
    { 
        // publishing x = a*sin(omega * t)
        std_msgs::Float32 ros_float;
        double x_des = a*sin(omega * t);
        t+=dt;
        ros_float.data = x_des;
        pub.publish(ros_float);
        
        // read and write
        ROS_INFO("about to read and write");

        // setting bytes in memory for string ASCII to 0
        memset(floatArray, 0, sizeof(floatArray));  
        snprintf(floatArray,sizeof(floatArray),"%f",ros_float.data);
        ROS_INFO("floatArray %s", floatArray);

        // send string ASCII to server, n = 1 if successful
        n = send(sockfd,floatArray, strlen(floatArray), 0);
        if (n < 0)
             error("ERROR writing to socket");

        ROS_INFO("move sent to controller");

        // here, enable client to receive data from server
        /*n = read(sockfd,floatArray,255);
        if (n < 0)
             error("ERROR reading from socket");
        printf("%s\n",floatArray);*/

        ros::Duration(1).sleep();

    }  
    // done with reading and writing to controller
    ROS_INFO("completed move distance");

    // Close client socket
    close(sockfd);

    return 0;
}

