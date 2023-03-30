#include "Helper.hpp"

bool Helper::isFile(const char *directory, const char *filename) {
    //Iterate through the current directory and check if the "filename" is actually a file
    int fd = PS::open(directory, 0, 0);

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
                if (dent->d_type == DT_REG && (PS2::strcmp(filename, dent->d_name) == 0))
                    return true;
                dent = (struct dirent *) ((char *) dent + dent->d_reclen);

            }

            memset(buffer, 0, sizeof(buffer));


        }
    }
    PS::close(fd);
    return false;
}

bool Helper::isDirectory(const char *directory, const char *dirname) {
    //Iterate through the current directory and check if the "filename" is actually a directory
    int fd = PS::open(directory, 0, 0);

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
                if (dent->d_type == DT_DIR && (PS2::strcmp(dirname, dent->d_name) == 0))
                    return true;
                dent = (struct dirent *) ((char *) dent + dent->d_reclen);

            }

            memset(buffer, 0, sizeof(buffer));


        }
    }
    PS::close(fd);
    return false;
}

bool Helper::copyFile(const char *directory, const char *source, const char *destination) {
    size_t filesize = PS::Filesystem::getFileSize(source);

    // Show progress bar dialog
    PS::Sce::MsgDialog::Initialize();
    PS::Sce::MsgDialogProgressBar progressDialog = PS::Sce::MsgDialogProgressBar("Copying file...");
    progressDialog.open();
    Helper::setProgress(progressDialog, 0, filesize);

    //Open the source and destination filepaths
    int fd_src = PS::open(source, O_RDONLY, 0);
    int fd_dst = PS::open(destination, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

    size_t offset = 0;
    size_t bufferSize = 8192;
    char buffer[bufferSize];

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
        } else
            write_count = PS::writeAll(fd_dst, buffer, bufferSize);

        //Break if the directory is read only
        if (read_count != write_count)
            break;

        if (updateBar == 2500) {
            if (filesize != 0)
                Helper::setProgress(progressDialog, offset, filesize);
            updateBar = 0;
        }
        updateBar++;
    }

    progressDialog.setValue(100);
    progressDialog.close();
    PS::Sce::MsgDialog::Terminate();

    PS::close(fd_src);
    PS::close(fd_dst);

    //Check if the file copied successfully
    if (Helper::isValidMultipleDirectoryFile(directory, destination) == 0)
        return false;
    return true;
}


