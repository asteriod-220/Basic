# Stream Engine Encryptor

A light weight command-line file encrypting tool built from scratch using C.

It features a custom polyalphabetic **Streamed Byte-Substution and Transposition Cipher** that destroyes data formatting

It is processed strictly in isolated 4KB block chunks, ensuring the program can encrypt large files while consuming virtually zero RAM

# Mathematical Architecture

The underlying cipher ensures that the output file remains the exact same size down to byre while completely breaking frequency analysis

## Encryption Pipeline:
1. **Mathematical shift:** "Shift = (Plaintext_byte + Key_Byte) % 256
2. **Circular rotation:** Rotate bits left by 3 position within a strict 8-bit boundary
3. **Bitwise Inversion:** "Final_Junk_Data = Rotated_Byte XOR (NOT Key_Byte)"

## Decryption Pipeline:
The decryption engine perfectly rewinds the clock by performing the exact inverse operations in reverse order

# Compilation

'''bash
Compile the source using gcc and youre good to go!
gcc encryptor.c -o encryptor
'''
