#include <omp.h>

class rwlatch {
public:
    rwlatch() {
        omp_init_nest_lock(&read_lock);
        omp_init_lock(&write_lock);
        readers = 0;
        writers = 0;
    }

    ~rwlatch() {
        omp_destroy_nest_lock(&read_lock);
        omp_destroy_lock(&write_lock);
    }

    void lock_shared() {
        omp_set_nest_lock(&read_lock);
        readers++;
        if (readers == 1) {
            // First reader, block writers
            omp_set_lock(&write_lock);
        }
        omp_unset_nest_lock(&read_lock);
    }

    void unlock_shared() {
        omp_set_nest_lock(&read_lock);
        readers--;
        if (readers == 0) {
            // Last reader, unblock writers
            omp_unset_lock(&write_lock);
        }
        omp_unset_nest_lock(&read_lock);
    }

    void lock() {
        omp_set_lock(&write_lock);
    }

    void unlock() {
        omp_unset_lock(&write_lock);
    }

private:
    int readers;
    int writers;
    omp_nest_lock_t read_lock;
    omp_lock_t write_lock;
};