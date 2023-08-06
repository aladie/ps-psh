#include "Execute.hpp"
#include "../downloader/Downloader.hpp"
#include "../uploader/Uploader.hpp"
#include "../helper/Helper.hpp"

void Execute::ls(PS::TcpClient client, const char *directory) {
    // Print directories/files
    int fd = PS::open(directory, O_RDONLY, 0);

    if (fd > 0) {
        char buffer[1024];
        int nread = 0;
        while ((nread = PS::getdents(fd, buffer, sizeof(buffer))) > 0) {

            struct dirent *dent = (struct dirent *) buffer;

            while (dent->d_fileno) {

                if (dent->d_reclen == 0)
                    break;
                nread -= dent->d_reclen;
                if (nread < 0)
                    break;
                if (dent->d_type == DT_DIR)
                    client.printf("d %s%s\r\n", directory, dent->d_name);
                if (dent->d_type == DT_REG)
                    client.printf("- %s%s\r\n", directory, dent->d_name);
                dent = (struct dirent *) ((char *) dent + dent->d_reclen);

            }

            memset(buffer, 0, sizeof(buffer));
        }
    }

    PS::close(fd);
}

void Execute::notification(PS::TcpClient client, const char *notification) {
    if (PS2::strcmp(notification, "") == 0) {
        client.printf("Syntax: notification \"Your message here\"");
        client.printf("\r\n");
    } else {
        PS::notification(notification);
    }
}

void Execute::upload(PS::TcpClient client, const char *directory, const char *filepath, int DOWNLOADER_PORT) {
    if (PS2::strcmp(filepath, "") == 0) {
        client.printf("Syntax: upload \"Filepath here e.g. /av_contents/content_tmp/disc01.iso\"");
        client.printf("\r\n");
    } else {
        char temp[128] = "";

        //Validate filepath
        if (Helper::isValidMultipleDirectoryCreatableFile(directory, filepath) == 1) {
            //Build temp filepath
            PS2::strcat(temp, directory);
            PS2::strcat(temp, filepath);
        } else if (Helper::isValidMultipleDirectoryCreatableFile(directory, filepath) == 2) {
            //Build temp filepath
            PS2::strcat(temp, filepath);
        }

        //Continue if filepath was valid
        if (PS2::strcmp(temp, "") != 0) {
            client.printf("Upload your file with a TCP client on the port 9045");
            client.printf("\r\n");

            //Download file
            if (!Downloader::download(temp, DOWNLOADER_PORT)) {
                PS::notification("Failed to download File");
                return;
            }

            //Check if file is there
            if (Helper::isValidMultipleDirectoryFile(directory, filepath) == 0) {
                client.printf(
                        "There was an error while downloading the file.\r\nMaybe the destination directory is read only?\r\n");
            }
        } else {
            client.printf("Not a valid file location/filename: ");
            client.printf(filepath);
            client.printf("\r\n");
        }
    }
}

void Execute::download(PS::TcpClient client, const char *directory, const char *filepath, int UPLOADER_PORT) {
    if (PS2::strcmp(filepath, "") == 0) {
        //Print help for rm
        client.printf("Syntax: download \"Filepath/Directory path here\"\r\n");
    } else {
        char temp[128] = "";
        bool isFile = false;

        //Validate filepath
        switch (Helper::isValidMultipleDirectoryFile(directory, filepath)) {
            case 1:
                //Build temp filepath
                PS2::strcat(temp, directory);
                PS2::strcat(temp, filepath);
                isFile = true;
                break;
            case 2:
                //Build temp filepath
                PS2::strcat(temp, filepath);
                isFile = true;
                break;
            case 0:
                switch (Helper::isValidMultipleDirectoryDirectory(directory, filepath)) {
                    case 1:
                        //Build temp directory path
                        PS2::strcat(temp, directory);
                        PS2::strcat(temp, filepath);
                        break;
                    case 2:
                        //Build temp directory path
                        PS2::strcat(temp, filepath);
                        break;
                }
        }


        //Continue if filepath was valid
        if (PS2::strcmp(temp, "") != 0) {

            // Print disclaimer
            if (!isFile)
                client.printf("Uploading directories is in a very experimental state!\r\nBe mindful of possible corrupt files!\r\n\r\n");
            else
                client.printf("This version of PS-PSH uses a very experimental method of transmitting files.\r\nIf you experience corrupt files, please consider downgrading to PS-PSH v0.1.0!!\r\n\r\n");

            client.printf(
                    "Connect to the console with the \"PS-PSH File Receiver\" and download the file/directory.\r\nhttps://github.com/aladie/ps-psh-file-receiver\r\n\r\n");

            //Download file
            if (!Uploader::uploadHandler(client, temp, UPLOADER_PORT, isFile)) {
                client.printf("(ERROR) Failed to listen on port 9046. Try again in 1 minute.\r\n");
                PS::notification("Failed to listen on port 9046.\nTry again in 1 minute.");
                return;
            }
        } else {
            client.printf("No such file/directory: ");
            client.printf(filepath);
            client.printf("\r\n");
        }
    }
}

