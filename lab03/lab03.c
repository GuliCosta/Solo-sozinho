#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHM_SIZE 1024

int main() {
    int shmid;
    key_t key = 5678;
    int *shm, *ptr;
    int value = 1;

    // Create the shared memory segment
    if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach the segment to our data space
    if ((shm = shmat(shmid, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    // Write the initial value to the shared memory
    ptr = shm;
    *ptr = value;

    // Fork the process
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("Child process is running...\n");

        // Access the shared memory and modify the value
        ptr = shm;
        *ptr += 2;
        printf("Child process modified the value to %d\n", *ptr);

        // Detach the shared memory segment from our data space
        if (shmdt(shm) == -1) {
            perror("shmdt");
            exit(1);
        }

        // Terminate the child process
        exit(0);
    } else {
        // Parent process
        printf("Parent process is running...\n");

        // Wait for the child process to finish
        wait(NULL);

        // Access the shared memory and modify the value again
        ptr = shm;
        *ptr *= 4;
        printf("Parent process modified the value to %d\n", *ptr);

        // Detach and remove the shared memory segment
        if (shmdt(shm) == -1) {
            perror("shmdt");
            exit(1);
        }
        if (shmctl(shmid, IPC_RMID, 0) == -1) {
            perror("shmctl");
            exit(1);
        }
    }

    return 0;
}
