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
    script_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
    build_file = os.path.join(script_dir, "build_number.txt")
    build_number = get_next_build_number(build_file)
    timestamp = datetime.now().strftime("%d-%m-%Y-%H%M%SHRS")
    firmware_name = f"{NAME_BASE}-{VERSION}-B-{build_number:04d}-T-{timestamp}.bin"
    destino_dir = os.path.abspath(DEST_DIR)
    os.makedirs(destino_dir, exist_ok=True)
    destino = os.path.join(destino_dir, firmware_name)
    shutil.copy(firmware_bin, destino)
    print(f'Firmware copiado a: {destino}')

Import("env")
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)
