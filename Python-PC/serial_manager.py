"""Serial bridge to the FRDM-KL46Z firmware"""
import queue
import re
import threading
import time

import serial

from config import (
    BAUD_RATE,
    PERIODIC_SYNC_ALPHA,
    PERIODIC_SYNC_INTERVAL_S,
    SERIAL_PORT,
    SERIAL_TIMEOUT,
)

ACK_RE   = re.compile(r"^ACK:(\d+)$")
H_RE     = re.compile(r"^H:(\d+):([PGM]):(\d+)$")
M_RE     = re.compile(r"^M:(\d+)$")
BUSY_RE  = re.compile(r"^BUSY:(\d+)$")
SWING_RE = re.compile(r"^SWING:([UDLR]):(\d+)$")


def parse_v2_line(line):
    if line == "BOOT":
        return {"type": "boot"}
    m = ACK_RE.match(line)
    if m:
        return {"type": "ack", "board_ms": int(m.group(1))}
    m = H_RE.match(line)
    if m:
        return {"type": "hit",
                "idx": int(m.group(1)),
                "grade": m.group(2),
                "actual_ms": int(m.group(3))}
    m = M_RE.match(line)
    if m:
        return {"type": "miss", "idx": int(m.group(1))}
    m = BUSY_RE.match(line)
    if m:
        return {"type": "busy", "idx": int(m.group(1))}
    m = SWING_RE.match(line)
    if m:
        return {"type": "swing", "dir": m.group(1), "t_ms": int(m.group(2))}
    return None


class SerialDispatcher:
    """Single reader thread for the serial port."""

    def __init__(self, ser):
        self.ser = ser
        self.ack_q = queue.Queue()
        self.event_q = queue.Queue()
        self._stop = threading.Event()
        self._tx_lock = threading.Lock()
        self._reader = threading.Thread(target=self._read_loop, daemon=True)

    def start(self):
        self._reader.start()

    def stop(self, timeout=1.0):
        self._stop.set()
        self._reader.join(timeout=timeout)

    def write(self, data):
        with self._tx_lock:
            self.ser.write(data)
            self.ser.flush()

    def _read_loop(self):
        while not self._stop.is_set():
            try:
                raw = self.ser.readline()
            except (serial.SerialException, OSError):
                return
            if not raw:
                continue
            line = raw.decode("utf-8", errors="replace").strip()
            if not line:
                continue
            ev = parse_v2_line(line)
            if ev is None:
                continue                # drop INIT/MODE/etc.
            (self.ack_q if ev["type"] == "ack" else self.event_q).put(ev)


class PeriodicSync:
    """Background SYN/ACK + EMA-smoothed offset"""

    def __init__(self, dispatcher, interval_sec, alpha, ack_timeout=1.0):
        self.dispatcher = dispatcher
        self.interval = interval_sec
        self.alpha = alpha
        self.ack_timeout = ack_timeout
        self._offset_ms = None
        self._lock = threading.Lock()
        self._stop = threading.Event()
        self._thread = None

    def _sync_once(self):
        # Drain any stale ACKs
        while True:
            try:
                self.dispatcher.ack_q.get_nowait()
            except queue.Empty:
                break

        t0 = time.time()
        self.dispatcher.write(b"SYN\n")
        try:
            ev = self.dispatcher.ack_q.get(timeout=self.ack_timeout)
        except queue.Empty:
            raise TimeoutError(f"No ACK within {self.ack_timeout:.1f}s")
        t1 = time.time()
        rtt_ms = (t1 - t0) * 1000.0
        host_mid_ms = (t0 + t1) / 2.0 * 1000.0
        offset_ms = host_mid_ms - ev["board_ms"]
        return offset_ms, rtt_ms

    def start(self, do_initial_sync=True):
        if do_initial_sync:
            offset, _ = self._sync_once()
            with self._lock:
                self._offset_ms = offset
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def stop(self, timeout=2.0):
        self._stop.set()
        if self._thread:
            self._thread.join(timeout=timeout)

    def _run(self):
        while not self._stop.wait(self.interval):
            try:
                new_off, _ = self._sync_once()
            except (TimeoutError, serial.SerialException, OSError):
                continue
            with self._lock:
                if self._offset_ms is None:
                    self._offset_ms = new_off
                else:
                    self._offset_ms = (
                        self.alpha * new_off
                        + (1.0 - self.alpha) * self._offset_ms
                    )

    @property
    def offset_ms(self):
        with self._lock:
            return self._offset_ms

    def host_to_board_ms(self, host_ms):
        with self._lock:
            if self._offset_ms is None:
                raise RuntimeError("PeriodicSync: not synced yet")
            return int(host_ms - self._offset_ms)

    def force_resync(self):
        offset, _ = self._sync_once()
        with self._lock:
            self._offset_ms = offset
        return offset


class SerialManager:
    """High-level façade: connect, sync clock, send beats, poll events."""

    def __init__(self, port=SERIAL_PORT, baud=BAUD_RATE):
        self.port = port
        self.baud = baud
        self.ser = None
        self.dispatcher = None
        self.sync = None

    def connect(self):
        try:
            self.ser = serial.Serial()
            self.ser.port = self.port
            self.ser.baudrate = self.baud
            self.ser.timeout = SERIAL_TIMEOUT
            self.ser.dtr = False
            self.ser.rts = False
            self.ser.open()
        except serial.SerialException as e:
            print(f"[Serial] Failed to open {self.port}: {e}")
            self.ser = None
            return False
        time.sleep(1.0)                     # let board finish boot prints
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

        self.dispatcher = SerialDispatcher(self.ser)
        self.dispatcher.start()

        time.sleep(0.3)
        while True:
            try:
                self.dispatcher.event_q.get_nowait()
            except queue.Empty:
                break

        self.sync = PeriodicSync(
            self.dispatcher,
            interval_sec=PERIODIC_SYNC_INTERVAL_S,
            alpha=PERIODIC_SYNC_ALPHA,
        )
        try:
            self.sync.start(do_initial_sync=True)
        except TimeoutError as e:
            print(f"[Serial] Initial SYN failed: {e}")
            self.disconnect()
            return False
        print(
            f"[Serial] Connected to {self.port} @ {self.baud}, "
            f"offset={self.sync.offset_ms:.1f} ms"
        )
        return True

    def disconnect(self):
        if self.sync:
            self.sync.stop()
            self.sync = None
        if self.dispatcher:
            self.dispatcher.stop()
            self.dispatcher = None
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
            self.ser = None

    def is_connected(self):
        return (self.ser is not None
                and self.ser.is_open
                and self.sync is not None
                and self.sync.offset_ms is not None)

    def send_beat(self, idx, direction, host_target_ms):
        if not self.is_connected():
            return
        direction = direction.upper()
        if direction not in ("U", "D", "L", "R"):
            return
        try:
            board_t = self.sync.host_to_board_ms(host_target_ms)
        except RuntimeError:
            return                          # not synced yet
        msg = f"B:{int(idx)}:{direction}:{int(board_t)}\n"
        self.dispatcher.write(msg.encode("ascii"))

    def poll_events(self):
        """Drain dispatcher's event queue. Returns list of event dicts."""
        events = []
        if self.dispatcher is None:
            return events
        while True:
            try:
                events.append(self.dispatcher.event_q.get_nowait())
            except queue.Empty:
                break
        return events
