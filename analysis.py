import pandas as pd
import numpy as np

def analyze_execution_log(filename):
    metrics = {"throughput": 0, "average_turnaround_time": 0, "average_wait_time": 0}
    process_info = {}
    turnaround_times = []
    wait_times = []

    with open(filename, "r") as file:
        lines = file.readlines()
        for line in lines:
            if "|" in line:
                parts = [p.strip() for p in line.split("|")]
                if parts[1].isdigit():  # Process transitions
                    pid = int(parts[1])
                    time = int(parts[0])
                    state = parts[3]

                    if pid not in process_info:
                        process_info[pid] = {"arrival": time, "first_run": None, "completion": None}

                    if state == "RUNNING" and process_info[pid]["first_run"] is None:
                        process_info[pid]["first_run"] = time

                    if state == "TERMINATED":
                        process_info[pid]["completion"] = time

    for pid, info in process_info.items():
        if info["completion"] is not None:
            turnaround_time = info["completion"] - info["arrival"]
            turnaround_times.append(turnaround_time)
            metrics["throughput"] += 1

            if info["first_run"] is not None:
                wait_time = info["first_run"] - info["arrival"]
                wait_times.append(wait_time)

    metrics["average_turnaround_time"] = np.mean(turnaround_times) if turnaround_times else 0
    metrics["average_wait_time"] = np.mean(wait_times) if wait_times else 0

    return metrics

# Example Usage
results_fcfs = analyze_execution_log("execution_FCFS_1.txt")
results_ep = analyze_execution_log("execution_EP_1.txt")
results_rr = analyze_execution_log("execution_RR_1.txt")

print("FCFS Metrics:", results_fcfs)
print("EP Metrics:", results_ep)
print("RR Metrics:", results_rr)
