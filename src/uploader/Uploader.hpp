#pragma once

#include <types.hpp>
#include "mast1c0re.hpp"

#define MAGIC 0x0000EA6E
#define UPLOAD_CHUNK_SIZE 4096
#define UPLOAD_BAR_UPDATE_FREQUENCY 2500

class Uploader
{
public:
    static bool uploadHandler(PS::TcpClient client, const char *path, uint16_t port, bool isFile);
private:
    static void uploadFile(const char* filepath, PS::TcpClient data_client, PS::TcpClient console);
    static void uploadDirectory(const char* dirpath, PS::TcpClient data_client, PS::TcpClient console);
    static void setProgress(PS::Sce::MsgDialogProgressBar dialog, size_t uploaded, size_t total);
    static void sizeTtoCharArray(size_t x, char output[]);
};