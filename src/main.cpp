#include <mast1c0re.hpp>
#include "execute/Execute.hpp"

#define SERVER_PORT 9030
#define DOWNLOADER_PORT 9045
#define UPLOADER_PORT 9046


void main() {
#if (defined(PS4) && PS4)
    const char* system = "PS4";
#elif (defined(PS5) && PS5)
    const char* system = "PS5";
#endif

    // Create server
    PS::TcpServer server = PS::TcpServer();

    // Listen
    if (server.listen(SERVER_PORT)) {
        PS::notification("Listening on port %i", SERVER_PORT);

        // Accept connection
        PS::TcpClient client = server.accept();

        // Send messages
        client.printf("Welcome to PS-PSH! v0.1.5\r\n");
        client.printf("Twitter: twitter.com/aladie11\r\n\r\n");

        //Set initial directory
        char directory[256] = "/";

        // Receive message
        char buffer[2] = "";
        char buildCommand[256] = "";
        char buildParam1[256] = "";
        char buildParam2[256] = "";
        int selectBuild = 0;
        bool cleanTask = false;
        bool part1 = false;

        //Print current directory
        client.printf("PS-PSH@%s: ", system);
        client.printf(directory);
        client.printf(" $");

        while (client.isConnected()) {
            //Reset the stored commands and parameters after any command
            if (cleanTask) {
                memset(buildCommand, 0, sizeof(buildCommand));
                memset(buildParam1, 0, sizeof(buildParam1));
                memset(buildParam2, 0, sizeof(buildParam2));
                memset(buffer, 0, sizeof(buffer));
                cleanTask = false;
            }

            //Receive user input in single characters
            memset(buffer, 0, sizeof(buffer));
            int received = client.read(buffer, 1);

            if (received > 0) {
                //Check for the user hitting ENTER -> CR LF
                if (buffer[0] == '\r') {
                    part1 = true;
                } else if (part1 && buffer[0] == '\n') {
                    part1 = false;

                    //Check if the command contains anything
                    if (PS2::strcmp(buildCommand, "") != 0) {
                        //Check user input for a valid command
                        if (PS2::strcmp(buildCommand, "ls") == 0) {
                            Execute::ls(client, directory);
                        } else if (PS2::strcmp(buildCommand, "notification") == 0) {
                            Execute::notification(client, buildParam1);
                        } else if (PS2::strcmp(buildCommand, "upload") == 0) {
                            Execute::upload(client, directory, buildParam1, DOWNLOADER_PORT);
                        } else if (PS2::strcmp(buildCommand, "download") == 0) {
                            Execute::download(client, directory, buildParam1, UPLOADER_PORT);
                        } else if (PS2::strcmp(buildCommand, "play") == 0) {
                            Execute::play(client, directory, buildParam1, buildParam2);
                        } else if (PS2::strcmp(buildCommand, "pwd") == 0) {
                            Execute::pwd(client, directory);
                        } else if (PS2::strcmp(buildCommand, "rm") == 0) {
                            Execute::rm(client, directory, buildParam1, buildParam2);
                        } else if (PS2::strcmp(buildCommand, "cd") == 0) {
                            Execute::cd(client, directory, buildParam1);
                        } else if (PS2::strcmp(buildCommand, "cp") == 0) {
                            Execute::cp_or_mv(client, directory, buildParam1, buildParam2, false);
                        } else if (PS2::strcmp(buildCommand, "mv") == 0) {
                            Execute::cp_or_mv(client, directory, buildParam1, buildParam2, true);
                        } else if (PS2::strcmp(buildCommand, "mkdir") == 0) {
                            Execute::mkdir(client, directory, buildParam1);
                        } else if (PS2::strcmp(buildCommand, "help") == 0) {
                            Execute::help(client);
                        } else if (PS2::strcmp(buildCommand, "exit") == 0) {
                            client.printf("Goodbye.\r\n");

                            //Disconnect client
                            client.disconnect();
                        } else {
                            client.printf("Unknown Command: ");
                            client.printf(buildCommand);
                            client.printf("\r\n");
                            client.printf("Type help to see a list of all available commands\r\n");
                        }

                        cleanTask = true;
                        selectBuild = 0;
                    }

                    //Print current directory
                    client.printf("PS-PSH@%s: ", system);
                    client.printf(directory);
                    client.printf(" $");
                } else if (PS2::strcmp(buffer, " ") == 0) {
                    //Switch to command parameters
                    selectBuild++;
                } else if (selectBuild == 0) {
                    //Build the command
                    PS2::strcat(buildCommand, buffer);
                } else if (selectBuild == 1) {
                    //Build the first parameter
                    PS2::strcat(buildParam1, buffer);
                } else {
                    //Build the second parameter
                    PS2::strcat(buildParam2, buffer);
                }
            }
            //Exit loop
            received = 0;
        }
        //Disconnect server
        server.disconnect();
    } else
        PS::notification("Failed to listen on port %i, wait 1 minute to try again.", SERVER_PORT);
}
