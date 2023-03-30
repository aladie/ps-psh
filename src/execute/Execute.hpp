#pragma once

#include <types.hpp>
#include "mast1c0re.hpp"


class Execute
{
public:
    static void ls(PS::TcpClient client, const char *directory);
    static void notification(PS::TcpClient client, const char *notification);
    static void upload(PS::TcpClient client, const char *directory, const char *filename, int DOWNLOADER_PORT);
    static void download(PS::TcpClient client, const char *directory, const char *filename, int UPLOADER_PORT);
    static void play(PS::TcpClient client, const char *directory, const char *filepath, const char *configpath);
    static void pwd(PS::TcpClient client, const char *directory);
    static void rm(PS::TcpClient client, const char *directory, const char *filepath);
    static void mkdir(PS::TcpClient client, const char *directory, const char *filepath);
    static void cd(PS::TcpClient client, char directory[256], char new_directory[256]);
    static void cp(PS::TcpClient client, const char *directory, const char *source, const char *dest);
    static void mv(PS::TcpClient client, const char *directory, const char *source, const char *dest);
    static void help(PS::TcpClient client);
};