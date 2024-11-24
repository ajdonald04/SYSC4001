import numpy as np

def analyze_memory_log(filename):
    metrics = {
        "average_memory_usage": 0,
        "max_memory_usage": 0,
        "min_memory_usage": 0,
        "total_memory_utilization": 0,
    }
    memory_usage = []

    try:
        with open(filename, "r") as file:
            lines = file.readlines()

            for line in lines:
                # Skip headers or non-data lines
                if "Time of Event" in line or "Memory Used" in line or not line.strip():
                    continue

                # Split the line into parts and extract relevant data
                parts = [p.strip() for p in line.split("|") if p.strip()]

                # Ensure the line has the expected format
                if len(parts) >= 5:
                    try:
                        # Parse the memory used column (second column in the log)
                        memory_used = int(parts[1])  # Adjust column index if needed
                        memory_usage.append(memory_used)
                    except ValueError:
                        # Skip lines with non-integer values in the memory column
                        continue

        # Compute metrics if data is available
        if memory_usage:
            metrics["average_memory_usage"] = round(np.mean(memory_usage), 2)
            metrics["max_memory_usage"] = max(memory_usage)
            metrics["min_memory_usage"] = min(memory_usage)
            metrics["total_memory_utilization"] = sum(memory_usage)

    except FileNotFoundError:
        print(f"Error: File {filename} not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

    return metrics


# Analyze all memory logs for schedulers and traces
def analyze_all_memory_logs(schedulers, traces):
    results = {trace: [] for trace in traces}
    for trace in traces:
        for scheduler in schedulers:
            filename = f"memory_status_{scheduler}_{trace}.txt"
            metrics = analyze_memory_log(filename)
            metrics["scheduler"] = scheduler
            results[trace].append(metrics)
    return results


# Example Usage
if __name__ == "__main__":
    schedulers = ["FCFS", "EP", "RR"]
    traces = [f"input_data_{i}" for i in range(1, 11)]  # input_data_1 to input_data_10

    # Analyze all logs
    memory_results = analyze_all_memory_logs(schedulers, traces)

    # Print results grouped by trace
    for trace, metrics_list in memory_results.items():
        print(f"\nTrace: {trace}")
        for metrics in metrics_list:
            print(f"  Scheduler: {metrics['scheduler']}")
            print(f"    Average Memory Usage: {metrics['average_memory_usage']} MB")
            print(f"    Max Memory Usage: {metrics['max_memory_usage']} MB")
            print(f"    Min Memory Usage: {metrics['min_memory_usage']} MB")
            print(f"    Total Memory Utilization: {metrics['total_memory_utilization']} MB")
