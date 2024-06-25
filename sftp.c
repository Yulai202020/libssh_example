#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <string.h>
#include "string.c"

char* readFile(const char *filePath) {
    FILE *file = fopen(filePath, "rb"); // Open the file in binary mode
    if (file == NULL) {
        perror("Could not open file");
        return NULL;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Allocate memory for the file content (+1 for null terminator)
    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    // Read the file content into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror("File read error");
        free(buffer);
        fclose(file);
        return NULL;
    }

    // Null-terminate the buffer
    buffer[fileSize] = '\0';

    // Close the file
    fclose(file);

    return buffer;
}
char* readfile(char* path) {
    FILE* ptr = fopen("test.txt", "a+");
    if (NULL == ptr) {
        printf("file can't be opened \n");
    }

    char str[50];
    int count = 0;
    char* full_string = (char *)malloc(count);
    memset(full_string, '\0', 1);
 
    while (fgets(str, 50, ptr) != NULL) {
        count += 50;
        char* tmp = (char*) malloc(count);
        strcpy(tmp, full_string);
        strcat(tmp, str);
        full_string = tmp;
    }

    // full_string[count+1] = '\0';
    printf(full_string);
 
    fclose(ptr);
    return full_string;
}

int mysftp_mkdir(ssh_session session, sftp_session sftp, char* folder_name) {
    int rc = sftp_mkdir(sftp, folder_name, S_IRWXU);
    
    if (rc != SSH_OK) {
        if (sftp_get_error(sftp) != SSH_FX_FILE_ALREADY_EXISTS) {
            fprintf(stderr, "Can't create directory: %s\n", ssh_get_error(session));
            return rc;
        }
    }

    return SSH_OK;
}

int sftp_helloworld(ssh_session session, char* src, char* dst, char* folder_dst) {
    sftp_session sftp;
    int rc, nwritten;
 
    sftp = sftp_new(session);
    if (sftp == NULL) {
        fprintf(stderr, "Error allocating SFTP session: %s\n", ssh_get_error(session));
        return SSH_ERROR;
    }
 
    rc = sftp_init(sftp);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error initializing SFTP session: code %d.\n", sftp_get_error(sftp));
        sftp_free(sftp);
        return rc;
    }

    mysftp_mkdir(session, sftp, folder_dst);

    int access_type = O_WRONLY | O_CREAT | O_TRUNC;
    sftp_file file;
    const char *helloworld = readFile(src);
    int length = strlen(helloworld);

    if (folder_dst[strlen(folder_dst)-1] != '/') {
        folder_dst = stringSum(folder_dst, "/");
    }

    char* full_dst = stringSum(folder_dst, dst);

    file = sftp_open(sftp, full_dst, access_type, S_IRWXU);
    if (file == NULL) {
        fprintf(stderr, "Can't open file for writing: %s\n", ssh_get_error(session));
        return SSH_ERROR;
    }

    nwritten = sftp_write(file, helloworld, length);
    if (nwritten != length) {
        fprintf(stderr, "Can't write data to file: %s\n", ssh_get_error(session));
        sftp_close(file);
        return SSH_ERROR;
    }

    rc = sftp_close(file);
    if (rc != SSH_OK) {
        fprintf(stderr, "Can't close the written file: %s\n", ssh_get_error(session));
        return rc;
    }
 
    sftp_free(sftp);
    return SSH_OK;
}


int authenticate_pubkey(ssh_session session, const char* username) {
    int rc = ssh_userauth_publickey_auto(session, username, NULL);

    if (rc == SSH_AUTH_ERROR) {
        fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
        return SSH_AUTH_ERROR;
    }

    return rc;
}


int main() {
    ssh_session session;
    int verbosity = 0;

    int port = 22; // standartly its 22 but if you moved openssh server to another port change it
    const char* hostname = "127.0.0.1"; // Ip address
    const char* username = "root"; // username
    const char* password = "qaz123ZX"; // password

    // Create SSH session object
    session = ssh_new();
    if (session == NULL) {
        fprintf(stderr, "Error creating SSH session\n");
        return -1;
    }

    // Set SSH options
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(session, SSH_OPTIONS_HOST, hostname);

    // Connect to SSH server
    int rc = ssh_connect(session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting to %s: %s\n", hostname, ssh_get_error(session));
        ssh_free(session);
        return -1;
    }

    // Authenticate with username and password
    authenticate_pubkey(session, username);

    // Execute command
    ssh_channel channel = ssh_channel_new(session);
    if (channel == NULL) {
        fprintf(stderr, "Error creating channel\n");
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error opening channel: %s\n", ssh_get_error(session));
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    sftp_helloworld(session, "tmp.txt", "helloworld.txt", "/root/helloworld");

    // Close channel
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    // Disconnect and free SSH session
    ssh_disconnect(session);
    ssh_free(session);

    return 0;
}
