import shutil
import os
import json
import re
from datetime import datetime
import inspect

from bin_build_config import HEADER_PATH, OTA_DIR

def parse_header(header_path):
    defines = {}
    pattern = re.compile(r'#define\s+(\w+)\s+"([^"]+)"')
    with open(header_path, "r") as f:
        for line in f:
            m = pattern.match(line.strip())
            if m:
                defines[m.group(1)] = m.group(2)
    return defines

def after_build(source, target, env):
    firmware_bin = str(target[0])

    script_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
    header_path = os.path.abspath(os.path.join(script_dir, HEADER_PATH))
    info = parse_header(header_path)

    firmware       = info.get("SISTEMA_FIRMWARE", "esp32.av")
    ver            = info.get("SISTEMA_VERSION", "0.0.0")
    etapa          = info.get("SISTEMA_ETAPA", "beta")
    version_file   = f"{ver}-{etapa}"    # 0.1.0-beta.1  (para nombre de archivo)
    version_mf     = f"{ver}.{etapa}"    # 0.1.0.beta.1  (para manifest)

    timestamp = datetime.now().strftime("T.%d.%m.%Y.%H%M%SHRS")
    ota_name  = f"{firmware}-v{version_file}-{timestamp}-ota.bin"

    ota_dir  = os.path.abspath(OTA_DIR)
    os.makedirs(ota_dir, exist_ok=True)

    for f in os.listdir(ota_dir):
        if f.endswith("-ota.bin"):
            os.remove(os.path.join(ota_dir, f))

    ota_dest = os.path.join(ota_dir, ota_name)

    print("\n" + "="*80)
    print(f"BUILD  {firmware}  v{version_file}  {timestamp}")
    print("="*80)

    if os.path.exists(firmware_bin):
        shutil.copy(firmware_bin, ota_dest)
        size_kb = os.path.getsize(ota_dest) / 1024
        print(f"✓ OTA: {ota_name} ({size_kb:.2f} KB)")
        print(f"       {ota_dir}")
    else:
        print(f"✗ ERROR: No se encontró {firmware_bin}")

    manifest = {
        "firmware": firmware,
        "version": version_mf,
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
