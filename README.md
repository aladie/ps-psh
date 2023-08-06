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
cp <Source Path> <Destination Path> - Copy a file or directory
mv <Source Path> <Destination Path> - Move a file or directory
rm <OPTIONAL: -r> <Filepath + Filename> - Delete a file or directory
mkdir - <Path + Name> - Creates a new directory
pwd - Print the current working directory
notification <Notification text> - Display a notification with your text
upload <Filepath + Filename> - Upload a file to the PS4/PS5 to the specified filepath
download <File-/Directorypath + File-/Directoryname> - Download a file/directory to the local machine
play <Filepath + Filename> <OPTIONAL: Config Filepath + Filename> - Play a PS2-ISO with or without a config file
exit - Terminate the server
```

To download a file from the server use the "[PS-PSH File-Receiver](https://github.com/aladie/ps-psh-file-receiver)"

Note: All commands requiring a filepath will also work with only a filename provided. -> The server will search for the file in the current working directory.

# Directories with write privileges
Most of the directories are sadly read-only. However there are some known and working directories:

- `/av_contents/content_tmp/` _Will be automatically cleared after closing the game._
- `/temp0/` _The contents of this directory will not be deleted after closing the game or restarting the console._

# Known Issues/Limitations
- After downloading/uploading a file and directly trying to download/upload another file can fail. -> Workaround: wait a couple of seconds
- The server can crash after resending/restarting the server after exiting the server with the exit command -> Workaround: reopen OKAGE

# Next Steps
- (Download) Improve the currently experimental state of downloading directories
- Add more commands


Thanks to @McCaulay for providing the mast1c0re sdk and samples.