void Execute::play(PS::TcpClient client, const char *directory, const char *filepath, const char *configpath) {
    if (PS2::strcmp(filepath, "") == 0) {
        //Print help for play
        client.printf(
                "Syntax: play \"Filepath/name here e.g. /av_contents/content_tmp/disc01.iso\" \"(Optional) Config filepath/name here e.g. /av_contents/content_tmp/game.conf\"\r\n");
    } else {
        char temp[128] = "";
        char temp_conf[128] = "";

        //Validate game filepath
        if (Helper::isValidMultipleDirectoryFile(directory, filepath) == 1) {
            //Build temp filepath
            PS2::strcat(temp, directory);
            PS2::strcat(temp, filepath);
        } else if (Helper::isValidMultipleDirectoryFile(directory, filepath) == 2) {
            //Build temp filepath
            PS2::strcat(temp, filepath);
        }

        //Validate config filepath
        if (Helper::isValidMultipleDirectoryFile(directory, configpath) == 1) {
            //Build temp filepath
            PS2::strcat(temp_conf, directory);
            PS2::strcat(temp_conf, configpath);
        } else if (Helper::isValidMultipleDirectoryFile(directory, configpath) == 2) {
            //Build temp filepath
            PS2::strcat(temp_conf, configpath);
        }

        //Continue if gamepath was valid
        if (PS2::strcmp(temp, "") != 0) {
            client.printf("Trying to load: ");
            client.printf(temp);
            client.printf("\r\n");
            client.disconnect();
            Helper::playPS2ISO(temp, temp_conf);
        } else {
            client.printf("No such file: ");
            client.printf(filepath);
            client.printf("\r\n");
        }
    }
}

void Execute::pwd(PS::TcpClient client, const char *directory) {
    client.printf("You are currently in: ");
    client.printf(directory);
    client.printf("\r\n");
}

void Execute::rm(PS::TcpClient client, const char *directory, const char *filepath_or_arg, const char *directoryPath) {
    if (PS2::strcmp(filepath_or_arg, "") == 0) {
        //Print help for rm
        client.printf("Syntax: rm \"Filepath/name or Directory path/name here\"\r\n");
    } else {
        char temp[512] = "";
        char toTest[256] = "";
        bool isFile = false;
        bool argProvided = false;

        //Check if -r is provided
        if (PS2::strcmp(filepath_or_arg, "-r") == 0) {
            PS2::strcat(toTest, directoryPath);
            argProvided = true;
        } else
            PS2::strcat(toTest, filepath_or_arg);

        //Validate filepath
        switch (Helper::isValidMultipleDirectoryFile(directory, toTest)) {
            case 1:
                //Build temp filepath
                PS2::strcat(temp, directory);
                PS2::strcat(temp, toTest);
                isFile = true;
                break;
            case 2:
                //Build temp filepath
                PS2::strcat(temp, toTest);
                isFile = true;
                break;
            default:
                switch (Helper::isValidMultipleDirectoryDirectory(directory, toTest)) {
                    case 1:
                        //Build temp filepath
                        PS2::strcat(temp, directory);
                        PS2::strcat(temp, toTest);
                        break;
                    case 2:
                        //Build temp filepath
                        PS2::strcat(temp, toTest);
                        break;
                }
        }

        //Continue if filepath was valid
        if (PS2::strcmp(temp, "") != 0) {
            //Check if the path is a file
            if (isFile) {
                PS::unlink(temp);

                //Check if the operation was successfull
                if (Helper::isValidMultipleDirectoryFile(directory, toTest) != 0)
                    client.printf("There was an error while deleting the file.\r\nMaybe the file is read only?\r\n");
                else {
                    client.printf("Successfully deleted the file: ");
                    client.printf(temp);
                    client.printf("\r\n");
                }
            } else {
                if (!argProvided) {
                    client.printf("This is a directory.\r\nTo delete a directory use: \"rm -r <path to directory>\"\r\n");
                    return;
                }

                //Check if the operation was successfull
                if (!Helper::deleteDirectory(temp))
                    client.printf(
                            "There was an error while deleting the directory.\r\nMaybe the directory is read only?\r\n");
                else {
                    client.printf("Successfully deleted the directory: ");
                    client.printf(temp);
                    client.printf("\r\n");
                }
            }
        } else {
            client.printf("No such file or directory: ");
            if (argProvided)
                client.printf(directoryPath);
            else
                client.printf(filepath_or_arg);
            client.printf("\r\n");
        }
    }
}

