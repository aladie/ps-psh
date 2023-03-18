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
                if (dent->d_type == DT_REG && (PS2::strcmp(filename, dent->d_name) == 0))
                    return true;
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
    return false;
}

bool Helper::isDirectory(const char *directory, const char *dirname) {
    //Iterate through the current directory and check if the "filename" is actually a file
    int fd = PS::open(directory, 0, 0);

    if (fd > 0) {
        char buffer[1024];
        int nread = 0;
        while ((nread = PS::getdents(fd, buffer, sizeof(buffer))) > 0) {

            struct dirent *dent = (struct dirent *) buffer;

            while (dent->d_fileno) {
                if (dent->d_type == DT_DIR && (PS2::strcmp(dirname, dent->d_name) == 0))
                    return true;
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
    return false;
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