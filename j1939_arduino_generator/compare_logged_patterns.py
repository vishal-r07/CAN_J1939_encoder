import os
import csv
import glob
import re
import sys
import math
from pathlib import Path
import pandas as pd

# Add project root to path so we can import J1939Database
sys.path.append(str(Path(__file__).parent))
from realtime_j1939_verifier import J1939Database

# =============================================================================
# STRESS TESTING CONFIGURATIONS (Must match the Arduino firmware)
# =============================================================================
TEST_PERIOD_MS = 3.0      # Broadcast period in milliseconds
TOTAL_DATA_POINTS = 5000  # Number of steps in the sequence
MARKER_DURATION_MS = 3000 # Duration of start/end markers in ms

# All logical signals now start/end at 0.0
MARKER_VALUES = {
    "SPN190": 0.0,
    "SPN91": 0.0,
    "SPN84": 0.0,
    "SPN183": 0.0,
    "SPN184": 0.0
}

# =============================================================================
# DYNAMIC MATHEMATICAL WAVEFORM FUNCTIONS (One Cycle, starting/ending at 0.0)
# =============================================================================
def get_original_phys_value(spn, index):
    if spn == "SPN190":
        # Ramp up to 3000, then down to 0
        half = TOTAL_DATA_POINTS // 2
        if index < half:
            return 3000.0 * index / half
        else:
            return 3000.0 - 3000.0 * (index - half) / half
            
    elif spn == "SPN91":
        # Staircase step: 0 to 100 to 0 (20 steps total)
        samples_per_step = TOTAL_DATA_POINTS // 20
        if samples_per_step == 0:
            samples_per_step = 1
        step_idx = index // samples_per_step
        if step_idx < 10:
            return step_idx * 10.0
        elif step_idx < 20:
            return 100.0 - (step_idx - 10) * 10.0
        else:
            return 0.0
            
    elif spn == "SPN84":
        # Complete sine wave with ramp-in and ramp-out to start and end at 0.0
        if index < 500:
            return 50.0 * index / 500.0
        elif index < 4500:
            return 50.0 + 30.0 * math.sin(2.0 * math.pi * (index - 500) / 4000.0)
        else:
            return 50.0 - 50.0 * (index - 4500) / 500.0
        
    elif spn == "SPN183":
        # State sequence: 5 intervals (starts/ends at 0.0)
        samples_per_interval = TOTAL_DATA_POINTS // 5
        if samples_per_interval == 0:
            samples_per_interval = 1
        interval = index // samples_per_interval
        if interval == 0:
            return 0.0
        elif interval == 1:
            return 120.0
        elif interval == 2:
            return 250.0
        elif interval == 3:
            return 380.0
        else:
            return 0.0
            
    elif spn == "SPN184":
        # Constant economy during the pattern, transitions from/to 0.0 marker
        return 15.0
        
    return 0.0

def find_newest_csv():
    csv_files = glob.glob("*.csv") + glob.glob("*.CSV")
    csv_files = [f for f in csv_files if "comparison_results" not in f.lower()]
    if not csv_files:
        raise FileNotFoundError("No log CSV files found in the current directory.")
    csv_files.sort(key=os.path.getmtime, reverse=True)
    return csv_files[0]

def parse_raw_payload(raw_data_str):
    try:
        parts = raw_data_str.strip().split()
        if 'D' in parts:
            d_idx = parts.index('D')
            dlc = int(parts[d_idx + 1])
            hex_bytes = parts[d_idx + 2 : d_idx + 2 + dlc]
            return bytes.fromhex("".join(hex_bytes))
    except Exception:
        pass
    return None

