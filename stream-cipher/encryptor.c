#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

// Define the chunk size to 4KB
#define CHUNK_SIZE 4096

// Function to prompt the user for passowrd without displaying the text
size_t get_user_password(char *final_password, int decrypt){
    #include <termios.h>
    // Use the termios struct
    struct termios old, new;

    // Get the current terminal settings from STDIN
    if (tcgetattr(0, &old) < 0){
        // Write the error message if returncode is -1
        write(2, "Failed to read terminal settings\n", 33);
        // Exit with code 1
        exit(1);
    }

    // Copy the old settings to new target structure
    new = old;

    // Clear the ECHO bit
    // ~ECHO sets the ECHO bit to 0 and bitwise AND (&) removes it off our config
    new.c_lflag &= ~ECHO;

    // Apply the new settings
    if (tcsetattr(0, TCSAFLUSH, &new) < 0){
        // Write the error message if theres error
        write(2, "Failed to modify terminal attributes\n", 37);
        // Exit with code 1
        exit(1);
    }

    // Define the password
    char password[256], password2[256];

    // Defind the index counters
    size_t index = 0, index2 = 0;

    // Define the current character
    char current_character, current_character2;

    // Prompt the user to type password
    write(1, "Enter password: ", 16);

    // Read 1 by at a time until hitting \n
    // Read from STDIN 
    while ((read(0, &current_character, 1) > 0)){
        // Check if the user pressed enter
        if (current_character == '\n'){
            // Check if the pasword is empty
            if (strlen(password) < 0){
                // If password is empty then display the error message
                write(2, "Password cannot be empty\n", 25);
                // Exit with code 1
                exit(1);
            }
            password[index] = '\0';
            break;
        }
        password[index] = current_character;
        index++;
    }

    // Check if the mode is decryption mode
    if (decrypt == 1){
        // Turn ECHO back on
        tcsetattr(0, TCSAFLUSH, &old);
        // Copy the password to final_password byte by byte
        for (size_t i = 0; i <= index; i++){
            final_password[i] = password[i];
        }
        return index;
    }

    // Write a newline
    write(1, "\n", 1);
    write(1, "Enter password again: ", 22);
    // Prompt the user to  enter the password again
    while (read(0, &current_character2, 1) > 0){
        // Check if the user pressed enter
        if (current_character2 == '\n'){
            if (strlen(password2)< 0){
                // If password is empty then display the error message
                write(2, "Password cannot be empty\n", 25);
                // Exit with code 1
                exit(1);
            }
            // Replace \n with \0
            password2[index2] = '\0';
            // End the loop
            break;
        }
        // Add the current character the user typed to the i index for password2
        password2[index2] = current_character2;
        // Increment the index
        index2++;
    }
    
    // Turn ECHO back
    tcsetattr(0, TCSAFLUSH, &old);

    // Compare if both passwords are of equal length
    if (index != index2){
        // If theyre not equal display error message and exit with code 1
        write(1, "\n", 1);
        write(2, "Password does not match\n", 25);
        exit(1);
    }

    // Compare both password byte by byte
    for (size_t i = 0; i < index; i++){
        // Comparing password
        if (password[i] != password2[i]){
            // If a byte isnt equal write error message and exit with code 1
            write(1, "\n", 1);
            write(2, "Password does not match!\n", 25);
            exit(1);
        }
    }
    // Copy the password to final_password byte by byte
    for (size_t i = 0; i <= index; i++){
        final_password[i] = password[i];
    }
    // Return the length of password
    return index;
}

// Check if the incoming data is a file or data passed from a pipe
int check_file_or_pipe(char *data){
    /*
    // Check if the data is NULL before doing any operation
    if (data == NULL){
        // Write the error message
        // 2 for STDERR, Error message, Size of the error message
        write(2, "No files or pipe stream provided\n", 33);
    }
    */

    // Using stat to determine STDIN type
    struct stat stdin_stat;

    // Check the descriptor of fd (STDIN)
    if (fstat(0, &stdin_stat) < 0){
        // If the returncode is less then 0 then somethings wrong.
        // Write the error message
        write(2, "Error processing stdin\n", 23);
        // Terminate the program with returncode 1
        exit(1);
    }

    // Extract the file type bits
    if ((stdin_stat.st_mode & S_IFMT) == S_IFIFO){ // S_IFIFO is a pattern for a PIPE
        // Return 2 to indicate its a PIPE
        return 2;
    }
    else{
        // If it isnt a PIPE its a file,
        // Check if the file exists by trying to open it in read only mode
        int fd = open(data, O_RDONLY);
        
        // Check if there any error opening the file
        if (fd < 0){
            // Write the error message if yes
            write(2, "Error accessing file: ", 22);
            // Display the filename
            write(2, data, strlen(data));
            // Print a newline
            write(2, "\n", 1);
            // Exit the program with returncode -1 for error
            return -1;
        }
        // Close the file descriptor
        close(fd);
        // Return 3 to indicate its a file
        return 3;
    }
    // Return with error code -1 if its neither
    return -1;
}