void Execute::mkdir(PS::TcpClient client, const char *directory, const char *dirpath) {
    if (PS2::strcmp(dirpath, "") == 0) {
        //Print help for mkdir
        client.printf("Syntax: mkdir \"New directory path/name here\"\r\n");
    } else {
        char temp[512] = "";

        //Test given destination path
        //Helper::isValidMultipleDirectoryCreatableFile can be used since it would be the same algorithm
        switch (Helper::isValidMultipleDirectoryCreatableFile(directory, dirpath)) {
            case 1:
                //Build destination directory path
                PS2::strcat(temp, directory);
                PS2::strcat(temp, dirpath);
                break;
            case 2:
                //Build destination filepath
                PS2::strcat(temp, dirpath);
                break;
        }

        //Continue if new directory path was valid
        if (PS2::strcmp(temp, "") != 0) {
            //Create new directory
            PS::mkdir(temp, 0777);

            //Check if the operation was successfull
            if (Helper::isValidMultipleDirectoryDirectory(directory, temp) == 0) {
                client.printf(
                        "There was an error while creating the directory.\r\nMaybe the destination directory is read only?\r\n");
            } else {
                client.printf("The directory was successfully created!\r\n");
            }
        } else {
            client.printf("Either the directory already exists or your given path is not valid!\r\n");
        }
    }
}

