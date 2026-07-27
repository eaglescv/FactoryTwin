# FactoryTwin Sensor Simulator

Local stand-in for the real sensor/PLC feed. Pushes JSON readings for
equipment `EQ-01` (`temperature`, `pressure`) over a plain WebSocket at
`ws://127.0.0.1:8765`, matching the schema `FWebSocketSensorSource` expects:

```json
{"equipmentId":"EQ-01","sensorKey":"temperature","value":72.5,"ts":"2026-07-27T14:03:00Z"}
```

## Setup

Requires Python 3.9+.

```bash
cd Sim
python -m venv .venv
.venv\Scripts\activate        # Windows
# source .venv/bin/activate   # macOS/Linux
pip install -r requirements.txt
```

## Run

Windows: double-click `run_simulator.bat` (or run it from a terminal). It
installs/updates `requirements.txt` and launches the simulator, so you don't
need to open PowerShell and type commands each time.

```bash
python simulator.py
```

Console output shows every value pushed, e.g.:

```
FactoryTwin simulator listening on ws://127.0.0.1:8765
[+] client connected (1 total)
EQ-01/temperature = 71.32
EQ-01/pressure = 29.87
EQ-01/temperature = 86.10 !! THRESHOLD
```

Stop with `Ctrl+C`.

### Fast mode (for quickly eyeballing UI/color transitions)

Set `FACTORYTWIN_SIM_FAST=1` to push every ~0.1-0.2s and cross thresholds
much more often (35% of ticks instead of 5%), instead of waiting on the
normal cadence:

```powershell
$env:FACTORYTWIN_SIM_FAST="1"; python simulator.py
```

```bash
FACTORYTWIN_SIM_FAST=1 python simulator.py   # macOS/Linux
```

## Behavior

- Each sensor value does a bounded random walk around its baseline (~1-2
  updates/sec per sensor; much faster in fast mode, see above).
- About 5% of ticks apply a larger kick so the value occasionally crosses
  its threshold (`temperature` >= 85, `pressure` >= 45) — useful for
  eyeballing that alerting logic downstream actually fires. Fast mode raises
  this to 35%.
- Values are clamped to a sane min/max so the walk can't run away.
- The server accepts multiple simultaneous clients (e.g. UE PIE + a debug
  client) and broadcasts the same stream to all of them.
