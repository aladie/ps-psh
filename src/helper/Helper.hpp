#pragma once

#include <types.hpp>
#include "mast1c0re.hpp"
#include <array>


class Helper
{
public:
    static bool isFile(const char *directory, const char *filename);
    static bool isDirectory(const char *directory, const char *dirname);
    static bool copyFile(const char *directory, const char *source, const char *destination);
    static bool copyDirectory(const char *directory, const char *source, const char *destination);
    static void playPS2ISO(const char *gamepath, const char *confpath);
    static int firstIndexOf(const char *string, char comp);
    static int isValidMultipleDirectoryFile(const char *directory, const char *multiple_directory_file);
    static int isValidMultipleDirectoryCreatableFile(const char *directory, const char *multiple_directory_file);
    static int isValidMultipleDirectoryDirectory(const char *directory, const char *multiple_directory_directory);
    static void setProgress(PS::Sce::MsgDialogProgressBar dialog, size_t progress, size_t total);
};