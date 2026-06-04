#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>


// Create a function that checks whether the file is accessible
// void function doesnt return anything
void check_file_exists(char *file){
    // Try opening the file in read only mode and store the file descriptor
    int fd = open(file, O_RDONLY);
    
    // Check if the return code for fd is -1. Which means error
    if (fd == -1){
        // If theres error accessing the file write it to the screen
        // 2 for STDERR, The error message, Size of the error message. In this case its 23 bytes long
        write(2, "Error accessing file: \n", 23);
        // Write the name of the file
        write(2, file, strlen(file));
        // Print a newline (\n)
        write(2, "\n", 1);
        // Exit the program with code 1 which means error
        exit(1);
    }
    // If theres no error opening the file close it
    close(fd);
}

// Create a function that reads the file. Store the data in a heap and returns it
char *read_file(char *file, long *size){
    // Open the file in read only mode
    int fd = open(file, O_RDONLY);

    // Open the stat and name it as info
    struct stat info;

    // Check if theres any error in opening the file and writing the info
    if (fstat(fd, &info) == -1){
        // Write the error message
        write(2, "Unable to determine file size\n", 30);
        // Close the file
        close(fd);
        // Return from the function by returning NULL
        return NULL;
    }

    // Check the size of the file
    long file_size = info.st_size;
    // Save the size of the file
    *size = file_size;

    // Allocate memory on the heap with memory size of the file + 1 for \0
    char *buffer = malloc(file_size + 1);

    // Check if the buffer is empty
    if (buffer == NULL){
        // Write the error message
        write(2, "Memory allocation error\n", 24);
        // Close the file
        close(fd);
        // Exit function
        return NULL;
    }

    // Create a counter for how many bytes read
    long bytes_read = 0;
    // Create a integer to hold file descriptor
    int read_bytes;

    // Create a loop to read all the content of the file
    // Store the data into the buffer created before
    // 0 Means EOF
    while ((read_bytes = read(fd, buffer, 4096)) > 0){
        // Increase the counter with the number of bytes read
        bytes_read = bytes_read + read_bytes;
    }

    // Add \0 at the end of the buffer to mark termination of the string
    // Use '' for single character
    buffer[bytes_read] = '\0';

    // Close the file
    close(fd);

    // Return the buffer
    return buffer;
}

// Create a void function to convert and print hex
// Use unsigned char as regular char can sometimes handle sign-extension negatively
void convert_to_hex_and_print(unsigned char bytesx){
    // Define the valid hex characters
    char *alphabets = "0123456789abcdef";

    /*
        * Divide the 8 bytes into 4, 4 bits
        * Shift the 4 left bits into right and isolate the left side
        * For this we will use the Bitwise right shift operator (>>).
        
    */

    // Shift bytes by 4 places
    int left_side = bytesx >> 4;

    /*
        * Were using Bitwise And Operator (&) along with Data Mask (0x0F)
        * Hex 0 in binary means 0000 and F means 15 in deciman and 1111 in binary
        * So 0x0F means 15 in deciman and 1111 in binary
        
        * & operator is used to compare two numbers bit-by-bit
        * The & operator matches the number up vertically. Example: 0 0 0 0
                                                                  : 1 1 1 1
        
        * If both the upper side and lower side are 1. The result will be 1 but if either is 0 the result will be 0.
          Example: 0 1 1 0 :
                 : 1 1 0 0 :
                 > 0 1 0 0 <

    */

    // Create the right side of the bits and isolate the low 4 bits using bitwise and mask
    int right_side = bytesx & 0x0F;

    // Use our calculated numeric indexes to grab the visual character symbols
    
    // Grab the left side
    char left_character = alphabets[left_side];
    // Now grab the right side
    char right_character = alphabets[right_side];

    // Write the left side of the hex (1 digit)
    // 1 for STDOUT and & to point at the memory address of the variable
    write(1, &left_character, 1);

    // Write the right side now
    write(1, &right_character, 1);

    // Write a empty space so the hex doesnt budge together
    write(1 ," ", 1);
}

// The Main function

/* 
    * argc contains how many arguments were passed while executing the program
        Example: ./hex file.txt x y.    Here (./hex, file.txt, x, y) adds up to 4 so argc will be 4.
    * argv means the arguments stored in a list
        Example: ./hex file.txt x y.    Here argv will be ["hex", "file.txt", "x", "y"]
*/

// Function to display the welcome message
void welcome(char *name_of_program, char *filename){
    // Write the Welcome message
    write(1, "Welcome to ", 11);
    // Write the filename
    write(1, name_of_program, strlen(name_of_program));
    // Write a newline
    write(1, "\n", 1);
    // Write the text seperating the hex from welcome message
    write(1, "Content of file: ", 17);
    write(1, filename, strlen(filename));
    write(1, "\n\n", 2);
}

// The main function
int main(int argc, char *argv[]){
    // Define the program name and the filename

    // argv[0] is always the name of the program
    char *prog_name = argv[0];

    // argv[1] is the expected filename
    char *file = argv[1];

    // Check if the arguments were given
    if (argc < 2){
        // If the arguments were not given display the usage help and exit
        write(2, "Usage: ", 7);
        write(2, prog_name, strlen(prog_name));
        write(2, " [source]", 9);
        write(2, "\n", 1);
        return 1;
    }

    // Define the counter
    int counter = 0;
    // Define the file size
    long file_size = 0;

/*
    * The arguments after first and second index will be ignored
    * The second argument can be anything it is just used to denote whether to show welcome message or not
*/

    // First check if the file exists and does it have read permission
    // We will achieve this by using the check_file_exists(char *file) function
    check_file_exists(file);

    // Now check if the third argument is given. If yes we will not display the welcome message
    // Check if the argc count is greater then 3.
    if (argc > 2){
        // If yes print the welcome message
        welcome(prog_name, file);
    }
    else{
        // If no write a newline
        write(1, "\n", 1);
    }

    // Run the read_file() function and store the returned value in a variable
    char *file_data = read_file(file, &file_size); 

    // Check if the read_file funtion returned any error
    if (file_data == NULL){
        // If yes. Exit the program
        return 1;
    }
    
/*
    * Use a for loop to feed every bit to convert_to_hex_and_print() function.
    * Use the counter to count every time a loop cycle is completed.
    * If the cycle reaches 16. Write a newline to make the output look clean
*/
    for (int i = 0; i < file_size; i++){
        // Feed the i index of the files data to the function
        convert_to_hex_and_print((unsigned char)file_data[i]);
        // Increment the counter by 1
        counter++;
        // Check if the counter reached 16
        if (counter == 16){
            // If yes write a newline
            write(1, "\n", 1);
            // Set the counter back to 0
            counter = 0;
        }
    }
    // Free the heap memory we reserved in variable buffer using malloc() to prevent memory leaks
    free(file_data);

    // Finally write a newline and exit with returncode 0
    write(1, "\n", 1);
    return 0;
}