def parse_logged_data(filepath, db):
    print(f"Reading newest log file: {filepath}")
    
    signals_by_spn = {}
    for msg in db.messages:
        for sig in msg.signals:
            if "engine_speed" in sig.name.lower():
                signals_by_spn["SPN190"] = sig
            elif "accel_pedal" in sig.name.lower():
                signals_by_spn["SPN91"] = sig
            elif "vehicle_speed" in sig.name.lower():
                signals_by_spn["SPN84"] = sig
            elif "fuel_rate" in sig.name.lower():
                signals_by_spn["SPN183"] = sig
            elif "inst_fuel" in sig.name.lower():
                signals_by_spn["SPN184"] = sig

    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        reader = csv.reader(f)
        header = next(reader)
        
        clean_header = [h.strip().lower() for h in header]
        
        col_indices = {
            "time": -1,
            "SPN190": -1,
            "SPN91": -1,
            "SPN84": -1,
            "SPN183": -1,
            "SPN184": -1,
            "raw_data": -1
        }
        
        for i, col in enumerate(clean_header):
            if "time" in col:
                col_indices["time"] = i
                break
                
        for i, col in enumerate(clean_header):
            if "raw data" in col or "raw_data" in col:
                col_indices["raw_data"] = i
                break
        
        patterns = {
            "SPN190": re.compile(r"190|speed"),
            "SPN91": re.compile(r"91|pedal|accel"),
            "SPN84": re.compile(r"84|vehicle|wheel"),
            "SPN183": re.compile(r"183|fuel_rate|fuel rate"),
            "SPN184": re.compile(r"184|inst_fuel|instantaneous")
        }
        
        for spn, pattern in patterns.items():
            for i, col in enumerate(clean_header):
                if "expected" in col or "ref" in col or col == "raw data":
                    continue
                if pattern.search(col):
                    col_indices[spn] = i
                    break
        
        logged_series = {spn: [] for spn in MARKER_VALUES.keys()}
        
        for row in reader:
            if not row or len(row) <= max(col_indices.values()):
                continue
                
            t_val = 0.0
            if col_indices["time"] != -1:
                try:
                    t_val = float(row[col_indices["time"]])
                except ValueError:
                    continue
                    
            raw_payload = None
            if col_indices["raw_data"] != -1:
                raw_payload = parse_raw_payload(row[col_indices["raw_data"]])

            for spn in MARKER_VALUES.keys():
                idx = col_indices[spn]
                if idx == -1:
                    continue
                    
                val_str = row[idx].strip()
                if val_str:
                    try:
                        phys_val = float(val_str)
                        raw_val = 0
                        
                        sig_def = signals_by_spn.get(spn)
                        if raw_payload is not None and sig_def is not None:
                            try:
                                parts = row[col_indices["raw_data"]].strip().split()
                                id_part = [p for p in parts if p.startswith('$')]
                                if id_part:
                                    msg_id = int(id_part[0].replace('$', ''), 16) & 0x1FFFFFFF
                                    if msg_id == sig_def.can_identifier:
                                        raw_val, _ = sig_def.decode(raw_payload)
                                    else:
                                        continue
                            except Exception:
                                raw_val = sig_def.encode(phys_val)
                        elif sig_def is not None:
                            raw_val = sig_def.encode(phys_val)
                            
                        logged_series[spn].append((t_val, phys_val, raw_val))
                    except ValueError:
                        pass
                        
        return logged_series, signals_by_spn

def find_master_start_time(logged_series):
    """
    Looks for the start of the waveform block by identifying the first
    timestamp where any signal leaves its 0.0 marker state.
    SPN 184 is the most robust since it jumps directly to 15.0 at index 0.
    """
    # 1. Look for SPN 184 jumping to 15.0
    for t, val, raw in logged_series["SPN184"]:
        if abs(val - 15.0) < 1.0:
            return t
            
    # 2. Look for SPN 190 leaving 0.0
    for t, val, raw in logged_series["SPN190"]:
        if val > 1.0:
            return t
            
    # 3. Generic fallback
    for spn in ["SPN190", "SPN91", "SPN84", "SPN183", "SPN184"]:
        for t, val, raw in logged_series[spn]:
            if val > 0.1:
                return t
                
    return 0.0

def align_all_series(logged_series):
    start_time = find_master_start_time(logged_series)
    print(f"Detected waveform master start time at: {start_time:.6f} seconds")
    
    extracted_phys = {}
    extracted_raw = {}
    
    for spn in ["SPN190", "SPN91", "SPN84", "SPN183", "SPN184"]:
        aligned_phys = [None] * TOTAL_DATA_POINTS
        aligned_raw = [None] * TOTAL_DATA_POINTS
        
        for t, val, raw in logged_series[spn]:
            if t < start_time - 0.05:  # Skip start markers
                continue
            elapsed_ms = (t - start_time) * 1000.0
            idx = int(round(elapsed_ms / TEST_PERIOD_MS))
            
            if 0 <= idx < TOTAL_DATA_POINTS:
                aligned_phys[idx] = val
                aligned_raw[idx] = raw
                
        extracted_phys[spn] = aligned_phys
        extracted_raw[spn] = aligned_raw
        
    return extracted_phys, extracted_raw

