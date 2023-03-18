# PS-PSH
A primitive shell server for the PS4/PS5 using the mast1c0re exploit in the PS2-Emulator.

# Connecting to the Sever
The server does NOT implement the TELNET-Protocol correctly. Therefore it is recommended that you use a raw TCP connection to the server on port 9030.

Known working methods:
- (Windows) Use "[Putty](https://www.putty.org/)" and do NOT use any specified protocols and select RAW before connecting to the server.
- (Ubuntu) Use the the telnet command normally `telnet <IP_ADDRESS> 9030`
- (MacOs) Use the the telnet command normally `telnet <IP_ADDRESS> 9030`

# Available Commands
```
ls - Lists all folders and files in the current working directory
cd <Directory> - Change the current working directory
cp <Source Filepath> <Destination Filepath> - Copy a file
mv <Source Filepath> <Destination Filepath> - Move a file
rm <Filepath + Filename> - Delete a file
pwd - Print the current working directory
notification <Notification text> - Display a notification with your text
upload <Filepath + Filename> - Upload a file to the PS4/PS5 to the specified filepath
download <Filepath + Filename> - Download a file to the local machine
play <Filepath + Filename> <OPTIONAL: Config Filepath + Filename> - Play a PS2-ISO with or without a config file
exit - Terminate the server
```

To download a file from the server use the "[PS-PSH File-Receiver](https://github.com/aladie/ps-psh-file-receiver)"

Note: All commands requiring a filepath will also work with only a filename provided. -> The server will search for the file in the current working directory.

# Known Issues/Limitations
- You cannot write to all folders in the sandbox. -> Known and working: `/av_contents/content_tmp/`
- After downloading/uploading a file and directly trying to download/upload another file can fail. -> Workaround: wait a couple of seconds
- The server can crash after resending/restarting the server after exiting the server with the exit command -> Workaround: reopen OKAGE

# Next Steps
- (Download) Support the transmission of the filename and file extention
- (Download) Support the transmission of multiple files (Directories)
- Add more commands such as mkdir etc.
- The ability to copy/move directories


Thanks to @McCaulay for providing the mast1c0re sdk and samples.
