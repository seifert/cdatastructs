
import ctypes
import logging
import os
import random
import signal
import sys
import time

import sysv_ipc

from cdatastructs.hashmap import Int2Int

from shmdemo.common import configure_logging, StopProcess


def indexer(parentpid, shmkey, shmlock):
    configure_logging()
    logger = logging.getLogger('indexer')

    logger.info("Indexer has been started with pid %d", os.getpid())

    shm_filled = False
    shm = sysv_ipc.SharedMemory(shmkey, flags=0, mode=0o600, size=0)
    try:
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        while 1:
            try:
                data = Int2Int()
                for i in range(random.randint(0, 16)):
                    entity_id = random.randint(100, 999)
                    data[entity_id] = i
                data.make_readonly()

                if shm_filled:
                    shmlock.acquire()
                try:
                    if shm.size < data.buffer_size:
                        raise RuntimeError("SHM is too small")
                    ctypes.memmove(
                        shm.address, data.buffer_ptr, data.buffer_size)
                    shm_filled = True
                finally:
                    if shm_filled:
                        shmlock.release()

                logger.info("Created %d keys: %s", len(data), list(data))
            except Exception:
                logger.exception("Indexer error")

            for unused in range(10):
                if os.getppid() != parentpid:
                    raise StopProcess
                time.sleep(1)
    except StopProcess:
        logger.info("Stopping indexer")
    finally:
        shm.detach()

    sys.exit(0)
