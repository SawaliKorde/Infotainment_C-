//client.c

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ncurses.h>

#define SHM_NAME "/random_shm"
#define SHM_SIZE 4096

struct data_entry {
    unsigned long timestamp;
    int value;
};

void display_data(struct data_entry *data, int count) {
    initscr();
    noecho();
    curs_set(FALSE);
    int row = 0;

    while (1) {
        clear();
        mvprintw(0, 0, "Timestamp        | Value");
        mvprintw(1, 0, "----------------|------");

        for (int i = 0; i < count; i++) {
            mvprintw(i + 2, 0, "%lu | %d", data[i].timestamp, data[i].value);
        }

        refresh();
        sleep(1);
    }

    endwin();
}

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    struct data_entry *shm_region = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_region == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    display_data(shm_region, SHM_SIZE / sizeof(struct data_entry));

    return 0;
}



