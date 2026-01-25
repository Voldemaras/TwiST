# Distance-Controlled Servo Behavior

## Distance → Servo Angle Mapping (30 cm range)

```
Distance (cm)     Servo Angle     Zone
─────────────────────────────────────────────
0-4 cm            0°              TOO CLOSE
5 cm              0°              Active range start
10 cm             36°
15 cm             72°
20 cm             108°
25 cm             144°
30 cm             180°            Active range end
31+ cm            180°            OUT OF RANGE
```

## Sensitivity Control (Hysteresis Example)

**HYSTERESIS = 2 cm means servo moves ONLY if distance changes >= 2 cm:**

### Example 1: Small changes (NO MOVEMENT)
```
Time    Distance    Change    Servo Action
────────────────────────────────────────────
t0      10 cm       -         Move to 36°
t1      11 cm       +1 cm     NO MOVE (< 2 cm)
t2      10 cm       -1 cm     NO MOVE (< 2 cm)
t3      9 cm        -1 cm     NO MOVE (< 2 cm)
```
**Result:** Servo stays at 36° (stable, no jitter)

### Example 2: Large change (MOVEMENT)
```
Time    Distance    Change    Servo Action
────────────────────────────────────────────
t0      10 cm       -         Move to 36°
t1      12 cm       +2 cm     MOVE to 50° (change >= 2 cm)
t2      13 cm       +1 cm     NO MOVE (< 2 cm from last)
t3      14 cm       +1 cm     NO MOVE (< 2 cm from last)
t4      15 cm       +1 cm     MOVE to 72° (change = 3 cm from t1)
```
**Result:** Servo moves in ~2 cm "steps"

### Example 3: Rapid oscillation (STABLE)
```
Time    Distance    Change    Servo Action
────────────────────────────────────────────
t0      15 cm       -         Move to 72°
t1      16 cm       +1 cm     NO MOVE (< 2 cm)
t2      15 cm       -1 cm     NO MOVE (< 2 cm)
t3      16 cm       +1 cm     NO MOVE (< 2 cm)
t4      15 cm       -1 cm     NO MOVE (< 2 cm)
```
**Result:** Servo ignores noise, stays at 72° (completely stable)

## Three-Layer Noise Reduction

1. **Low-Pass Filter (α = 0.3)**
   - Smooths raw sensor readings
   - Formula: `filtered = 0.3 × raw + 0.7 × previous`
   - Reduces high-frequency noise

2. **Integer Centimeters Only**
   - `getDistanceCm()` returns `uint16_t` (no decimals)
   - 5.7 cm and 5.2 cm both become 5 cm
   - Eliminates sub-centimeter jitter

3. **Hysteresis (2 cm dead zone)**
   - Servo moves only if distance changes >= 2 cm
   - Creates discrete "steps" in servo position
   - Prevents oscillation around boundaries

## Formula Breakdown

### Distance to Angle Conversion (5-30 cm range):

```cpp
// Step 1: Normalize distance to 0-based range
float normalizedDistance = distanceCm - MIN_DISTANCE;  // 5 cm → 0, 30 cm → 25

// Step 2: Calculate range
float range = MAX_DISTANCE - MIN_DISTANCE;  // 30 - 5 = 25 cm

// Step 3: Map to 0-180° angle
float angle = (normalizedDistance / range) * 180.0f;

// Example calculations:
// distanceCm = 5  → normalized = 0  → angle = (0/25) * 180 = 0°
// distanceCm = 15 → normalized = 10 → angle = (10/25) * 180 = 72°
// distanceCm = 30 → normalized = 25 → angle = (25/25) * 180 = 180°
```

## Tuning Parameters

### To change sensitivity (less movement):
```cpp
const uint16_t HYSTERESIS_CM = 3;   // Increase to 3 cm (even less sensitive)
sensor.setFilterStrength(0.2f);      // Decrease alpha (more smoothing)
```

### To change sensitivity (more responsive):
```cpp
const uint16_t HYSTERESIS_CM = 1;   // Decrease to 1 cm (more sensitive)
sensor.setFilterStrength(0.5f);      // Increase alpha (less smoothing)
```

### To change working range:
```cpp
const float MIN_DISTANCE = 10.0f;   // Start at 10 cm instead of 5 cm
const float MAX_DISTANCE = 50.0f;   // Extend to 50 cm instead of 30 cm
```

## Expected Serial Output

```
Distance: 5 cm → Servo: 0°
Distance: 7 cm → Servo: 14°
Distance: 9 cm → Servo: 28°
Distance: 10 cm → Servo: 28°      ← No movement (change = 1 cm < 2 cm)
Distance: 11 cm → Servo: 43°      ← Moved (change = 2 cm from 9 cm)
Distance: 12 cm → Servo: 43°      ← No movement (change = 1 cm < 2 cm)
Distance: 14 cm → Servo: 64°      ← Moved (change = 3 cm from 11 cm)
Distance: 32 cm (OUT OF RANGE) → Servo: 180°
```

Notice: Servo doesn't react to every 1 cm change, creating stable discrete positions.
