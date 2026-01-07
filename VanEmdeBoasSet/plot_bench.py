import json
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import re

def parse_benchmark_name(full_name):
    """
    Парсит имя вида "StdSet_Insert/4096" или "StdSet_Insert/4096/repeat:0"
    Возвращает (Structure, Operation, Size)
    """
    clean_name = full_name.split('/')[0]

    match = re.search(r'/(\d+)', full_name)
    size = int(match.group(1)) if match else 0

    parts = clean_name.split('_')
    structure = parts[0]
    operation = parts[1]

    return structure, operation, size

def plot_benchmarks(json_file):
    with open(json_file, 'r') as f:
        data = json.load(f)

    records = []

    for bench in data['benchmarks']:
        if 'aggregate_name' in bench:
            continue

        name = bench['name']
        time_ns = bench['cpu_time']

        structure, operation, size = parse_benchmark_name(name)

        records.append({
            "Structure": structure,
            "Operation": operation,
            "Input Size": size,
            "Time (ns)": time_ns
        })

    df = pd.DataFrame(records)

    sns.set_theme(style="whitegrid")

    operations = df["Operation"].unique()

    for op in operations:
        plt.figure(figsize=(10, 6))

        subset = df[df["Operation"] == op]

        ax = sns.lineplot(
            data=subset,
            x="Input Size",
            y="Time (ns)",
            hue="Structure",
            style="Structure",
            markers=True,
            dashes=False,
            linewidth=2.5
        )

        plt.title(f"Benchmark: {op} Operation", fontsize=16)
        plt.xlabel("Number of Elements", fontsize=12)
        plt.ylabel("CPU Time (ns)", fontsize=12)
        plt.xscale("log")
        plt.yscale("log")

        plt.grid(True, which="both", ls="--", alpha=0.5)

        plt.legend(title="Data Structure")
        plt.tight_layout()

        # filename = f"benchmark_{op}.png"
        filename = f"benchmark_{op}_log_scale.png"
        plt.savefig(filename)
        print(f"Saved plot to {filename}")
        plt.show()

if __name__ == "__main__":
    plot_benchmarks("results.json")