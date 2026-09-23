# Architecture

## High-level flow

```text
Automated test case
      |
      v
Command / setpoint
      |
      v
DER device object
(Inverter / Battery / EVCharger)
      |
      v
Time-step model + operating rules
      |
      +----> fault / communications state
      |
      v
Simulated telemetry
      |
      +----> Modbus-style register map
      |
      v
Expected-vs-actual comparison
      |
      v
PASS / FAIL
      |
      +----> test_results.csv
      +----> inverter_step_response.csv
```

## Inheritance

`DerDevice` contains behaviour shared by every device: device ID, operating status, communications state, `start()`, `stop()` and the common telemetry interface.

`Inverter`, `Battery` and `EVCharger` inherit from `DerDevice` and add their own states, commands and simulation equations.

## Inverter model

The inverter uses a first-order response:

```text
dP/dt = (P_command - P)/tau
```

The implementation uses the exact discrete first-order coefficient:

```text
alpha = 1 - exp(-dt/tau)
P_next = P + alpha(P_command - P)
```

It also models a demonstration over-voltage fault and a manual reset condition.

## Battery model

Positive setpoint = discharge. Negative setpoint = charging.

Discharge:

```text
energy removed from battery = (P_out / eta_discharge) * dt
```

Charge:

```text
energy stored = P_in * eta_charge * dt
```

SOC change is energy change divided by rated capacity.

## EV charger model

The charger current follows a first-order response toward the requested current limit. Power is derived from:

```text
P = V * I
```

## Testability

Deterministic rather than random measurement error is used so every build produces repeatable automated test results.