void Execute::cp_or_mv(PS::TcpClient client, const char *directory, const char *source, const char *dest, bool move) {
    if (PS2::strcmp(source, "") == 0 || PS2::strcmp(dest, "") == 0) {
        //Print help for cp/mv
        if (!move)
            client.printf("Syntax: cp \"Source filepath/directory\" \"Destination filepath/directory\"\r\n");
        else
            client.printf("Syntax: mv \"Source filepath/directory\" \"Destination filepath/directory\"\r\n");
    } else {
        char filepath_src[512] = "";
        bool src_isFile = false;
        char filepath_dst[512] = "";
        bool dst_isFile = false;

        //Test given source path
        switch (Helper::isValidMultipleDirectoryFile(directory, source)) {
            case 1:
                //Build source filepath
                PS2::strcat(filepath_src, directory);
                PS2::strcat(filepath_src, source);
                src_isFile = true;
                break;
            case 2:
                //Build source filepath
                PS2::strcat(filepath_src, source);
                src_isFile = true;
                break;
            case 0:
                //Test if given source is a directory
                switch (Helper::isValidMultipleDirectoryDirectory(directory, source)) {
                    case 1:
                        //Build source directory
                        PS2::strcat(filepath_src, directory);
                        PS2::strcat(filepath_src, source);
                        break;
                    case 2:
                        //Build source directory
                        PS2::strcat(filepath_src, source);
                        break;
                }
                break;
        }

        //Test given destination path
        switch (Helper::isValidMultipleDirectoryCreatableFile(directory, dest)) {
            case 1:
                //Build destination filepath
                PS2::strcat(filepath_dst, directory);
                PS2::strcat(filepath_dst, dest);
                dst_isFile = true;
                break;
            case 2:
                //Build destination filepath
                PS2::strcat(filepath_dst, dest);
                dst_isFile = true;
                break;
            case 0:
                //Test if given destination is a directory
                switch (Helper::isValidMultipleDirectoryDirectory(directory, dest)) {
                    case 1:
                        //Build destination directory
                        PS2::strcat(filepath_dst, directory);
                        PS2::strcat(filepath_dst, dest);
                        break;
                    case 2:
                        //Build destination directory
                        PS2::strcat(filepath_dst, dest);
                        break;
                }
                break;
        }

        //Validate given source and destination combination
        if (PS2::strcmp(filepath_src, "") != 0) {
            if (PS2::strcmp(filepath_dst, "") != 0) {

                //Test the combination
                if (src_isFile) {

                    //Add source filename to the destination if no filename is provided
                    if (!dst_isFile) {
                        int pointer = 0;
                        char filename[128] = "";
                        for (int i = PS2::lastIndexOf(filepath_src, '/') + 1; i < PS2::strlen(filepath_src); i++) {
                            filename[pointer] = filepath_src[i];
                            pointer++;
                        }
                        PS2::strcat(filepath_dst, filename);
                    }

                    if (!move)
                        client.printf("Copying file please wait...\r\n");
                    else
                        client.printf("Moving file please wait...\r\n");

                    //Test copy result
                    switch (Helper::copyFile(directory, filepath_src, filepath_dst)) {
                        case 1:
                            if (!move)
                                client.printf("The file was copied successfully!\r\n");
                            else {
                                //Try to delete the source file
                                PS::unlink(filepath_src);

                                if (Helper::isValidMultipleDirectoryFile(directory, filepath_src) == 0)
                                    client.printf("The file was moved successfully!\r\n");
                                else
                                    client.printf(
                                            "The file was copied successfully.\r\nHowever the source file could not be deleted.\r\nMaybe the source directory is read only?\r\n");
                            }
                            break;
                        case 0:
                            if (!move)
                                client.printf(
                                        "There was an error while copying the file.\r\nMaybe the destination directory is read only?\r\n");
                            else
                                client.printf(
                                        "There was an error while moving the file.\r\nMaybe the destination directory is read only?\r\n");
                            break;
                    }
                } else {
                    //Test destination for directory
                    bool new_name;
                    if (!dst_isFile)
                        new_name = false;
                    else
                        new_name = true;

                    if (!move) {
                        //Validate given combination
                        if (new_name) {
                            client.printf("The source is a directory but the given destination is a file!\r\n");
                            return;
                        }
                        client.printf("Copying directory please wait...\r\n");
                    }
                    else
                        client.printf("Moving directory please wait...\r\n");

                    //Test copy result
                    switch (Helper::copyDirectory(directory, filepath_src, filepath_dst, new_name)) {
                        case 1:
                            if (!move)
                                client.printf("The directory was copied successfully!\r\n");
                            else {
                                //Try to delete the source directory
                                PS::rmdir(filepath_src);

                                if (Helper::isValidMultipleDirectoryDirectory(directory, filepath_src) == 0)
                                    client.printf("The directory was moved successfully!\r\n");
                                else
                                    client.printf(
                                            "The directory was copied successfully.\r\nHowever the source directory could not be deleted.\r\nMaybe the source directory is read only?\r\n");
                            }
                            break;
                        case 0:
                            if (!move)
                                client.printf(
                                        "There was an error while copying the directory.\r\nMaybe the destination directory is read only?\r\n");
                            else
                                client.printf(
                                        "There was an error while moving the directory.\r\nMaybe the destination directory is read only?\r\n");
                            break;
                    }

                }
            } else {
                client.printf("The destination filepath/directory is not valid!\r\n");
            }
        } else {
            client.printf("The source filepath/directory is not valid!\r\n");
        }
    }
}

void Execute::help(PS::TcpClient client) {
    client.printf(
            "Available commands:\r\n - ls - Prints the content of the current directory\r\n - cd - Changes the current working directory\r\n - cp - Copies a file or directory, with the specified path, to the specified location\r\n - mv - Moves a file or directory, with the specified path, to the specified location\r\n - rm - Deletes a file or directory with the specified path\r\n - mkdir - Creates a new directory\r\n - pwd - Prints the current working directory\r\n - notification - Displays a notification with your desired message\r\n - upload - Uploads a file to the specified filepath\r\n - download - Download a file to your local machine\r\n - play - Play a PS2 ISO with the specified filepath\r\n - exit - Closes the Server\r\n");
}

