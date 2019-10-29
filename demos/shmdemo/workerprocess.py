
import logging
import os
import random
import signal
import sys
import time

import sysv_ipc

from cdatastructs.hashmap import Int2Int

from shmdemo.common import configure_logging, StopProcess


def worker(parentpid, shmkey, shmlock):
    configure_logging()
    logger = logging.getLogger('worker')

    logger.info("Worker has been started with pid %d", os.getpid())

    shm = sysv_ipc.SharedMemory(shmkey, flags=0, mode=0o600, size=0)
    try:
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        while 1:
            try:
                if shmlock.acquire(block=True, timeout=1):
                    try:
                        data = Int2Int.from_ptr(shm.address)
                        logger.info(
                            "Processing %d keys: %s", len(data), list(data))
                    finally:
                        shmlock.release()
                else:
                    raise logger.warning("SHM is not available")
            except Exception:
                logger.exception("Processing data error")

            for unused in range(random.randint(0, 10)):
                if os.getppid() != parentpid:
                    raise StopProcess
                time.sleep(0.5)
    except StopProcess:
        logger.info("Stopping worker process")
    finally:
        shm.detach()

    sys.exit(0)
