#include "Uploader.hpp"

bool Uploader::upload(const char *filepath, uint16_t port) {
    // Create TCP server
    PS::TcpServer server = PS::TcpServer();
    if (!server.listen(port))
        return false;

    // Waiting for game file dialog
    PS::Sce::MsgDialog::Initialize();
    PS::Sce::MsgDialogUserMessage waitingDialog = PS::Sce::MsgDialogUserMessage("Waiting for client connection...",
                                                                                PS::Sce::MsgDialog::ButtonType::NONE);
    waitingDialog.open();

    // Accept connection
    PS::TcpClient client = server.accept();

    // Open the source filepath
    int fd_src = PS::open(filepath, O_RDONLY, 0);

    // Get the filesize
    size_t filesize = PS::Filesystem::getFileSize(filepath);

    // Create the file chunk buffer and total send offset
    size_t bufferSize = UPLOAD_CHUNK_SIZE;
    size_t offset = 0;
    char buffer[bufferSize];

    // Close waiting for client dialog
    waitingDialog.close();
    PS::Sce::MsgDialog::Terminate();

    // Show progress bar dialog
    PS::Sce::MsgDialog::Initialize();
    PS::Sce::MsgDialogProgressBar progressDialog = PS::Sce::MsgDialogProgressBar("Uploading file...");
    progressDialog.open();
    Uploader::setProgress(progressDialog, 0, filesize);

    //Send the file in chunks
    uint32_t updateBar = 0;
    while (true) {
        size_t read_count = PS::readAll(fd_src, buffer, bufferSize);
        offset += read_count;

        //Adjust buffersize if EOF
        if (read_count < bufferSize) {
            client.send(buffer, read_count);
            break;
        } else {
            client.send(buffer, bufferSize);
        }

        // Update progress bar
        if (updateBar == UPLOAD_BAR_UPDATE_FREQUENCY) {
            if (filesize != 0)
                Uploader::setProgress(progressDialog, offset, filesize);
            updateBar = 0;
        }
        updateBar++;
    }

    PS::close(fd_src);
    client.disconnect();
    server.disconnect();

    progressDialog.setValue(100);
    progressDialog.close();
    PS::Sce::MsgDialog::Terminate();

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