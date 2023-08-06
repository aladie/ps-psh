#include "Uploader.hpp"

void Uploader::uploadFile(const char *filepath, PS::TcpClient data_client, PS::TcpClient console) {
    // Open the source filepath
    int fd_src = PS::open(filepath, O_RDONLY, 0);

    console.printf("(INFO) FILEPATH:");
    console.printf(filepath);
    console.printf("\r\n");

    // Get the filesize
    size_t filesize = PS::Filesystem::getFileSize(filepath);

    // Send type -> File
    char type[2] = "f";
    data_client.send(type, 1);

    // Get filename
    int pointer = 0;
    char filename[512] = "";
    for (int i = PS2::lastIndexOf(filepath, '/') + 1; i < PS2::strlen(filepath); i++) {
        filename[pointer] = filepath[i];
        pointer++;
    }

    // Convert length of filename to char array
    char filenameLength[21] = "";
    sizeTtoCharArray(pointer, filenameLength);

    // Add end of message indicator
    PS2::strcat(filenameLength, "R");

    // Send filesize, length of the filename and the filename
    data_client.send(filenameLength, 20);
    data_client.send(filename, pointer);

    //Wait for answer from client
    console.printf("(INFO) Waiting for response from client...\r\n");
    char answer[5];
    data_client.read(answer, 4);
    if (PS2::strcmp(answer, "F_OK") != 0) {
        console.printf(answer);
        console.printf("(ERROR) Filename not received. Aborting.\r\n");
        return;
    }
    console.printf("(INFO) Filename successfully send.\r\n");

    // Create the file chunk buffer and total send offset
    size_t bufferSize = UPLOAD_CHUNK_SIZE;
    size_t offset = 0;
    char buffer[bufferSize + 1];

    char buffersizeInChar[21];
    sizeTtoCharArray(bufferSize, buffersizeInChar);
    PS2::strcat(buffersizeInChar, "R");

    // Show progress bar dialog
    char message[141] = "Uploading ";
    PS2::strcat(message, filename);
    PS2::strcat(message, "...");
    PS::Sce::MsgDialog::Initialize();
    PS::Sce::MsgDialogProgressBar progressDialog = PS::Sce::MsgDialogProgressBar(message);
    progressDialog.open();
    Uploader::setProgress(progressDialog, 0, filesize);

    //Send the file in chunks
    uint32_t updateBar = 0;

    console.printf("(INFO) Starting the file upload...\r\n");

    while (true) {
        size_t read_count = PS::readAll(fd_src, buffer, bufferSize);
        offset += read_count;

        //Adjust buffersize if EOF
        if (read_count < bufferSize) {
            data_client.send(buffer, read_count);
            break;
        } else {
            data_client.send(buffer, bufferSize);
        }

        // Update progress bar
        if (updateBar == UPLOAD_BAR_UPDATE_FREQUENCY) {
            if (filesize != 0)
                Uploader::setProgress(progressDialog, offset, filesize);
            updateBar = 0;
        }
        updateBar++;

        memset(buffer, 0, sizeof(buffer));
    }

    // Terminate the progress bar
    progressDialog.setValue(100);
    progressDialog.close();
    PS::Sce::MsgDialog::Terminate();

    PS::close(fd_src);

    // Wait for answer from client
    console.printf("(INFO) Waiting for response from client...\r\n");
    memset(answer, 0, sizeof(answer));
    data_client.read(answer, 4);
    if (PS2::strcmp(answer, "R_OK") != 0) {
        console.printf(answer);
        console.printf("(ERROR) File not received/File corrupt. Aborting.\r\n");
        return;
    } else {
        console.printf("(INFO) File successfully send.\r\n");
    }
}

