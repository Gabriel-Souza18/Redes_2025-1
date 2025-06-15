import csv
from collections import defaultdict

T = ["Failed_requests_","Requests_per_second_","Time_per_request_concurrent_",
     "Time_per_request_mean_","Transfer_rate_"]
C = ["c1.txt", "c10.txt", "c25.txt", "c50.txt", "c100.txt"]

# 1) lê todos os dados em data[suffix][req][prefix][col] = valor
data = {c: defaultdict(dict) for c in C}
for suffix in C:
    for prefix in T:
        path = prefix + suffix
        with open(path, newline='') as f:
            reader = csv.reader(f, delimiter='\t')
            header = next(reader)[1:]            # ignora “Requisicoes”
            for row in reader:
                try:
                    req = int(row[0])
                except ValueError:
                    continue
                vals = {}
                for i, v in enumerate(row[1:]):
                    try:
                        vals[header[i]] = float(v)
                    except ValueError:
                        pass
                data[suffix][req][prefix] = vals

# 2) coleta todas as requisições existentes
all_reqs = sorted({req for suffix in C for req in data[suffix]})

# 3) para cada req e cada prefix, guarda o máximo de cada coluna sobre todos os suffixes
max_data = {
    req: {prefix: {} for prefix in T}
    for req in all_reqs
}

for suffix in C:
    for req, by_pref in data[suffix].items():
        for prefix, cols in by_pref.items():
            for col, v in cols.items():
                cur = max_data[req][prefix]
                if col not in cur or v > cur[col]:
                    cur[col] = v

# 4) imprime resultados agrupados por número de requisições
for req in all_reqs:
    print(f"== Requisições = {req} ==")
    for prefix in T:
        cols = max_data[req][prefix]
        if not cols:
            continue
        print(f"  → {prefix}")
        for col, v in cols.items():
            print(f"     {col}: {v:.2f}")
    print()