//HIGHLY EXPERIMENTAL -> WORK IN PROGRESS
//USE WITH CAUTION
bool Helper::copyDirectory(const char *directory, const char *source, const char *destination) {
    //Get source directory name
    char tmpSource[512] = "";
    PS2::strcat(tmpSource, source);

    char name[128] = "";
    int pointer = 0;

    //Check if there is a "/" at the end
    if (tmpSource[PS2::strlen(tmpSource) - 1] == '/')
        //Remove "/" at the end
        tmpSource[PS2::strlen(tmpSource) - 1] = '\0';

    //Get directory name
    for (int i = PS2::lastIndexOf(tmpSource, '/') + 1; i < PS2::strlen(tmpSource); i++) {
        if (tmpSource[i] != '/') {
            name[pointer] = tmpSource[i];
            pointer++;
        }
    }

    //Add "/" at the end
    PS2::strcat(tmpSource, "/");
    PS2::strcat(name, "/");

    //Create new directory path
    char tempDestination[512] = "";
    PS2::strcat(tempDestination, destination);

    //Check if there is no "/" at the end
    if (destination[PS2::strlen(destination) - 1] != '/')
        //Add "/" at the end
        PS2::strcat(tempDestination, "/");

    //Add directory name
    PS2::strcat(tempDestination, name);

    //Create destination folder if it does not exist
    if (isValidMultipleDirectoryDirectory(directory, tempDestination) == 0) {
        //Create destination directory
        PS::mkdir(tempDestination, 0777);

        //Check if the operation failed
        if (isValidMultipleDirectoryDirectory(directory, tempDestination) == 0)
            return false;

        //Iterate through the source directory and check for files
        int fd = PS::open(tmpSource, 0, 0);

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
                    if (dent->d_type == DT_REG) {

                        //Build source filepath
                        char sourceFilePath[512] = "";
                        PS2::strcat(sourceFilePath, tmpSource);
                        PS2::strcat(sourceFilePath, dent->d_name);

                        //Build destination filepath
                        char destinationFilePath[512] = "";
                        PS2::strcat(destinationFilePath, tempDestination);
                        PS2::strcat(destinationFilePath, dent->d_name);

                        //Finally actually copy the file
                        if (!copyFile(directory, sourceFilePath, destinationFilePath))
                            return false;
                    }
                    dent = (struct dirent *) ((char *) dent + dent->d_reclen);

                }
                memset(buffer, 0, sizeof(buffer));
            }
        }
        PS::close(fd);

        //Iterate through the source directory and check for directories
        int fd1 = PS::open(tmpSource, 0, 0);

        if (fd > 0) {
            char buffer[1024];
            int nread = 0;
            while ((nread = PS::getdents(fd1, buffer, sizeof(buffer))) > 0) {

                struct dirent *dent = (struct dirent *) buffer;

                while (dent->d_fileno) {

                    if (dent->d_reclen == 0)
                        break;
                    nread -= dent->d_reclen;
                    if (nread < 0)
                        break;
                    if (dent->d_type == DT_DIR) {
                        //Skip /. and /..
                        if (PS2::strcmp(dent->d_name, ".") != 0 && PS2::strcmp(dent->d_name, "..") != 0) {
                            //Build source directory path
                            char sourceDirectoryPath[512] = "";
                            PS2::strcat(sourceDirectoryPath, tmpSource);
                            PS2::strcat(sourceDirectoryPath, dent->d_name);

                            //Finally actually copy the directory
                            if (!copyDirectory(directory, sourceDirectoryPath, tempDestination))
                                return false;
                        }
                    }
                    dent = (struct dirent *) ((char *) dent + dent->d_reclen);

                }
                memset(buffer, 0, sizeof(buffer));
            }
        }
        PS::close(fd1);
    }

    return true;
}

void Helper::playPS2ISO(const char *gamepath, const char *configpath) {
    //Builld complete filepath
    char filepath[128] = "./..";
    PS2::strcat(filepath, gamepath);

    //Build complete configpath
    char confpath[128] = "./..";
    PS2::strcat(filepath, configpath);

    PS::MountDiscWithFilepath(filepath);

    // Get game code from mounted file
    char *gameCode = PS::GetMountedGameCode();
    if (PS2::strlen(gameCode) == 10) {
        // Convert name from "SCUS-97129" -> "cdrom0:\\SCUS_971.29;1"
        char *ps2Path = PS2::gameCodeToPath(gameCode);

        // If a config is provided, load it
        if (PS2::strcmp(confpath, "./..") != 0) {
            PS::ProcessConfigFile(confpath);
        }

        // Restore corruption
        PS::Breakout::restore();

        // Execute mounted iso
        PS2::LoadExecPS2(ps2Path, 0, NULL);
        return;
    }
}

int Helper::firstIndexOf(const char *str, char c) {
    int len = PS2::strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == c)
            return i;
    }
    return -1;
}

void Helper::setProgress(PS::Sce::MsgDialogProgressBar dialog, size_t uploaded, size_t total) {
    if (total == 0)
        return;

    // Calculate percentage without float/double
    uint64_t divident = uploaded * 100;
    uint64_t percentage = 0;

    while (divident >= total) {
        divident -= total;
        percentage++;
    }

    dialog.setValue(percentage);
}

