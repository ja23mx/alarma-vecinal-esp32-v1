import shutil
import os
from datetime import datetime
import inspect

# Importar configuración
from bin_build_config import NAME_BASE, VERSION, DEST_DIR

def get_next_build_number(file_path):
    if os.path.exists(file_path):
        with open(file_path, "r") as f:
            try:
                num = int(f.read().strip())
            except ValueError:
                num = 0  # Si el archivo está vacío o contiene un valor no válido, inicializa en 0
    else:
        num = 0
    num += 1
    with open(file_path, "w") as f:
        f.write(str(num))
    return num

def after_build(source, target, env):
    firmware_bin = str(target[0])
    build_dir = env.subst("$BUILD_DIR")
    
    # Obtener directorio del framework para boot_app0.bin
    framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
    boot_app0_path = os.path.join(framework_dir, "tools", "partitions", "boot_app0.bin")
    
    # Rutas de los archivos binarios
    bootloader_bin = os.path.join(build_dir, "bootloader.bin")
    partitions_bin = os.path.join(build_dir, "partitions.bin")
    
    script_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
    build_file = os.path.join(script_dir, "build_number.txt")
    build_number = get_next_build_number(build_file)
    timestamp = datetime.now().strftime("%d-%m-%Y-%H%M%SHRS")
    
    # Crear nombre base para los archivos
    base_name = f"{NAME_BASE}-{VERSION}-B-{build_number:04d}-T-{timestamp}"
    
    destino_dir = os.path.abspath(DEST_DIR)
    os.makedirs(destino_dir, exist_ok=True)
    
    # Lista de archivos a copiar con sus direcciones de flash
    archivos = [
        {"src": bootloader_bin, "name": f"{base_name}-0x1000-bootloader.bin", "addr": "0x1000"},
        {"src": partitions_bin, "name": f"{base_name}-0x8000-partitions.bin", "addr": "0x8000"},
        {"src": boot_app0_path, "name": f"{base_name}-0xe000-boot_app0.bin", "addr": "0xe000"},
        {"src": firmware_bin, "name": f"{base_name}-0x10000-firmware.bin", "addr": "0x10000"}
    ]
    
    print("\n" + "="*80)
    print(f"BUILD #{build_number:04d} - {timestamp}")
    print("="*80)
    
    # Copiar cada archivo
    for archivo in archivos:
        if os.path.exists(archivo["src"]):
            destino = os.path.join(destino_dir, archivo["name"])
            shutil.copy(archivo["src"], destino)
            size_kb = os.path.getsize(destino) / 1024
            print(f"✓ {archivo['addr']}: {archivo['name']} ({size_kb:.2f} KB)")
        else:
            print(f"✗ ERROR: No se encontró {archivo['src']}")
    
    # Crear archivo de instrucciones para flashear
    instructions_file = os.path.join(destino_dir, f"{base_name}-FLASH-INSTRUCTIONS.txt")
    with open(instructions_file, "w") as f:
        f.write(f"INSTRUCCIONES DE FLASHEO - BUILD #{build_number:04d}\n")
        f.write(f"Fecha: {timestamp}\n")
        f.write(f"Versión: {NAME_BASE} {VERSION}\n")
        f.write("="*80 + "\n\n")
        f.write("OPCIÓN 1 - ESP32 Flash Download Tool:\n")
        f.write("-" * 40 + "\n")
        for archivo in archivos:
            f.write(f"  {archivo['addr']}: {archivo['name']}\n")
        f.write("\nConfiguracion:\n")
        f.write("  SPI SPEED: 40MHz\n")
        f.write("  SPI MODE: DIO\n")
        f.write("  FLASH SIZE: 4MB\n\n")
        
        f.write("OPCIÓN 2 - esptool.py:\n")
        f.write("-" * 40 + "\n")
        cmd = "esptool.py --chip esp32 --port COM4 --baud 921600 write_flash "
        for archivo in archivos:
            cmd += f"{archivo['addr']} {archivo['name']} "
        f.write(cmd + "\n\n")
        
        f.write("OPCIÓN 3 - PlatformIO:\n")
        f.write("-" * 40 + "\n")
        f.write("pio run --target upload\n")
    
    print(f"\n✓ Instrucciones guardadas en: {instructions_file}")
    print("="*80 + "\n")

Import("env")
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)
