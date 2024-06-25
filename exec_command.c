#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libssh/libssh.h>

int authenticate_password(ssh_session session, const char* username) {
    char *password;
    int rc;

    password = getpass("Enter your password: ");
    rc = ssh_userauth_password(session, username, password);
    if (rc == SSH_AUTH_ERROR) {
        fprintf(stderr, "Authentication failed: %s\n",
        ssh_get_error(session));
        return SSH_AUTH_ERROR;
    }

    return rc;
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
    const char* command = "ls -l"; // command to run on remote server
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

    rc = ssh_channel_request_exec(channel, command);

    if (rc != SSH_OK) {
        fprintf(stderr, "Error executing command: %s\n", ssh_get_error(session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    // Read command output
    char buffer[256];
    int nbytes;
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, nbytes, stdout);
    }

    // Close channel
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    // Disconnect and free SSH session
    ssh_disconnect(session);
    ssh_free(session);

    return EXIT_SUCCESS;
}