//If = 1 -> directory without "/" at index 0. If = 2 -> directory with "/" at index 0. If = 0 -> not a valid directory.
int Helper::isValidMultipleDirectoryFile(const char *directory, const char *multiple_directory_file) {
    //Prepare for checking for multiple directories
    int index[10];

    for (int i = 0; i < 10; i++) {
        index[i] = 128;
    }

    int howMany = 0;
    char toTest[128] = "";
    PS2::strcat(toTest, multiple_directory_file);

    //Get indexes of possibly multiple "/"
    int actualIndex = firstIndexOf(toTest, '/');
    while (actualIndex != -1) {
        index[howMany] = actualIndex;
        howMany++;
        toTest[actualIndex] = 'P';
        actualIndex = firstIndexOf(toTest, '/');
    }

    //If only the filename is provided check normally
    if (howMany == 0) {
        //Check if the file exists
        if (isFile(directory, multiple_directory_file)) {
            return 1;
        } else {
            return 0;
        }

    } else {
        int fileType = 1;
        bool fileFound = false;

        //Create temporary new directory char
        char temp[128] = "";
        PS2::strcat(temp, directory);

        //Check if the filepath starts with "/"
        if (index[0] == 0) {
            memset(temp, 0, sizeof(temp));
            fileType++;
        }

        //Check individual directories
        for (int i = 0; i < howMany; i++) {
            if (i == 0) {
                //Get first folder name
                memset(toTest, 0, sizeof(toTest));
                PS2::strcat(toTest, multiple_directory_file);
                toTest[index[i]] = '\0';


                if (isDirectory(temp, toTest)) {
                    //Update temporary directory
                    PS2::strcat(temp, toTest);
                    PS2::strcat(temp, "/");
                } else if (isFile(temp, toTest)) {
                    fileFound = true;
                } else {
                    //Check if filepath started with "/"
                    if (PS2::strcmp(temp, "") == 0) {
                        PS2::strcat(temp, "/");
                    } else {
                        return 0;
                    }
                }
            }

            //Prevent false positive
            if (fileFound && (index[i + 1] == 128)) {
                return 0;
            }

            //Get possible multiple directories from command
            memset(toTest, 0, sizeof(toTest));
            int paramXLength = 0;
            for (int j = index[i] + 1; j < index[i + 1]; j++) {
                toTest[paramXLength] = multiple_directory_file[j];
                paramXLength++;
            }

            //Only check new string if it is not empty
            if (toTest[0] != '\0') {
                if (isDirectory(temp, toTest)) {
                    //Update temporary directory
                    PS2::strcat(temp, toTest);
                    PS2::strcat(temp, "/");
                } else if (isFile(temp, toTest)) {
                    fileFound = true;
                } else {
                    return 0;
                }
            }
        }
        if (fileFound)
            return fileType;
    }
    return 0;
}

//If = 1 -> Creatable file directory without "/" at index 0. If = 2 -> Creatable file directory with "/" at index 0. If = 0 -> not a valid directory.
int Helper::isValidMultipleDirectoryCreatableFile(const char *directory, const char *multiple_directory_file) {
    //Prepare for checking for multiple directories
    int index[10];

    for (int i = 0; i < 10; i++) {
        index[i] = 128;
    }

    int howMany = 0;
    char toTest[128] = "";
    PS2::strcat(toTest, multiple_directory_file);

    //Get indexes of possibly multiple "/"
    int actualIndex = firstIndexOf(toTest, '/');
    while (actualIndex != -1) {
        index[howMany] = actualIndex;
        howMany++;
        toTest[actualIndex] = 'P';
        actualIndex = firstIndexOf(toTest, '/');
    }

    //If only the filename is provided check normally
    if (howMany == 0) {
        //Check if the file/directory exists
        if (isFile(directory, multiple_directory_file))
            return 0;
        else if (isDirectory(directory, multiple_directory_file))
            return 0;
        else
            return 1;

    } else {
        int creatableType = 1;
        bool validLocationFound = false;

        //Create temporary new directory char
        char temp[128] = "";
        PS2::strcat(temp, directory);

        //Check if the filepath starts with "/"
        if (index[0] == 0) {
            memset(temp, 0, sizeof(temp));
            creatableType++;
        }

        //Check individual directories
        for (int i = 0; i < howMany; i++) {
            if (i == 0) {
                //Get first folder name
                memset(toTest, 0, sizeof(toTest));
                PS2::strcat(toTest, multiple_directory_file);
                toTest[index[i]] = '\0';


                if (isDirectory(temp, toTest)) {
                    //Update temporary directory
                    PS2::strcat(temp, toTest);
                    PS2::strcat(temp, "/");
                } else if (isFile(temp, toTest)) {
                    return 0;
                } else {
                    //Check if filepath started with "/"
                    if (PS2::strcmp(temp, "") == 0) {
                        PS2::strcat(temp, "/");
                    } else {
                        validLocationFound = true;
                    }
                }
            }

            //Prevent false positive
            if (validLocationFound && (index[i + 1] == 128)) {
                return 0;
            }

            //Get possible multiple directories from command
            memset(toTest, 0, sizeof(toTest));
            int paramXLength = 0;
            for (int j = index[i] + 1; j < index[i + 1]; j++) {
                toTest[paramXLength] = multiple_directory_file[j];
                paramXLength++;
            }

            //Only check new string if it is not empty
            if (toTest[0] != '\0') {
                if (isDirectory(temp, toTest)) {
                    //Update temporary directory
                    PS2::strcat(temp, toTest);
                    PS2::strcat(temp, "/");
                } else if (isFile(temp, toTest)) {
                    return 0;
                } else {
                    validLocationFound = true;
                }
            }
        }
        if (validLocationFound)
            return creatableType;
    }
    return 0;
}

