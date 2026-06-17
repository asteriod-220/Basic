#include <pwd.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Add some colors
#define RED     "\033[1;31m" // 7
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define UNDERL  "\033[4m" // 4
#define RESET   "\033[0m" // 4

// Set Protocol to TCP or UDP depending on what the user wants. Default = TCP
int sock_type(int type){
    int sock = -1;
    // If type = 0 then set socket to TCP mode
    if (type == 0){
        // Write the crafting message
        write(1, BLUE "[*] " RESET, 15);
        write(1, "Socket mode set to ", 19);
        write(1, GREEN "TCP\n" RESET, 15);
        // Craft the socket fd
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    // If type = 1 then set socket to UDP mode
    else if (type == 1){
        // Write the crafting message
        write(1, BLUE "[*] " RESET, 15);
        write(1, "Socket mode set to ", 19);
        write(1, GREEN "UDP\n" RESET, 15);
        // Craft the socket fd
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    else{
        write(2, RED "[!] " RESET, 15);
        write(2, "Invalid mode!\n", 14);
        exit(1);
    }
    // Check if there was error in initializing socket
    if (sock < 0){
        // Write the error message
        write(2, RED "[!] " RESET, 15);
        write(2, "Error while initializing socket\n", 31);
        // Exit with returncode 1
        exit(1);
    } 
    // Return the socket fd
    return sock;
}

// Function to check wether the file is accessible or not
void check_file(char *file){
    // Write some messages to the screen
    write(1, YELLOW "[+] " RESET, 15);
    write(1, "Checking file: " UNDERL, 19);
    write(1, file, strlen(file));
    write(1, RESET "\n", 5);

    // Check if the file is accessible
    int fd = open(file, O_RDONLY);
    // Check the return code
    if (fd < 0){
        // If theres trouble accessing file check for the file ownership
        struct stat file_ow;
        // Use stat() to get the files metadata
        if (stat(file, &file_ow) < 0){
            // If we couldnt get the metadata then display the error message and exit with code 1
            write(2, RED "[!] " RESET, 15);
            write(2, "Could not get the files metadata\n", 33);
            exit(1);
        }

        // Translate the file ID into text name
        struct passwd *pw = getpwuid(file_ow.st_uid);
        // If the file isnt accessible but the file is owned by the user then inform the user about it
        write(2, YELLOW "[-] " RESET, 15);
        write(2, "File is owned by user: ", 23);
        write(2, UNDERL, 4);
        write(2, pw->pw_name, strlen(pw->pw_name));
        write(2, RESET , 4);
        write(2, " and does not have read permission\n", 33);
        exit(1);
    }
    // If file is accessible then write success response
    write(1, GREEN "[+] " RESET, 15);
    write(1, "File: ", 6);
    write(1, GREEN, 7);
    write(1, file, strlen(file));
    write(1, RESET " is accessible\n", 19);
}

void tcp_handler(int sock, char *address, int port, int timeout){
    // Declare the structure variable and name it target_addr
    struct sockaddr_in target_addr;

    // Clear the memory block to remove any existing stack garbage
    memset(&target_addr, 0, sizeof(target_addr));

    // Set the Address Family Type to IPv4
    target_addr.sin_family = AF_INET;

    // Set the Target Port
    target_addr.sin_port = htons(port);

    // Use inet_pton to pack the address and port into sin_addr
    if (inet_pton(AF_INET, address, &target_addr.sin_addr) <= 0){
        // If we face error. let the user know about it and exit with code 1
        write(2, RED "[!] " RESET "Failed to configure the destination\n", 51);
        // Close the socket fd
        close(sock);
        exit(1);
    }

    write(1, BLUE "[*] " RESET, 15);
    write(1, "Target address set to: " GREEN, 30);
    write(1, address, strlen(address));
    write(1, RESET ":" GREEN, 12);

    // Get the size of the port
    char buffer[12]; // Large enough to hold any 32-bit integer string representation
    int i = 0;

    int temp_port = port;

    if (temp_port == 0) {
        buffer[i++] = '0';
    } else {
        while (temp_port > 0) {
            buffer[i++] = (temp_port % 10) + '0'; 
            temp_port /= 10;                      
        }
    }

    for (int j = i - 1; j >= 0; j--) {
        write(1, &buffer[j], 1);
    }

    // Set the Timeout
    struct timeval timeot;
    timeot.tv_sec = timeout;  // 3 seconds
    timeot.tv_usec = 0; // 0 microseconds

    // Apply the timeout setting directly to the socket descriptors transmission line (SO_SNDTIMEO)
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeot, sizeof(timeot));

    write(1, "\n" RESET, 5);

    // Check if the server is accepting TCP connection
    int connect_response = connect(sock, (struct sockaddr *)&target_addr, sizeof(target_addr));
    // Check for response code
    if (connect_response < 0){
        // Check for the error type
        if (errno == ECONNREFUSED){ // Conn Refused Error
            write(2, RED "[!] Connection Refused: " RESET, 31);
            write(2, "The Server is online but the port seems closed\n", 47);
        }
        else if (errno == ETIMEDOUT){ // In case of timeout
            write(2, RED "[!] " RESET, 15);
            write(2, "Connection Timed Out: ", 22);
            write(2, "The Server is unreachable or a firewall is active\n", 50);
        }
        else if (errno == ENETUNREACH){ // Network unreachable
            write(2, RED "[!] " RESET, 15);
            write(2, "Network Unreachable: ", 21);
            write(2, "Unable to connect to the internet\n", 34);
        }
        else{ // In case of unknown error
            write(2, RED "[!] " RESET, 15);
            write(2, "Failed to Connect: ", 19);
            write(2, "An unexpected network error occurred.\n", 38);
        }
    // Close the active socket fd
    close(sock);
    // Exit with code 1
    exit(1);
    }
    
    // If there was no error print the success message
    write(1, BLUE "[*] " GREEN, 18);
    write(1, "Connected " RESET, 14);
    write(1, "to the server\n", 14);
}

// THE TCP DATA TRANSFER PART
void send_data_tcp(char *data, int sock, int isfile){
    // Check if the data is empty
    if (data == NULL) return; // Return from function if yes
   
    // Check if the data is a file
    if (isfile == 1){
        // Open the file 
        int fd = open(data, O_RDONLY);
        // Make the counter to keep track of how many bytes weve read
        ssize_t bytes_rd = 0;
        // Make a buffer of size 4KB to hold the file data
        char buffer[4096];

        // Read the file in chunks of 4kb until we reach EOF
        while ((bytes_rd = read(fd, buffer, 4096)) > 0){
            // Make a counter for sent and received blocks
            size_t total_sent = 0;
            ssize_t sent_block = 0;
        
            // Ensure the entire 4KB data is send
            while (total_sent < bytes_rd){
                sent_block = send(sock, buffer + total_sent, bytes_rd - total_sent, 0);
            
                if (sent_block < 0) {
                    write(2, RED "[!] Error: Connection broken mid-transfer\n" RESET, 53);
                    close(fd);
                    return;
                }
                total_sent += sent_block;
            }
        }
        close(fd);
        write(1, GREEN "[+] " RESET "File transmission completed\n", 43);
    }
    else{
        // If the data isnt a file then its a string
         size_t data_length = strlen(data);
         size_t total_sent = 0;
         ssize_t sent_block = 0;

         while (total_sent < data_length){
            sent_block = send(sock, data + total_sent, data_length - total_sent, 0);

            if (sent_block < 0){
                write(2, RED "[!] Error: Connection broken mid-transfer\n" RESET, 53);
                return;
            }
            total_sent += sent_block;
        }
        write(1, GREEN "[+] " RESET "Data transmission successful\n", 44);
        return;
    }
}

// Functiont to send data using UDP protocol
void send_data_udp(char *data, int sock, int isfile, struct sockaddr_in *target_addr){
    // Check data
    if (data==NULL) return;

    // Check for file
    if (isfile == 1){
        int fd = open(data, O_RDONLY);
    
        ssize_t read_bytes = 0;
        char buffer[4096];
        // Let the user know the file transfer has started
        write(1, BLUE "[*] " RESET, 15);
        write(1, "File Transfer Started...\n", 25);
        // Start reading the file
        while((read_bytes = read(fd, buffer, 4096)) > 0){
            /*
            // Send the Entire file in 1 shot
            sendto(sock, buffer, read_bytes, 0, (struct sockaddr *)target_addr, sizeof(struct sockaddr_in));
            */
            // Start sending the data and check for error
            if (sendto(sock, buffer, read_bytes, 0, (struct sockaddr *)target_addr, sizeof(struct sockaddr_in)) < 0){
                write(2, RED "[!] " RESET, 15);
                write(2, "Error: UDP stream broken\n", 25);
                close(sock);
                exit(1);
            }
            // Pause every 500 microseconds before sending another 4KB chunk of data
            usleep(500);
        }
    }
    else{
        // Get the length of data
        size_t length_of_data = strlen(data);

        // Send the entire data at once

        sendto(sock, data, length_of_data, 0, (struct sockaddr *)target_addr, sizeof(struct sockaddr_in));
    }
    write(1, GREEN "[+] " RESET,15);
    write(1, "Sent payload: " GREEN,21);
    write(1, data, strlen(data));
    write(1, RESET "\n", 5);

    return;
}

void check_host_up(char *ip_addr, int timeout, int port){
    int probe = socket(AF_INET, SOCK_STREAM, 0);
    if (probe < 0){
        write(2, RED "[-] Failed to create Socket\n" RESET, 39);
        exit(1);
    }

    // Make the timeout
    struct timeval watch;
    if (timeout < 0){
        // Create a 3 second timeout if the user didnt give any
        watch.tv_sec = 3;
        watch.tv_usec = 0;
        // Set the timer in socket
        setsockopt(probe, SOL_SOCKET, SO_SNDTIMEO, &watch, sizeof(watch));
    }
    else{
        // Else set the timeout given by the user
        watch.tv_sec = timeout;
        watch.tv_usec = 0;
        // Set the timer in socket
        setsockopt(probe, SOL_SOCKET, SO_SNDTIMEO, &watch, sizeof(watch));
    }

    // Create the socket struct
    struct sockaddr_in sockaddr;

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip_addr, &sockaddr.sin_addr);

    // Try connecting to the given addr
    int res = connect(probe, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    write(1, BLUE "[*] " RESET, 15);
    write(1, "Sent SYN packet\n", 16);

    // Close the socket
    close(probe);

    // Check if the host is up
    if (res == 0){
        write(1, BLUE "[*] " RESET, 15);
        write(1, "Received SIN-ACK. Target Active\n", 32);
        return;
    }
    // Host up but connection refused
    else if (errno == ECONNREFUSED){
        write(1, YELLOW "[-] " RESET, 15);
        write(1, "Target is up but refused to connect\n", 37);
        return;
    }
    
    // Check for other errors

    // Timeout error
    else if (errno == ETIMEDOUT){
        write(2, YELLOW "[-] " RESET, 15);
        write(2, "The packet was dropped. Maybe theres an active Firewall\n", 56);
        exit(1);
    }
    // The target is down
    else if (errno == EHOSTUNREACH){
        write(2, RED "[!] " RESET, 15);
        write(2, "Target down\n", 12);
        exit(1);
    }
    // Internet problem
    else if( errno == ENETUNREACH){
        write(2, RED "[!] " RESET, 15);
        write(2, "No internet connection\n", 23);
        exit(1);
    }
    else{
        write(2, RED "[!] " RESET, 15);
        write(2, "No Response\n", 12);
        exit(1);
    }
}

// The help function
void func_help(){
    char *help = "Usage: ./socket -i <ip_addr> -p <port> [options]\n"
                 "  -f                             Send a file\n"
                 "  -t                             Send a raw string payload\n"
                 "  -u                             Swtich to UDP mode (default=TCP)\n"
                 "  -i <addr>                      IP address of target\n"
                 "  -p <port>                      Port number of target (default=80)\n"
                 "  -T                             Timeout in seconds (default=3s)\n"
                 "  -c                             Check if the target is up\n";
    write(1, help, strlen(help));
}

// The Main function
int main(int argc, char *argv[]){

    char *ip_address = NULL;
    char *data = NULL;
    int check_and_exit = 0;
    int mode = 0;
    int isfile = 0;
    int port = 80;
    int timeout_s = 3;

    if (argc < 2){
        func_help();
        return 0;
    }

    // Loop through argv to check for flags
    // Start loop from i=1 to avoid looping through the filename which is argv[0]
    for (int i = 1; argc > i; i++){
        // Parse the IP addr
        if (argv[i][0] == '-' && argv[i][1] == 'i' && i+1 < argc){
            ip_address = argv[i + 1]; // Set the IP address
            i++; // Increment i
            continue;
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'p' && i+1 < argc){
            port = atoi(argv[i+1]); // Convert to int and set port
            i++;
            continue;
        }
        else if (argv[i][0] == '-' && argv[i][1] == 't' && i+1 < argc){
            data = argv[i+1]; // Save the text data
            isfile = 0; // Set to text mode to avoid file checks
            i++;
            continue;
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'u'){
            mode = 1;; // Set to UDP mode
            continue;
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'f' && i+1 < argc){
            data = argv[i+1]; // Set to file mode
            isfile = 1;
            i++;
            continue;
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'T' && i+1 < argc){
            timeout_s = atoi(argv[i+1]); // Set timeout
            i++;
            continue;
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'c'){
            check_and_exit = 1; // Set Check and exit to YES
            continue;
        }
    }

    // Check if ip address was provided
    if (ip_address == NULL){
        write(2, RED "[!] " RESET, 15);
        write(2, "No IP Address specified\n", 24);
        return 1;
    }

    if (check_and_exit == 1){
        check_host_up(ip_address, timeout_s, port);
        exit(0);
    }

    // Check if the file is accessible if given
    if (isfile == 1){
        check_file(data);
    }

    // Check if the user intends to use UDP mode
    if (mode == 1){
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        // Create a address frame for UDP
        struct sockaddr_in udp_targ;
        // Clear the memory to wipe stack garbage
        memset(&udp_targ, 0, sizeof(udp_targ));

        // Configure the parameters
        udp_targ.sin_family = AF_INET;
        udp_targ.sin_port = htons(port);
        inet_pton(AF_INET, ip_address, &udp_targ.sin_addr);

        // Send the data or file using UDP protocol
        send_data_udp(data, sock, isfile, &udp_targ);
        close(sock);
    }
    else{
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        tcp_handler(sock, ip_address, port, timeout_s);
        send_data_tcp(data, sock, isfile);

        // Gracefully stop the tramission and wait for buffer to drain
        shutdown(sock, SHUT_WR);

        // Wait 50ms to let kernel flush remaining data chunks
        usleep(5000);
        close(sock);
    }
    
    return 0;
}
