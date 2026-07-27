"""FactoryTwin local sensor simulator.

Pushes JSON sensor readings for equipment EQ-01 over a plain WebSocket so the
UE5 FWebSocketSensorSource / USensorSubsystem slice has something to consume
without needing a real PLC / OPC UA server yet.

Run:
    pip install -r requirements.txt
    python simulator.py

Stop with Ctrl+C.
"""

import asyncio
import json
import random
from datetime import datetime, timezone

import websockets

HOST = "127.0.0.1"
PORT = 8765

EQUIPMENT_ID = "EQ-01"

# baseline: steady-state value the random walk drifts around
# step: max per-tick drift under normal conditions
# threshold: value at/above which we flag the reading as an excursion (log-only, for now)
SENSORS = {
    "temperature": {"baseline": 70.0, "value": 70.0, "step": 1.5, "threshold": 85.0, "min": 40.0, "max": 110.0},
    "pressure": {"baseline": 30.0, "value": 30.0, "step": 0.8, "threshold": 45.0, "min": 5.0, "max": 60.0},
}

connected_clients = set()


def next_value(state: dict) -> float:
    drift = random.uniform(-state["step"], state["step"])

    # occasionally kick the walk hard so the threshold gets exercised
    if random.random() < 0.05:
        drift += state["step"] * random.uniform(3, 6) * random.choice([1, -1])

    state["value"] = max(state["min"], min(state["max"], state["value"] + drift))
    return state["value"]


def make_message(sensor_key: str, value: float) -> str:
    return json.dumps(
        {
            "equipmentId": EQUIPMENT_ID,
            "sensorKey": sensor_key,
            "value": round(value, 2),
            "ts": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        }
    )


async def handler(websocket):
    connected_clients.add(websocket)
    print(f"[+] client connected ({len(connected_clients)} total)")
    try:
        # push-only simulator: drain and ignore anything the client sends
        async for _ in websocket:
            pass
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        connected_clients.discard(websocket)
        print(f"[-] client disconnected ({len(connected_clients)} total)")


async def broadcast_loop():
    while True:
        for sensor_key, state in SENSORS.items():
            value = next_value(state)
            message = make_message(sensor_key, value)

            if connected_clients:
                await asyncio.gather(
                    *(client.send(message) for client in list(connected_clients)),
                    return_exceptions=True,
                )

            flag = " !! THRESHOLD" if value >= state["threshold"] else ""
            print(f"{EQUIPMENT_ID}/{sensor_key} = {value:.2f}{flag}")

        # ~1-2 updates/sec per sensor
        await asyncio.sleep(random.uniform(0.5, 1.0))


async def main():
    print(f"FactoryTwin simulator listening on ws://{HOST}:{PORT}")
    async with websockets.serve(handler, HOST, PORT):
        await broadcast_loop()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nSimulator stopped.")