//If = 1 -> Valid directory without "/" at index 0. If = 2 -> Valid directory with "/" at index 0. If = 0 -> not a valid directory.
int Helper::isValidMultipleDirectoryDirectory(const char *directory, const char *multiple_directory_directory) {
    //Prepare for checking for multiple directories
    int index[10];

    for (int i = 0; i < 10; i++) {
        index[i] = 128;
    }

    int howMany = 0;
    char toTest[128] = "";
    PS2::strcat(toTest, multiple_directory_directory);

    //Get indexes of possibly multiple "/"
    int actualIndex = firstIndexOf(toTest, '/');
    while (actualIndex != -1) {
        index[howMany] = actualIndex;
        howMany++;
        toTest[actualIndex] = 'P';
        actualIndex = firstIndexOf(toTest, '/');
    }

    //If only the filename is provided check normally
    if (howMany == 0) {
        //Check if the file/directory exists
        if (isDirectory(directory, multiple_directory_directory))
            return 1;
        else
            return 0;

    } else {
        int directoryType = 1;

        //Create temporary new directory char
        char temp[128] = "";
        PS2::strcat(temp, directory);

        //Check if the filepath starts with "/"
        if (index[0] == 0) {
            memset(temp, 0, sizeof(temp));
            directoryType++;
        }

        //Check individual directories
        for (int i = 0; i < howMany; i++) {
            if (i == 0) {
                //Get first folder name
                memset(toTest, 0, sizeof(toTest));
                PS2::strcat(toTest, multiple_directory_directory);
                toTest[index[i]] = '\0';


                if (Helper::isDirectory(temp, toTest)) {
                    //Update current directory
                    PS2::strcat(temp, toTest);
                    PS2::strcat(temp, "/");
                } else {
                    //Check if user wanted to go back to /
                    if (PS2::strcmp(temp, "") == 0) {
                        PS2::strcat(temp, "/");
                    } else {
                        return 0;
                    }
                }

            }

            //Get possible multiple directories from command
            memset(toTest, 0, sizeof(toTest));
            int paramXLength = 0;
            for (int j = index[i] + 1; j < index[i + 1]; j++) {
                toTest[paramXLength] = multiple_directory_directory[j];
                paramXLength++;
            }

            //Only check new string if it is not empty
            if (toTest[0] != '\0') {
                if (Helper::isDirectory(temp, toTest)) {
                    //Update current directory
                    PS2::strcat(temp, toTest);
                    PS2::strcat(temp, "/");
                } else {
                    return 0;
                }
            }

        }
        return directoryType;
    }
}