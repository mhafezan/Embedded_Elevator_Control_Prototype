# Test Cases

## Test Case 1: Move from Floor 1 to Floor 3

**Initial condition:**

- Current floor: 1
- Target floor: 1
- State: IDLE

**Input:**

- Requested floor: 3
- Emergency stop: 0
- Door obstruction: 0
- Upper limit switch: 0
- Lower limit switch: 0

**Expected behavior:**

- Controller enters MOVING_UP state
- Motor command becomes UP
- Elevator moves toward floor 3
- At floor 3, motor stops
- Door opens

---

## Test Case 2: Move from Floor 3 to Floor 1

**Input:**

- Requested floor: 1

**Expected behavior:**

- Controller enters MOVING_DOWN state
- Motor command becomes DOWN
- Elevator moves toward floor 1
- At floor 1, motor stops
- Door opens

---

## Test Case 3: Emergency Stop During Movement

**Input:**

- Emergency stop: 1

**Expected behavior:**

- Motor immediately stops
- Controller enters EMERGENCY_STOP state
- Door remains closed

---

## Test Case 4: Door Obstruction

**Input:**

- Door obstruction: 1

**Expected behavior:**

- Door remains open
- Motor remains stopped
- Controller does not resume normal operation until obstruction is cleared

---

## Test Case 5: Upper Limit Switch Protection

**Input:**

- Upper limit switch: 1 while motor command is UP

**Expected behavior:**

- Motor stops
- Controller prevents further upward movement

---

## Test Case 6: Lower Limit Switch Protection

**Input:**

- Lower limit switch: 1 while motor command is DOWN

**Expected behavior:**

- Motor stops
- Controller prevents further downward movement