void Execute::cd(PS::TcpClient client, char directory[256], char new_directory[256]) {
    if (PS2::strcmp(new_directory, "") == 0) {
        //Print help for cd
        client.printf("Syntax: cd \"Directory name here\"");
        client.printf("\r\n");
        client.printf("Syntax: To return back to / use \"cd /\"");
        client.printf("\r\n");
    } else {
        //Create temporary new directory char
        char temp[512] = "";
        PS2::strcat(temp, directory);
        PS2::strcat(temp, new_directory);

        //Check for multiple directories
        int index[20];

        for (int i = 0; i < 20; i++) {
            index[i] = 512;
        }

        int howMany = 0;
        char toTest[512] = "";
        PS2::strcat(toTest, new_directory);

        //Get indexes of possibly multiple "/"
        int actualIndex = Helper::firstIndexOf(toTest, '/');
        while (actualIndex != -1) {
            index[howMany] = actualIndex;
            howMany++;
            toTest[actualIndex] = 'P';
            actualIndex = Helper::firstIndexOf(toTest, '/');
        }

        //If only one directory check normally
        if (howMany == 0) {
            PS2::strcat(temp, "/");

            //Check if user wants to go up in the directory tree
            if (PS2::strcmp(new_directory, "..") == 0) {
                if (PS2::strcmp(directory, "/") != 0) {
                    directory[PS2::lastIndexOf(directory, '/')] = '\0';
                    directory[PS2::lastIndexOf(directory, '/') + 1] = '\0';
                }
            } else {
                //Check if the new directory exists
                if (Helper::isDirectory(directory, new_directory)) {
                    //Update current directory
                    memset(directory, 0, sizeof(directory));
                    PS2::strcat(directory, temp);
                } else {
                    client.printf("No such directory: ");
                    client.printf(temp);
                    client.printf("\r\n");
                }
            }
        } else {
            //Check if user wants to go back to /
            if (index[0] == 0) {
                memset(directory, 0, sizeof(directory));
                memset(temp, 0, sizeof(temp));
                PS2::strcat(temp, new_directory);
            }

            //Check individual directories
            for (int i = 0; i < howMany; i++) {
                if (i == 0) {
                    //Get first folder name
                    memset(toTest, 0, sizeof(toTest));
                    PS2::strcat(toTest, new_directory);
                    toTest[index[i]] = '\0';

                    //Check if user wants to go up in the directory tree
                    if (PS2::strcmp(new_directory, "..") == 0) {
                        if (PS2::strcmp(directory, "/") != 0) {
                            directory[PS2::lastIndexOf(directory, '/')] = '\0';
                            directory[PS2::lastIndexOf(directory, '/') + 1] = '\0';
                        }
                    } else {
                        if (Helper::isDirectory(directory, toTest)) {
                            //Update current directory
                            PS2::strcat(directory, toTest);
                            PS2::strcat(directory, "/");
                        } else {
                            //Check if user wanted to go back to /
                            if (PS2::strcmp(directory, "") == 0) {
                                PS2::strcat(directory, "/");
                            } else {
                                //Fix output
                                for (int j = 0; j < sizeof(temp); j++) {
                                    if (temp[j] == '\0') {
                                        if (temp[j - 1] == '/')
                                            temp[j - 1] = '\0';
                                        break;
                                    }
                                }

                                client.printf("No such directory: ");
                                client.printf(temp);
                                client.printf("/\r\n");
                                break;
                            }
                        }
                    }
                }

                //Get possible multiple directories from command
                memset(toTest, 0, sizeof(toTest));
                int paramXLength = 0;
                for (int j = index[i] + 1; j < index[i + 1]; j++) {
                    toTest[paramXLength] = new_directory[j];
                    paramXLength++;
                }

                //Check if user wants to go up in the directory tree
                if (PS2::strcmp(new_directory, "..") == 0) {
                    directory[PS2::lastIndexOf(directory, '/')] = '\0';
                    directory[PS2::lastIndexOf(directory, '/') + 1] = '\0';
                } else {
                    //Only check new string if it is not empty
                    if (toTest[0] != '\0') {
                        if (Helper::isDirectory(directory, toTest)) {
                            //Update current directory
                            PS2::strcat(directory, toTest);
                            PS2::strcat(directory, "/");
                        } else {
                            //Fix output
                            for (int j = 0; j < sizeof(temp); j++) {
                                if (temp[j] == '\0') {
                                    if (temp[j - 1] == '/')
                                        temp[j - 1] = '\0';
                                    break;
                                }
                            }

                            client.printf("No such directory: ");
                            client.printf(temp);
                            client.printf("/\r\n");
                            break;
                        }
                    }
                }
            }
        }
    }
}