// Function to scramble the data into junk
unsigned char scramble_data(unsigned char data, unsigned char passkey){
    // Add key value to shift the byte
    unsigned char shift = (data + passkey) % 256;

    // Shift left by 3, shift right by 5 and catch the bits and glue them
    unsigned char glued_bytes = (unsigned char)((shift << 3) | (shift >> 5));

    // XOR the inverted key to shatter repeating patterns
    unsigned char final_data = glued_bytes ^ (~passkey);

    // Return the scrambled data
    return final_data;
}


// Funtion to unscramble the junk data into original byte
unsigned char unscramble_data(unsigned char data, unsigned char passkey){
    // Undo the XOR inversion
    unsigned char xor_inversion = data ^ (~passkey);

    // Undo the shift left by 3
    unsigned char left_undo = (unsigned char)((xor_inversion >> 3) | (xor_inversion << 5));

    // Add 256 to ensure the number stays positive
    unsigned char final_data = (left_undo - passkey + 256) % 256;

    // Return the unscrambled data
    return final_data;
}

// Function to decrypt the data
void decrypt_data(char *data, char *passkey, size_t key_len, char *output_file, int mode){
    // Define the output mode and input mode
    int output = -1;
    int file = -1;
    
    // Check if the mode is [ 1 - PIPE, 2 - FILE ]
    if (mode == 1){
        // Set file variable to data if the given data is PIPED
        file = 0;
    }
    else{
        // Open the target file in read only mode
        file = open(data, O_RDONLY);
    }

    // Check if the output_fil
    // e is NULL
    if (output_file != NULL){
        // Open the output file and create if it doesnt exist and set the permission to 644
        output = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        // Check if there was any trouble opening the file
        if (output < 0){
            // If there was any error write the error message, write a newline and exit with code 1
            write(2, "Couldnt open: ", 14);
            write(2, output_file, strlen(output_file));
            write(2, "\n", 1);
            exit(1);
        }
    }
    else{
        // If the output_file is NULl set the output mode to STDOUT (1)
        output = 1;
    }

    // Set the buffer to 4KB
    char buffer[CHUNK_SIZE];
    // Make a counter for total bytes read and the key index
    ssize_t bytes_read = 0;
    size_t key_index = 0;

    // Loop through the file reading 4KB of data
    while ((bytes_read = read(file, buffer, CHUNK_SIZE)) > 0){
        // Point to the address of buffer
        unsigned char *scrambled_pointer = (unsigned char *)buffer; 

        // Loop through each byte in bytes_read
        for (ssize_t i =0; i < bytes_read; i++){
            // Feed the i index of scrambled_pointer to our scramble_data function
           scrambled_pointer[i] = unscramble_data(scrambled_pointer[i], (unsigned char)passkey[key_index]);
        
            // Increment the key index
            key_index++;
            // Check if key_index has been equal to key length
            if (key_index == key_len){
                // If it has. Then reset the key index to 0
                key_index = 0;
            }
        }
        // Write the scrambled 4KB data ro the output file
        if (write(output, buffer, bytes_read) != bytes_read){
            // If it couldnt write the entire data then display the error message, close the file and exit with code 1
            write(2, "Couldnt write data\n", 19);
            if (output != 1) close(output);
            if (mode == 2) close(file);
            exit(1);
        }
    }

    // Close the file after writing the data
    
    if (output_file != NULL){
        close(output);
    }
    if (mode == 2){
        close(file);
    }

}

// Function to encrypt the data and write to a STDOUT or a File if given
void encrypt_data(char *data, char *passkey, size_t key_len, char *output_file, int mode){
    // Define the output and the input mode
    int file = -1;
    int output = -1;

    // Check the mode 
    if (mode == 1){
        // Set file variable to data if the given data is PIPED
        file = 0;
    }
    else{
        // Open the target file in read only mode
        file = open(data, O_RDONLY);

    }

    // Check if the output_file is NULL
    if (output_file != NULL){
        // Open the output file and create if it doesnt exist and set the permission to 644
        output = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        // Check if there was any trouble opening the file
        if (output < 0){
            // If there was any error write the error message, write a newline and exit with code 1
            write(2, "Couldnt open: ", 14);
            write(2, output_file, strlen(output_file));
            write(2, "\n", 1);
            exit(1);
        }
    }
    else{
        // If the output_file is NULl set the output mode to STDOUT (1)
        output = 1;
    }

    // Set the buffer to 4KB
    char buffer[CHUNK_SIZE];
    // Make a counter for total bytes read and the key index
    ssize_t bytes_read = 0;
    size_t key_index = 0;

    // Loop through the file reading 4KB of data
    while ((bytes_read = read(file, buffer, CHUNK_SIZE)) > 0){
        // Point to the address of buffer
        unsigned char *scrambled_pointer = (unsigned char *)buffer; 

        // Loop through each byte in bytes_read
        for (ssize_t i =0; i < bytes_read; i++){
            // Feed the i index of scrambled_pointer to our scramble_data function
            scrambled_pointer[i] = scramble_data(scrambled_pointer[i], (unsigned char)passkey[key_index]);
        
            // Increment the key index
            key_index++;
            // Check if key_index has been equal to key length
            if (key_index == key_len){
                // If it has. Then reset the key index to 0
                key_index = 0;
            }
        }
        // Write the scrambled 4KB data ro the output file
        if (write(output, buffer, bytes_read) != bytes_read){
            // If it couldnt write the entire data then display the error message, close the file and exit with code 1
            write(2, "Couldnt write data\n", 19);
            if (output != 1) close(output);
            if (mode == 2) close(file);
            exit(1);
        }
    }
    // Close the file after writing the data
    
    if (output_file != NULL){
        close(output);
    }
    if (mode == 2){
        close(file);
    }
}

