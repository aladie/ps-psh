#pragma once

#include <types.hpp>
#include "mast1c0re.hpp"

#define MAGIC 0x0000EA6E
#define UPLOAD_CHUNK_SIZE 4096
#define UPLOAD_BAR_UPDATE_FREQUENCY 2500

class Uploader
{
public:
    static bool upload(const char* filepath, uint16_t port);
private:
    static void setProgress(PS::Sce::MsgDialogProgressBar dialog, size_t uploaded, size_t total);
};