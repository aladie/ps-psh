#include "Execute.hpp"
#include "../downloader/Downloader.hpp"
#include "../uploader/Uploader.hpp"
#include "../helper/Helper.hpp"

void Execute::ls(PS::TcpClient client, const char *directory) {
    // Print directories/files
    int fd = PS::open(directory, 0, 0);

    if (fd > 0) {
        char buffer[1024];
        int nread = 0;
        while ((nread = PS::getdents(fd, buffer, sizeof(buffer))) > 0) {

            struct dirent *dent = (struct dirent *) buffer;

            while (dent->d_fileno) {

                if (dent->d_type == DT_DIR)
                    client.printf("d %s%s\r\n", directory, dent->d_name);
                if (dent->d_type == DT_REG)
                    client.printf("- %s%s\r\n", directory, dent->d_name);
                if (dent->d_reclen == 0)
                    break;
                nread -= dent->d_reclen;
                if (nread < 0)
                    break;
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
                client.printf("There was an error while downloading the file.\r\nMaybe the destination directory is read only?\r\n");
            }
        } else {
            client.printf("No such file: ");
            client.printf(filepath);
            client.printf("\r\n");
        }
    }
}

void Execute::download(PS::TcpClient client, const char *directory, const char *filepath, int UPLOADER_PORT) {
    if (PS2::strcmp(filepath, "") == 0) {
        //Print help for rm
        client.printf("Syntax: download \"Filepath/name here\"\r\n");
    } else {
        char temp[128] = "";

        //Validate filepath
        if (Helper::isValidMultipleDirectoryFile(directory, filepath) == 1) {
            //Build temp filepath
            PS2::strcat(temp, directory);
            PS2::strcat(temp, filepath);
        } else if (Helper::isValidMultipleDirectoryFile(directory, filepath) == 2) {
            //Build temp filepath
            PS2::strcat(temp, filepath);
        }

        //Continue if filepath was valid
        if (PS2::strcmp(temp, "") != 0) {
            client.printf("Connect and download the file with the \"PS-PSH File Receiver\"\r\nOr use your own tool on the TCP port 9045");
            client.printf("\r\n");

            //Download file
            if (!Uploader::upload(temp, UPLOADER_PORT)) {
                PS::notification("Failed to listen on port 9045.\nTry again in 1 minute.");
                return;
            }
        } else {
            client.printf("No such file: ");
            client.printf(filepath);
            client.printf("\r\n");
        }
    }
}

