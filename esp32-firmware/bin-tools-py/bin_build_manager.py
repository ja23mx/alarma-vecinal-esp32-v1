import shutil
import os
import json
from datetime import datetime
import inspect

# Importar configuración
from bin_build_config import NAME_BASE, VERSION, OTA_DIR

def get_next_build_number(file_path):
    if os.path.exists(file_path):
        with open(file_path, "r") as f:
            try:
                num = int(f.read().strip())
            except ValueError:
                num = 0
    else:
        num = 0
    num += 1
    with open(file_path, "w") as f:
        f.write(str(num))
    return num

def after_build(source, target, env):
    firmware_bin = str(target[0])

    script_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
    build_file = os.path.join(script_dir, "build_number.txt")
    build_number = get_next_build_number(build_file)
    timestamp = datetime.now().strftime("%d-%m-%Y-%H%M%SHRS")

    base_name = f"{NAME_BASE}{VERSION}-B-{build_number:04d}-T-{timestamp}"

    ota_dir = os.path.abspath(OTA_DIR)
    os.makedirs(ota_dir, exist_ok=True)
    ota_name = f"{base_name}-ota.bin"
    ota_dest = os.path.join(ota_dir, ota_name)

    print("\n" + "="*80)
    print(f"BUILD #{build_number:04d} - {timestamp}")
    print("="*80)

    if os.path.exists(firmware_bin):
        shutil.copy(firmware_bin, ota_dest)
        size_kb = os.path.getsize(ota_dest) / 1024
        print(f"✓ OTA: {ota_name} ({size_kb:.2f} KB)")
        print(f"       {ota_dir}")
    else:
        print(f"✗ ERROR: No se encontró {firmware_bin}")

    manifest = {
        "version": VERSION,
        "build": f"{build_number:04d}",
        "fecha": timestamp,
        "file": ota_name
    }
    manifest_path = os.path.join(ota_dir, "manifest.json")
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2)
    print(f"✓ manifest.json actualizado")

    print("="*80 + "\n")

Import("env")
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)
