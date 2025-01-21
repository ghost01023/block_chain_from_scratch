#ifndef SYSTEM_SETUP_H
#define SYSTEM_SETUP_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <minizip/zip.h>
#include <minizip/unzip.h>

int get_windows_architecture()
{
    if ((size_t)-1 > 0xffffffffUL)
    {
        return 64;
    }
    else
    {
        return 32;
    }
}

int extract_zip_file(const char *zip_filename)
{
    const char *output_dir = "C:/SQLite/";

    // Create the output directory if it does not exist
    if (CreateDirectory(output_dir, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("Failed to create directory: %s\n", output_dir);
        return 1;
    }

    // Open the zip file
    unzFile zip = unzOpen(zip_filename);
    if (zip == NULL)
    {
        printf("Failed to open zip file\n");
        return 1;
    }

    // Get the number of files in the zip
    if (unzGoToFirstFile(zip) != UNZ_OK)
    {
        printf("Failed to locate the first file in zip\n");
        unzClose(zip);
        return 1;
    }

    do
    {
        char filename[256];
        unz_file_info file_info;

        // Get the file info
        if (unzGetCurrentFileInfo(zip, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK)
        {
            printf("Failed to get file info\n");
            break;
        }

        // Construct the full path for extraction
        char file_path[MAX_PATH];
        snprintf(file_path, sizeof(file_path), "%s%s", output_dir, filename);

        // Open the output file
        FILE *output_file = fopen(file_path, "wb");
        if (output_file == NULL)
        {
            printf("Failed to open file for writing: %s\n", file_path);
            break;
        }

        // Open the file from the zip
        if (unzOpenCurrentFile(zip) != UNZ_OK)
        {
            printf("Failed to open file from zip: %s\n", filename);
            fclose(output_file);
            break;
        }

        // Buffer for reading from the zip
        char buffer[8192];
        int bytes_read;
        while ((bytes_read = unzReadCurrentFile(zip, buffer, sizeof(buffer))) > 0)
        {
            fwrite(buffer, 1, bytes_read, output_file);
        }

        fclose(output_file);
        unzCloseCurrentFile(zip);

    } while (unzGoToNextFile(zip) == UNZ_OK);

    unzClose(zip);
    printf("Extraction complete.\n");
    return EXIT_SUCCESS;
}

void add_to_path(const char *folder)
{
    // Retrieve the current user PATH variable
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        printf("Error: Unable to open registry key.\n");
        return;
    }

    DWORD path_size;
    result = RegQueryValueEx(hKey, "PATH", NULL, NULL, NULL, &path_size);
    if (result != ERROR_SUCCESS)
    {
        printf("Error: Unable to retrieve current PATH.\n");
        RegCloseKey(hKey);
        return;
    }

    // Allocate memory for the existing PATH value
    char *path = malloc(path_size);
    if (path == NULL)
    {
        printf("Error: Memory allocation failed.\n");
        RegCloseKey(hKey);
        return;
    }

    // Get the current PATH value
    result = RegQueryValueEx(hKey, "PATH", NULL, NULL, (LPBYTE)path, &path_size);
    if (result != ERROR_SUCCESS)
    {
        printf("Error: Unable to read current PATH.\n");
        free(path);
        RegCloseKey(hKey);
        return;
    }

    // Check if the folder is already in PATH
    if (strstr(path, folder) != NULL)
    {
        printf("Folder is already in the PATH.\n");
        free(path);
        RegCloseKey(hKey);
        return;
    }

    // Calculate the new size for the PATH value
    size_t new_path_size = strlen(path) + strlen(folder) + 2; // +2 for the semicolon and null terminator
    char *new_path = malloc(new_path_size);
    if (new_path == NULL)
    {
        printf("Error: Memory allocation failed.\n");
        free(path);
        RegCloseKey(hKey);
        return;
    }

    // Prepare the new PATH string (existing PATH + new folder)
    snprintf(new_path, new_path_size, "%s;%s", path, folder);

    // Set the new PATH value
    result = RegSetValueEx(hKey, "PATH", 0, REG_EXPAND_SZ, (const BYTE *)new_path, new_path_size);
    if (result == ERROR_SUCCESS)
    {
        printf("Successfully added '%s' to the PATH permanently.\n", folder);
    }
    else
    {
        printf("Error: Unable to update PATH in the registry.\n");
    }

    // Clean up
    free(path);
    free(new_path);
    RegCloseKey(hKey);
}

// int main()
// {

//     return 0;
// }


#endif