// The help message
void usage(){
    char *message = 
        "Usage: ./encryptor file -p password -o output.txt\n\n"
        "Flags:\n"
        "-p                    Enter the password\n"
        "-o                    Save the output to a file\n"
        "-d                    Decrypt the data\n"
        "-v                    Display version and exit\n";

    // Display the help message
    write(1, message, 233);
    // Exit with code 0
    exit(0);
}

// The Main Function
int main(int argc, char *argv[]){
    // Check if the theres anything in argc other then the program name
    if (argc == 1){
        // If argc is 1 display the help message
        usage();
    }
    // Define some variables
    int mode = 0;
    int decrypt = -1;
    char *version = "1.0.0";
    char *input = NULL;
    char pass[256];
    char *passkey = NULL;
    size_t key_length;
    size_t key_len = 0;
    char *output_file = NULL;

    // Loop through argv to see if flags are provided
    for (int i = 0; i < argc; i++){
        // Check for -v
        if (argv[i][0] == '-' && argv[i][1] == 'v' && argv[i][2] == '\0'){
            // Display the version
            write(1, "version ", 8);
            write(1, version, 5);
            write(1, "\n", 1);
            // Exit with code 0
            exit(0);
        }
        // Check if i index of argv is '-
        if (argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == '\0'){
            if (i+1 < argc){
                // Set output to [i + 1] index.
                output_file = argv[i+1];
                // Mark other slots empty so we can retrieve filename
                argv[i] = ""; // Mark empty
                argv[i+1] = ""; // Mark empty
                i++; // Skip whatevers next
                continue;
            }
            else{
                // If empty -o is provided show an error message and exit with code 1
                write(2, "-o: expected filename\n", 22);
                exit(1);
            }

        }
        // Get the return code of check_file_or_pipe function
        // Check for the -p flag
        if (argv[i][0] == '-' && argv[i][1] == 'p' && argv[i][2] == '\0'){
            if (i+1 < argc){
                passkey = argv[i+1]; // Get the passkey
                argv[i] = ""; // Mark empty
                argv[i + 1] = ""; // Mark empty
                i++;
                continue; 
            }
            else{
                // If -p is empty write an error and exit with code 1
                write(2, "-p: expected password\n", 22);
                exit(1);
            }
        }
        // Check for -d flag
        if (argv[i][0] == '-' && argv[i][1] == 'd' && argv[i][2] == '\0'){
            decrypt = 1; // Set decryption to yes
            argv[i] = ""; // Mark empty
            continue;
        }
    }

    // Loop to find the filename
    // Set i to 1 because argv[0] is the programs name
    for (int i = 1; i < argc; i++){
        if (argv[i][0] != '\0'){
            input = argv[i]; // Grab anything found
            break;
        }
    }
    
    // check wehther the input is a filename or a pipe
    int file_or_pipe = check_file_or_pipe(input);

    // Set mode to 1 if its PIPE and 2 if its a File
    if (file_or_pipe == 2) mode = 1;
    else if (file_or_pipe == 3) mode = 2;
    else return 1;

    // Check if the password was provided
    if (passkey == NULL){
        // If no passkey was provided prompt the user for the password and store it in 'pass' variable
        key_len = get_user_password(pass, decrypt);
        // Copy the password from pass to passkey
        passkey = pass;
    }
    else{
        // Calculate the length of passkey from command line
        while( passkey[key_len] != '\0'){
            key_len++;
        }
    }

    // Write a newline before calling the encrypt/decrypt function
    write(1, "\n", 1);

    // Check whether the user wants to encrypt or decrypt 
    if (decrypt == 1){
        // Encrypt the piped data
        decrypt_data(input, passkey, key_len, output_file, mode);
    }
    else{
        // Encrypt the data from file
        encrypt_data(input, pass, key_len, output_file, mode);
    }

    // Write a newline and finish the program with code 0
    write(1, "\n", 1);
    return 0;
}
    


