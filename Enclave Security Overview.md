Enclave Keychain
================
## Keychain Overview
Bob and Alice both have an Enclave Keychain, each Keychain is unique to each user (Bob and Alice).

Each fob handles message encryption and decryption along with sensitive key storage and truly random key generation.

If either Bob or Alice's phone is compromised, the attacker will not be able to decrypt packages cached on the device or taken from the API given that the keys are stored on the fob itself rather than the device.  

Both Bob and Alice's identity keys used for p2p encryption/decryption are derived from the fob itself and is unique to the fob.

### Phone-to-fob security
The communications between the Enclave Keychain fob and the users device is handled by the Bluetooth LE stack.
Although the BTLE stack already provides AES crypto between devices, Enclave adds a step of encryption by deriving a Curve25519 key between the device identity key and fob identity, the device key may change but the fob key will not in this case.

The key that is derived will be used to encrypt the package being sent to the fob with AES.

Note; Since the fob identity is unchanged throughout its use, the phone can verify its identity via HMAC and package signature validation.

### Message security
Alice creates a message package, this includes her public key for Bob's use. Alice encrypts this message with Bob's public key given to her when the conversation is first initialized. The package is then sent to the API server for temporary storage until Bob is ready to receive the package, once received the package is obliterated from the server.

Bob receives Alice's package, the package is stored in the mobile applications cache until Bob is ready to decrypt and read the message. When Bob initiates the decryption, the package is sent to the Enclave Keychain and is then decrypted with the fobs unique keys, once decrypted the fob encrypts a response package for Bob, this is returned to the application for decryption and plaintext reading by Bob.
