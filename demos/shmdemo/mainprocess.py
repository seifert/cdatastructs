
import logging
import multiprocessing
import os
import signal

import sysv_ipc

from shmdemo.common import configure_logging
from shmdemo.serviceprocess import indexer
from shmdemo.workerprocess import worker

SHM_KEY = None

SHM_SIZE = 1024 * 1024 * 16


def main():
    configure_logging()
    logger = logging.getLogger('main')

    logger.info("Start main process")

    pid = os.getpid()
    shmlock = multiprocessing.Lock()
    shm = sysv_ipc.SharedMemory(
        SHM_KEY, flags=sysv_ipc.IPC_CREX, mode=0o600, size=SHM_SIZE)
    try:
        shmlock.acquire()

        for unused in range(4):
            worker_process = multiprocessing.Process(
                target=worker, args=(pid, shm.key, shmlock), daemon=True)
            worker_process.start()

        indexer_process = multiprocessing.Process(
            target=indexer, args=(pid, shm.key, shmlock), daemon=True)
        indexer_process.start()
    finally:
        shm.detach()

    try:
        signal.pause()
    except KeyboardInterrupt:
        pass

    logger.info("Stopping main process")
