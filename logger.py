import serial
import csv
import time
import os

# --- 1. CONFIGURATION ---
PORT = 'COM5'  # Change this to your actual COM port (e.g., COM4)
BAUD = 115200

# --- 2. GET USER INPUT ---
label = input("Enter label (top_left, top_right, bottom_left, bottom_right, mouse, noise): ")
folder_path = f"data/{label}"

# --- 3. CREATE FOLDER ---
if not os.path.exists(folder_path):
    os.makedirs(folder_path)
    print(f"Created new folder: {folder_path}")

filename = f"{folder_path}/sample_{int(time.time())}.csv"

# --- 4. START LOGGING ---
try:
    # Ensure no other program is using the COM port!
    ser = serial.Serial(PORT, BAUD, timeout=1)
    print(f"Connected! Logging '{label}' to {filename}. Press Ctrl+C to STOP.")
    
    with open(filename, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["TL", "TR", "BL", "BR"]) # Header for Edge Impulse
        
        while True:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                data = line.split(',')
                if len(data) == 4:
                    writer.writerow(data)
                    # Optional: Print to console so you know it's working
                    print(f"Data: {data}", end='\r') 

except serial.SerialException:
    print(f"Error: Could not open {PORT}. Is the Serial Monitor still open?")
except KeyboardInterrupt:
    print(f"\nStopped. Sample saved to {filename}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()