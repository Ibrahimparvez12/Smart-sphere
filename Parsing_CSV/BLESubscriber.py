import asyncio
import logging
from bleak import BleakClient
from BLEParse import BLEParse

logging.basicConfig(level=logging.DEBUG)

BLE_DEVICE_ADDRESS = "04:A3:FB:CD:39:F4"

BLE_UUIDS = {
    "Acceleration": "a6edb165-4f29-4c2e-8c07-5472acdcfa0b",
    "Gyroscope": "a6edb166-4f29-4c2e-8c07-5472acdcfa0b",
    "Magnetometer": "a6edb167-4f29-4c2e-8c07-5472acdcfa0b",
    "Linear Acceleration": "a6edb168-4f29-4c2e-8c07-5472acdcfa0b",
    "Gravity": "a6edb169-4f29-4c2e-8c07-5472acdcfa0b",
    "Temperature": "a6edb16a-4f29-4c2e-8c07-5472acdcfa0b",
    "Orientation": "a6edb16b-4f29-4c2e-8c07-5472acdcfa0b",
}


class BLESubscriber:
    def __init__(self, address):
        self.address = address
        self.client = BleakClient(self.address)
        self.ble_parser = BLEParse()
        self.connected = False

    async def connect(self):
        """Connect to the BLE device with a timeout."""
        try:
            await self.client.connect(timeout=30)
            self.connected = True
            print(f"Connected to BLE device with address: {self.address}")
        except asyncio.TimeoutError:
            print("Connection timed out. Please check the device and try again.")
            self.connected = False
        except Exception as e:
            print(f"Failed to connect: {e}")
            self.connected = False

        return self.connected

    async def start_notifications(self, characteristic_uuid, callback):
        """Start receiving notifications for a specific characteristic."""
        if self.connected:
            await self.client.start_notify(characteristic_uuid, callback)

    async def stop_notifications(self, characteristic_uuid):
        """Stop receiving notifications for a specific characteristic."""
        if self.connected:
            await self.client.stop_notify(characteristic_uuid)

    def parse_and_print_data(self, raw_data, description):
        """Parse and print sensor data based on the characteristic."""
        parsed_values = None

        if description in ["Acceleration", "Gyroscope", "Magnetometer", "Linear Acceleration", "Gravity"]:
            parsed_values = self.ble_parser.parse_float_triplet(raw_data)
        elif description == "Temperature":
            parsed_values = self.ble_parser.parse_single_float(raw_data)
        elif description == "Orientation":
            parsed_values = self.ble_parser.parse_float_triplet(raw_data)

        if parsed_values is None:
            print(f"Error parsing data for characteristic: {description}")
        else:
            print(f"{description}: {parsed_values}")
