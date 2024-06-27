#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>

void error(ssh_session session) {
    fprintf(stderr, "Error: %s\n", ssh_get_error(session));
    ssh_disconnect(session);
    ssh_free(session);
    exit(EXIT_FAILURE);
}

int main() {
    ssh_session session;
    ssh_channel channel;
    int rc;

    // Initialize session
    session = ssh_new();
    if (session == NULL) {
        fprintf(stderr, "Error creating SSH session\n");
        exit(EXIT_FAILURE);
    }

    // Set server options
    ssh_options_set(session, SSH_OPTIONS_HOST, "192.168.100.37");
    ssh_options_set(session, SSH_OPTIONS_USER, "yulai");

    // Connect to server
    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        error(session);
    }

    // Authenticate
    rc = ssh_userauth_password(session, NULL, "qaz123ZX");
    if (rc != SSH_AUTH_SUCCESS) {
        error(session);
    }

    // Open a channel
    channel = ssh_channel_new(session);
    if (channel == NULL) {
        error(session);
    }

    // Request reverse remote port forwarding on the server
    rc = ssh_channel_open_forward(channel, "192.168.100.37", 8080, "192.168.100.37", 8000);
    if (rc != SSH_OK) {
        error(session);
    }

    // Keep the session alive
    while (ssh_channel_is_open(channel)) {
        char buffer[256];
        int nbytes;

        while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
            // Process the data read from the channel
            fwrite(buffer, 1, nbytes, stdout);
        }

        if (nbytes < 0) {
            fprintf(stderr, "Error reading from channel: %s\n", ssh_get_error(session));
        }
    }

    // Cleanup
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);

    return 0;
}
