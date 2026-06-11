import serial
import time
import os

PORT = "COM6"
BAUD = 2_000_000

TOTAL_PACKETS = 5_000_000      # 10 MB total, since each packet is 2 bytes
BATCH_PACKETS = 16_384         # 32 KB per write
VALUE = 32768                  # fixed 16-bit value

def value_to_bytes(value: int) -> bytes:
    value = max(0, min(65535, int(value)))
    return bytes([(value >> 8) & 0xFF, value & 0xFF])

def main():
    packet = value_to_bytes(VALUE)
    full_chunk = packet * BATCH_PACKETS

    with serial.Serial(
        PORT,
        BAUD,
        timeout=0,
        write_timeout=10
    ) as ser:
        time.sleep(2.0)

        # Clear anything stale.
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        print("Unlimited serial write speed test")
        print(f"Port: {PORT}")
        print(f"Baud setting: {BAUD}")
        print(f"Total packets: {TOTAL_PACKETS}")
        print(f"Total bytes: {TOTAL_PACKETS * 2}")
        print(f"Batch packets: {BATCH_PACKETS}")
        print(f"Batch bytes: {BATCH_PACKETS * 2}")
        print()

        sent_packets = 0
        sent_bytes = 0
        writes = 0

        start = time.perf_counter()
        last_report = start
        last_packets = 0
        last_bytes = 0

        while sent_packets < TOTAL_PACKETS:
            remaining_packets = TOTAL_PACKETS - sent_packets

            if remaining_packets >= BATCH_PACKETS:
                chunk = full_chunk
                packets_this_write = BATCH_PACKETS
            else:
                packets_this_write = remaining_packets
                chunk = packet * packets_this_write

            n = ser.write(chunk)

            sent_packets += n // 2
            sent_bytes += n
            writes += 1

            now = time.perf_counter()

            if now - last_report >= 1.0:
                dt = now - last_report
                dp = sent_packets - last_packets
                db = sent_bytes - last_bytes

                print(
                    f"live: "
                    f"{dp / dt:,.0f} packets/s, "
                    f"{db / dt:,.0f} bytes/s, "
                    f"{(db * 8) / dt:,.0f} bit/s, "
                    f"total={sent_packets:,} packets"
                )

                last_report = now
                last_packets = sent_packets
                last_bytes = sent_bytes

        ser.flush()

        elapsed = time.perf_counter() - start

        print()
        print("Done")
        print(f"Sent packets: {sent_packets:,}")
        print(f"Sent bytes: {sent_bytes:,}")
        print(f"Writes: {writes:,}")
        print(f"Elapsed: {elapsed:.3f} s")
        print(f"Average packet rate: {sent_packets / elapsed:,.1f} packets/s")
        print(f"Average byte rate: {sent_bytes / elapsed:,.1f} bytes/s")
        print(f"Average bit rate: {(sent_bytes * 8) / elapsed:,.1f} bit/s")
        print(f"Average write rate: {writes / elapsed:,.1f} writes/s")

        # Send final off command.
        ser.write(value_to_bytes(0))
        ser.flush()

if __name__ == "__main__":
    main()