void Execute::play(PS::TcpClient client, const char *directory, const char *filepath, const char *configpath) {
    if (PS2::strcmp(filepath, "") == 0) {
        //Print help for play
        client.printf("Syntax: play \"Filepath/name here e.g. /av_contents/content_tmp/disc01.iso\" \"(Optional) Config filepath/name here e.g. /av_contents/content_tmp/game.conf\"\r\n");
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

void Execute::rm(PS::TcpClient client, const char *directory, const char *filepath) {
    if (PS2::strcmp(filepath, "") == 0) {
        //Print help for rm
        client.printf("Syntax: rm \"Filepath/name here\"\r\n");
    } else {
        char temp[128] = "";

        //Validate filepath
        if (Helper::isValidMultipleDirectoryFile(directory, filepath) == 1) {
            //Build temp filepath
            PS2::strcat(temp, directory);
            PS2::strcat(temp, filepath);
        } else if (Helper::isValidMultipleDirectoryFile(directory, filepath) == 2) {
            //Build temp filepath
            PS2::strcat(temp, filepath);
        }

        //Continue if filepath was valid
        if (PS2::strcmp(temp, "") != 0) {
            PS::unlink(temp);
            client.printf("Successfully deleted the file: ");
            client.printf(temp);
            client.printf("\r\n");
        } else {
            client.printf("No such file: ");
            client.printf(filepath);
            client.printf("\r\n");
        }
    }
}

void Execute::cp(PS::TcpClient client, const char *directory, const char *source, const char *dest) {
    if (PS2::strcmp(source, "") == 0 || PS2::strcmp(dest, "") == 0) {
        //Print help for cp
        client.printf("Syntax: cp \"Source filepath/name here\" \"Destination filepath/name here\"\r\n");
    } else {
        char filepath_src[256] = "";
        char filepath_dst[256] = "";

        //Test given source path
        if (Helper::isValidMultipleDirectoryFile(directory, source) == 1) {
            //Build source filepath
            PS2::strcat(filepath_src, directory);
            PS2::strcat(filepath_src, source);
        } else if (Helper::isValidMultipleDirectoryFile(directory, source) == 2) {
            //Build source filepath
            PS2::strcat(filepath_src, source);
        }

        //Test given destination path
        if (Helper::isValidMultipleDirectoryCreatableFile(directory, dest) == 1) {
            //Build destination filepath
            PS2::strcat(filepath_dst, directory);
            PS2::strcat(filepath_dst, dest);
        } else if (Helper::isValidMultipleDirectoryCreatableFile(directory, dest) == 2) {
            //Build destination filepath
            PS2::strcat(filepath_dst, dest);
        }

        //Continue if the source and destination paths are valid
        if (PS2::strcmp(filepath_src, "") != 0) {
            if (PS2::strcmp(filepath_dst, "") != 0) {
                size_t filesize = PS::Filesystem::getFileSize(filepath_src);

                // Show progress bar dialog
                PS::Sce::MsgDialog::Initialize();
                PS::Sce::MsgDialogProgressBar progressDialog = PS::Sce::MsgDialogProgressBar("Copying file...");
                progressDialog.open();
                Helper::setProgress(progressDialog, 0, filesize);

                //Open the source and destination filepaths
                int fd_src = PS::open(filepath_src, O_RDONLY, 0);
                int fd_dst = PS::open(filepath_dst, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

                size_t offset = 0;
                size_t bufferSize = 8192;
                char buffer[bufferSize];

                client.printf("Copying file please wait...\r\n");

                //Copy the file in chunks
                uint32_t updateBar = 0;
                while (true) {
                    size_t read_count = PS::readAll(fd_src, buffer, bufferSize);
                    size_t write_count;
                    offset += read_count;

                    //Adjust buffer if EOF
                    if (read_count < bufferSize) {
                        PS::writeAll(fd_dst, buffer, read_count);
                        break;
                    }
                    else
                        write_count = PS::writeAll(fd_dst, buffer, bufferSize);

                    //Break if the directory is read only
                    if (read_count != write_count)
                        break;

                    if (updateBar == UPLOAD_BAR_UPDATE_FREQUENCY) {
                        if (filesize != 0)
                            Helper::setProgress(progressDialog, offset, filesize);
                        updateBar = 0;
                    }
                    updateBar++;
                }

                progressDialog.setValue(100);
                progressDialog.close();
                PS::Sce::MsgDialog::Terminate();
            } else {
                client.printf("The destination filepath is not valid!\r\n");
            }
        } else {
            client.printf("The source filepath is not valid!\r\n");
        }

        //Check if the file copied successfully
        if (Helper::isValidMultipleDirectoryFile(directory, dest) == 0) {
            client.printf("There was an error while copying the file.\r\nMaybe the destination directory is read only?\r\n");
        }
    }
}

void Execute::mv(PS::TcpClient client, const char *directory, const char *source, const char *dest) {
    if (PS2::strcmp(source, "") == 0 || PS2::strcmp(dest, "") == 0) {
        //Print help for mv
        client.printf("Syntax: mv \"Source filepath/name here\" \"Destination filepath/name here\"\r\n");
    } else {
        char filepath_src[256] = "";
        char filepath_dst[256] = "";

        //Test given source path
        if (Helper::isValidMultipleDirectoryFile(directory, source) == 1) {
            //Build source filepath
            PS2::strcat(filepath_src, directory);
            PS2::strcat(filepath_src, source);
        } else if (Helper::isValidMultipleDirectoryFile(directory, source) == 2) {
            //Build source filepath
            PS2::strcat(filepath_src, source);
        }

        //Test given destination path
        if (Helper::isValidMultipleDirectoryCreatableFile(directory, dest) == 1) {
            //Build destination filepath
            PS2::strcat(filepath_dst, directory);
            PS2::strcat(filepath_dst, dest);
        } else if (Helper::isValidMultipleDirectoryCreatableFile(directory, dest) == 2) {
            //Build destination filepath
            PS2::strcat(filepath_dst, dest);
        }

        //Continue if the source and destination paths are valid
        if (PS2::strcmp(filepath_src, "") != 0) {
            if (PS2::strcmp(filepath_dst, "") != 0) {
                size_t filesize = PS::Filesystem::getFileSize(filepath_src);

                // Show progress bar dialog
                PS::Sce::MsgDialog::Initialize();
                PS::Sce::MsgDialogProgressBar progressDialog = PS::Sce::MsgDialogProgressBar("Moving file...");
                progressDialog.open();
                Helper::setProgress(progressDialog, 0, filesize);

                //Open the source and destination filepaths
                int fd_src = PS::open(filepath_src, O_RDONLY, 0);
                int fd_dst = PS::open(filepath_dst, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

                size_t offset = 0;
                size_t bufferSize = 8192;
                char buffer[bufferSize];

                client.printf("Moving file please wait...\r\n");

                //Copy the file in chunks
                uint32_t updateBar = 0;
                while (true) {
                    size_t read_count = PS::readAll(fd_src, buffer, bufferSize);
                    size_t write_count;
                    offset += read_count;

                    //Adjust buffer if EOF
                    if (read_count < bufferSize) {
                        PS::writeAll(fd_dst, buffer, read_count);
                        break;
                    }
                    else
                        write_count = PS::writeAll(fd_dst, buffer, bufferSize);

                    //Break if the directory is read only
                    if (read_count != write_count)
                        break;

                    if (updateBar == UPLOAD_BAR_UPDATE_FREQUENCY) {
                        if (filesize != 0)
                            Helper::setProgress(progressDialog, offset, filesize);
                        updateBar = 0;
                    }
                    updateBar++;
                }

                progressDialog.setValue(100);
                progressDialog.close();
                PS::Sce::MsgDialog::Terminate();
            } else {
                client.printf("The destination filepath is not valid!\r\n");
            }
        } else {
            client.printf("The source filepath is not valid!\r\n");
        }

        //Check if the file moved successfully
        if (Helper::isValidMultipleDirectoryFile(directory, dest) == 0) {
            client.printf("There was an error while moving the file.\r\nMaybe the destination directory is read only?\r\n");
        } else {
            PS::unlink(filepath_src);
        }
    }
}

void Execute::help(PS::TcpClient client) {
    client.printf(
            "Available commands:\r\n - ls - Prints the content of the current directory\r\n - cd - Changes the current working directory\r\n - cp - Copies a file with the specified filepath or filename to the specified filepath\r\n - mv - Moves a file with the specified filepath or filename to the specified filepath\r\n - rm - Deletes a file with the specified filepath or filename\r\n - pwd - Prints the current working directory\r\n - notification - Displays a notification with your desired message\r\n - upload - Uploads a file to the specified filepath\r\n - download - Download a file to your local machine\r\n - play - Play a PS2 ISO with the specified filepath\r\n - exit - Closes the Server\r\n");
}

void Execute::cd(PS::TcpClient client, char directory[128], char new_directory[128]) {
    if (PS2::strcmp(new_directory, "") == 0) {
        //Print help for cd
        client.printf("Syntax: cd \"Directory name here\"");
        client.printf("\r\n");
        client.printf("Syntax: To return back to / use \"cd /\"");
        client.printf("\r\n");
    } else {
        //Create temporary new directory char
        char temp[128] = "";
        PS2::strcat(temp, directory);
        PS2::strcat(temp, new_directory);

        //Check for multiple directories
        int index[10];

        for (int i = 0; i < 10; i++) {
            index[i] = 128;
        }

        int howMany = 0;
        char toTest[128] = "";
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