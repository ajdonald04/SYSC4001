import numpy as np

def analyze_execution_log(filename):
    metrics = {
        "throughput": 0,
        "average_turnaround_time": 0,
        "average_wait_time": 0,
        "average_response_time": 0,
        "cpu_utilization": 0
    }
    process_info = {}
    turnaround_times = []
    wait_times = []
    response_times = []
    total_execution_time = 0
    total_time = 0

    try:
        with open(filename, "r") as file:
            lines = file.readlines()

            for line in lines:
                if not line.strip() or "Time of Transition" in line:
                    continue

                parts = [p.strip() for p in line.split("|") if p.strip()]

                if len(parts) == 4 and parts[0].isdigit():
                    try:
                        time = int(parts[0])  # time of transition
                        pid = int(parts[1])  # process ID
                        state = parts[3]  # new State

                        # Initialize process information
                        if pid not in process_info:
                            process_info[pid] = {
                                "arrival": None,
                                "first_run": None,
                                "completion": None,
                                "total_cpu_time": 0,
                                "waiting_time": 0,
                                "last_run_time": 0
                            }

                        if state == "READY" and process_info[pid]["arrival"] is None:
                            process_info[pid]["arrival"] = time

                        if state == "RUNNING" and process_info[pid]["first_run"] is None:
                            process_info[pid]["first_run"] = time

                        if state == "RUNNING":
                            if process_info[pid]["last_run_time"] > 0:
                                process_info[pid]["total_cpu_time"] += time - process_info[pid]["last_run_time"]
                            total_execution_time += 1
                            process_info[pid]["last_run_time"] = time

                        if state == "TERMINATED":
                            process_info[pid]["completion"] = time

                        total_time = max(total_time, time)

                    except ValueError:
                        continue
                else:
                    continue

        # loop to calculate the metrics 
        for pid, info in process_info.items():
            if info["arrival"] is not None and info["completion"] is not None:
                turnaround_time = info["completion"] - info["arrival"]
                turnaround_times.append(turnaround_time)
                metrics["throughput"] += 1

                if info["first_run"] is not None:
                    response_time = info["first_run"] - info["arrival"]
                    response_times.append(max(response_time, 0)) 

                if info["total_cpu_time"] is not None:
                    wait_time = turnaround_time - info["total_cpu_time"]
                    wait_times.append(max(wait_time, 0))

        # calculate the averages
        metrics["average_turnaround_time"] = round(float(np.mean(turnaround_times)), 2) if turnaround_times else 0
        metrics["average_wait_time"] = round(float(np.mean(wait_times)), 2) if wait_times else 0
        metrics["average_response_time"] = round(float(np.mean(response_times)), 2) if response_times else 0

        # calculate the CPU utilization
        if total_time > 0:
            metrics["cpu_utilization"] = round((total_execution_time / total_time) * 100, 2)

    except FileNotFoundError:
        print(f"Error: File {filename} not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

    return metrics


if __name__ == "__main__":
    # swap these to the file names as needed 
    fcfs_results = analyze_execution_log("execution_FCFS_input_data_8.txt")
    ep_results = analyze_execution_log("execution_EP_input_data_8.txt")
    rr_results = analyze_execution_log("execution_RR_input_data_8.txt")

    print("FCFS Metrics:", fcfs_results)
    print("EP Metrics:", ep_results)
    print("RR Metrics:", rr_results)
