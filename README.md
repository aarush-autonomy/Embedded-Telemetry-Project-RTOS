# Real-Time Embedded Sensor Acquisition & Telemetry Node

A modular, multi-process embedded software simulation in C demonstrating real-time data acquisition, digital low-pass filtering, CAN-style bitwise serialization, and Inter-Process Communication (IPC) on Windows.

## Architecture

the system mimics a RTOS telemetry in the VCU inside automobiles
transmitting and recieving and also identifying basic errors
its a short hand model explaining how the communication and bit sifting works


- Sensor Node (`sensor_node.c`): It simulates ID's with an added noise for real time telemetry scenario,applies an Exponential Moving Average (EMA) low-pass filter, serializes telemetry into an 8-byte payload, and injects diagnostic faults.
- VCU Logger (`vcu_logger.c`): Acts as the central vehicle supervisory controller, listening on an OS-managed Windows Named Pipe (`\\.\pipe\SensorBus`), The VCU Logger acts as the car's main computer: it continuously listens for incoming sensor data over a local communication line, unwraps the raw 8-byte messages back into real temperature and timing numbers, catches any dropped packets by watching for skipped counter numbers, and flags safety warnings on screen if the sensor reports a fault.