def main():
    dbc_path = Path(__file__).parent / "STM32F103_J1939_Signal_Generator.dbc"
    db = J1939Database(dbc_path)
    
    try:
        newest_file = find_newest_csv()
    except FileNotFoundError as e:
        print(e)
        return
        
    logged_series, signals_by_spn = parse_logged_data(newest_file, db)
    
    # Run the master alignment
    extracted_phys, extracted_raw = align_all_series(logged_series)
    
    print("\nAligning waveforms using J1939 timing alignment:")
    for spn in ["SPN190", "SPN91", "SPN84", "SPN183", "SPN184"]:
        received_unique = sum(1 for v in extracted_phys[spn] if v is not None)
        dropped_count = TOTAL_DATA_POINTS - received_unique
        drop_rate = (dropped_count / TOTAL_DATA_POINTS) * 100.0
        print(f"  {spn} -> Logged: {received_unique}/{TOTAL_DATA_POINTS}. Missed: {dropped_count} frames ({drop_rate:.2f}% drop rate)")

    # =============================================================================
    # WRITE EXCEL WORKBOOK (Multi-sheet, each SPN has its own tab)
    # =============================================================================
    excel_filename = "comparison_results.xlsx"
    print(f"\nWriting multi-page Excel workbook: {excel_filename}")
    
    sheet_names = {
        "SPN190": "SPN190_Engine_Speed",
        "SPN91": "SPN91_Accelerator",
        "SPN84": "SPN84_Vehicle_Speed",
        "SPN183": "SPN183_Fuel_Rate",
        "SPN184": "SPN184_Fuel_Economy"
    }
    
    with pd.ExcelWriter(excel_filename, engine="openpyxl") as writer:
        for spn in ["SPN190", "SPN91", "SPN84", "SPN183", "SPN184"]:
            phys_logged = extracted_phys[spn]
            raw_logged = extracted_raw[spn]
            
            phys_original = [get_original_phys_value(spn, i) for i in range(TOTAL_DATA_POINTS)]
            sig_def = signals_by_spn.get(spn)
            raw_original = [sig_def.encode(p) if sig_def is not None else 0 for p in phys_original]
            
            # Compute errors
            errors = []
            for i in range(TOTAL_DATA_POINTS):
                if phys_logged[i] is not None:
                    errors.append(round(phys_logged[i] - phys_original[i], 4))
                else:
                    errors.append(None)
            
            df = pd.DataFrame({
                "Index": range(TOTAL_DATA_POINTS),
                "Logged_Physical": phys_logged,
                "Logged_Raw": raw_logged,
                "Original_Physical": phys_original,
                "Original_Raw": raw_original,
                "Physical_Error": errors
            })
            
            df.to_excel(writer, sheet_name=sheet_names[spn], index=False)
            
    print(f"Excel workbook created successfully with 5 sheets: {', '.join(sheet_names.values())}")

    # =============================================================================
    # WRITE SINGLE-SHEET BACKUP CSV
    # =============================================================================
    csv_filename = "comparison_results.csv"
    print(f"Writing single-sheet fallback CSV: {csv_filename}")
    with open(csv_filename, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "Index",
            "SPN190_Logged_Phys", "SPN190_Logged_Raw", "SPN190_Original_Phys", "SPN190_Original_Raw", "SPN190_Error",
            "SPN91_Logged_Phys",  "SPN91_Logged_Raw",  "SPN91_Original_Phys",  "SPN91_Original_Raw",  "SPN91_Error",
            "SPN84_Logged_Phys",  "SPN84_Logged_Raw",  "SPN84_Original_Phys",  "SPN84_Original_Raw",  "SPN84_Error",
            "SPN183_Logged_Phys", "SPN183_Logged_Raw", "SPN183_Original_Phys", "SPN183_Original_Raw", "SPN183_Error",
            "SPN184_Logged_Phys", "SPN184_Logged_Raw", "SPN184_Original_Phys", "SPN184_Original_Raw", "SPN184_Error"
        ])
        for i in range(TOTAL_DATA_POINTS):
            row = [i]
            for spn in ["SPN190", "SPN91", "SPN84", "SPN183", "SPN184"]:
                p_log = extracted_phys[spn][i]
                r_log = extracted_raw[spn][i]
                
                p_orig = get_original_phys_value(spn, i)
                sig_def = signals_by_spn.get(spn)
                r_orig = sig_def.encode(p_orig) if sig_def is not None else 0
                
                if p_log is not None:
                    row.extend([round(p_log, 4), r_log, round(p_orig, 4), r_orig, round(p_log - p_orig, 4)])
                else:
                    row.extend(["", "", round(p_orig, 4), r_orig, ""])
            writer.writerow(row)
            
    print("\nVerification Complete! Open 'comparison_results.xlsx' in Excel to view each SPN on its own tab.")

if __name__ == "__main__":
    main()