void Uploader::uploadDirectory(const char *dirpath, PS::TcpClient data_client, PS::TcpClient console) {
    // Get source directory name
    char tmpSource[512] = "";

    PS2::strcat(tmpSource, dirpath);

    char name[128] = "";
    int pointer = 0;

    // Check if there is a "/" at the end
    if (tmpSource[PS2::strlen(tmpSource) - 1] == '/')
        //Remove "/" at the end
        tmpSource[PS2::strlen(tmpSource) - 1] = '\0';

    // Get directory name
    for (int i = PS2::lastIndexOf(tmpSource, '/') + 1; i < PS2::strlen(tmpSource); i++) {
        if (tmpSource[i] != '/') {
            name[pointer] = tmpSource[i];
            pointer++;
        }
    }

    // Add "/" at the end
    PS2::strcat(name, "/");
    PS2::strcat(tmpSource, "/");

    // Send type -> Directory
    char type[2] = "d";
    data_client.send(type, 1);

    // Send the directory name to the client
    data_client.send(name, 128);
    console.printf("(INFO) DIRPATH:");
    console.printf(tmpSource);
    console.printf("\r\n");

    // Wait for answer from client
    console.printf("(INFO) Waiting for response from client...\r\n");
    char answer[5];
    data_client.read(answer, 4);
    if (PS2::strcmp(answer, "D_OK") != 0) {
        console.printf(answer);
        console.printf("(ERROR) Directory name not received. Aborting.\r\n");
        return;
    } else {
        console.printf("(INFO) Directory name successfully send.\r\n");
    }

    // Open the source directory
    int dir = PS::open(dirpath, O_RDONLY, 0);

    // Iterate through the directory and check for files and directories
    if (dir > 0) {
        char buffer[1024];
        int nread = 0;

        while ((nread = PS::getdents(dir, buffer, sizeof(buffer))) > 0) {
            struct dirent *dent = (struct dirent *) buffer;

            while (dent->d_fileno) {

                if (dent->d_reclen == 0)
                    break;
                nread -= dent->d_reclen;
                if (nread < 0)
                    break;

                // Check if it's a directory
                if (dent->d_type == DT_DIR) {
                    //Skip /. and /..
                    if (PS2::strcmp(dent->d_name, ".") != 0 && PS2::strcmp(dent->d_name, "..") != 0) {
                        // Build new source path
                        char newDirPath[512] = "";
                        PS2::strcat(newDirPath, tmpSource);
                        PS2::strcat(newDirPath, dent->d_name);

                        // Recursively upload directories
                        uploadDirectory(newDirPath, data_client, console);
                    }
                } else if (dent->d_type == DT_REG) {
                    // Build filepath
                    char newFilePath[512] = "";
                    PS2::strcat(newFilePath, tmpSource);
                    PS2::strcat(newFilePath, dent->d_name);

                    // Upload file
                    uploadFile(newFilePath, data_client, console);
                }
                dent = (struct dirent *) ((char *) dent + dent->d_reclen);

            }
            memset(buffer, 0, sizeof(buffer));
        }
    }

    PS::close(dir);

    // Tell the client to go up in the directory tree
    char cmd[2] = "r";
    data_client.send(cmd, 1);
}

bool Uploader::uploadHandler(PS::TcpClient client, const char *path, uint16_t port, bool isFile) {
    // Create TCP server
    PS::TcpServer server = PS::TcpServer();

    if (!server.listen(port))
        return false;

    // Waiting for client connection dialog
    PS::Sce::MsgDialog::Initialize();
    PS::Sce::MsgDialogUserMessage waitingDialog = PS::Sce::MsgDialogUserMessage("Waiting for client connection...",
                                                                                PS::Sce::MsgDialog::ButtonType::NONE);
    waitingDialog.open();

    // Accept connection
    PS::TcpClient upload_client = server.accept();

    // Close waiting for client dialog
    waitingDialog.close();
    PS::Sce::MsgDialog::Terminate();

    // Check if it's a file
    if (isFile)
        uploadFile(path, upload_client, client);
    else
        uploadDirectory(path, upload_client, client);

    // Send exit Message
    char exitMsg[2] = "e";
    upload_client.send(exitMsg, 1);

    // Close connection
    upload_client.disconnect();
    server.disconnect();

    return true;
}

void Uploader::setProgress(PS::Sce::MsgDialogProgressBar dialog, size_t uploaded, size_t total) {
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

void Uploader::sizeTtoCharArray(size_t x, char output[]) {
    // Recursively call function when > 10
    if (x >= 10)
        sizeTtoCharArray(x / 10, output);

    // Get individual digit
    int digit = x % 10;

    // Parse int to char*
    switch (digit) {
        case 0:
            PS2::strcat(output, "0");
            break;
        case 1:
            PS2::strcat(output, "1");
            break;
        case 2:
            PS2::strcat(output, "2");
            break;
        case 3:
            PS2::strcat(output, "3");
            break;
        case 4:
            PS2::strcat(output, "4");
            break;
        case 5:
            PS2::strcat(output, "5");
            break;
        case 6:
            PS2::strcat(output, "6");
            break;
        case 7:
            PS2::strcat(output, "7");
            break;
        case 8:
            PS2::strcat(output, "8");
            break;
        case 9:
            PS2::strcat(output, "9");
            break;
    }
}