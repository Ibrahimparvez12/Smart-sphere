import asyncio
from BLESubscriber import BLESubscriber
BLE_UUIDS = {
    "Acceleration": "a6edb165-4f29-4c2e-8c07-5472acdcfa0b",
    "Gyroscope": "a6edb166-4f29-4c2e-8c07-5472acdcfa0b",
    "Magnetometer": "a6edb167-4f29-4c2e-8c07-5472acdcfa0b",
    "Linear Acceleration": "a6edb168-4f29-4c2e-8c07-5472acdcfa0b",
    "Gravity": "a6edb169-4f29-4c2e-8c07-5472acdcfa0b",
    "Temperature": "a6edb16a-4f29-4c2e-8c07-5472acdcfa0b",
    "Orientation": "a6edb16b-4f29-4c2e-8c07-5472acdcfa0b",
}

async def main():
    ble_subscriber = BLESubscriber("04:A3:FB:CD:39:F4")
    is_connected = await ble_subscriber.connect()
    if not is_connected:
        print("Failed to connect. Exiting.")
        return
    def create_callback(description):
        return lambda sender, data: ble_subscriber.parse_and_print_data(data, description)

    for description, uuid in BLE_UUIDS.items():
        await ble_subscriber.start_notifications(uuid, create_callback(description))

    try:
        while True:
            await asyncio.sleep(1)
    except KeyboardInterrupt:
        print("Stopping...")

    for uuid in BLE_UUIDS.values():
        await ble_subscriber.stop_notifications(uuid)

    await ble_subscriber.client.disconnect()

asyncio.run(main())
