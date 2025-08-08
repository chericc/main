
import shutil
from enum import Enum, auto
import os
import threading
import logging
import time
from dataclasses import dataclass

class CopyStatus(Enum):
    COPYING = auto()
    COMPLETED = auto()
    ERROR = auto()

@dataclass
class StatusStructure:
    status: CopyStatus
    progress: str

class MyFileCopy:
    def __init__(self, src_path: str, dst_path: str):
        self.src_path = src_path
        self.dst_path = dst_path
        self.status = CopyStatus.COPYING
        self.bytes_total = os.path.getsize(src_path)
        self.bytes_copied = 0
        self.thread = None
        self._cancel_requested = False  # Flag to track cancellation
        
        self.start_copy()
    
    def start_copy(self):
        self.status = CopyStatus.COPYING
        self.thread = threading.Thread(target=self._copy_file)
        self.thread.daemon = True
        self.thread.start()
    
    def _copy_file(self):
        try:
            with open(self.src_path, 'rb') as src, open(self.dst_path, 'wb') as dst:
                while True:
                    if self._cancel_requested:  # Check for cancellation
                        self.status = CopyStatus.ERROR
                        dst.close()  # Ensure file is closed before removal
                        try:
                            if os.path.exists(self.dst_path):
                                os.remove(self.dst_path)
                        except Exception as e:
                            logging.error('remove failed: %s, %s', self.dst_path, str(e))
                        return
                    
                    chunk = src.read(1024 * 1024)
                    if not chunk:
                        break
                    dst.write(chunk)
                    dst.flush()
                    self.bytes_copied += len(chunk)
                    # logging.info("write %d/%d", self.bytes_copied, self.bytes_total)
            
            self.status = CopyStatus.COMPLETED
        except Exception as e:
            try:
                if os.path.exists(self.dst_path):
                    os.remove(self.dst_path)
            except Exception as e:
                logging.error('remove failed: %s, %s', self.dst_path, str(e))
            
            self.status = CopyStatus.ERROR

    def cancel(self):
        logging.warning('cancel task')
        self._cancel_requested = True
    
    def get_status(self) -> StatusStructure:
        return StatusStructure(status=self.status, progress='{}/{} kB'.format(int(self.bytes_copied / 1024), int(self.bytes_total / 1024)))
    
if __name__ == "__main__":
    
    logging.basicConfig(
        level=logging.DEBUG,
        format='%(asctime)s %(levelname)s %(filename)s:%(lineno)s:%(funcName)s %(message)s',
    )
    logging.info('begin')

    mycopy = MyFileCopy('screen_ad100.20250606.bin', 'E:/update/update.bin')
    try:     
        while True:
            stat = mycopy.get_status()
            logging.info('copy stat: %s / %s', stat.status, stat.progress)
            if stat.status != CopyStatus.COPYING:
                logging.info('copy end: %s', str(stat.status))
                break
            time.sleep(1)
    except KeyboardInterrupt as e:
        mycopy.cancel()
    
    logging.info('end')