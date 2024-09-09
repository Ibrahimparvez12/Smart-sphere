import struct

class BLEParse:
    def __init__(self):
        self.parsed_data = {}

    def parse_float_triplet(self, raw_data):
        """Parse 3 floats from raw binary data."""
        try:
            return struct.unpack("fff", raw_data)
        except Exception as e:
            print(f"Error parsing float triplet: {e}")
            return None

    def parse_single_float(self, raw_data):
        """Parse 1 float from raw binary data."""
        try:
            return struct.unpack("f", raw_data)[0]
        except Exception as e:
            print(f"Error parsing single float: {e}")
            return None
